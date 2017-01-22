// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Incremental decoding
//
// Author: somnath@google.com (Somnath Banerjee)

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "./alphai.h"
#include "./webpi.h"
#include "./vp8i.h"
#include "../utils/utils.h"

// In append mode, buffer allocations increase as multiples of this value.
// Needs to be a power of 2.
#define CHUNK_SIZE 4096
#define MAX_MB_SIZE 4096

//------------------------------------------------------------------------------
// Data structures for memory and states

// Decoding states. State normally flows as:
// WEBP_HEADER->DEDUP_vP8__HEADER->DEDUP_vP8__PARTS0->DEDUP_vP8__DATA->DONE for a lossy image, and
// WEBP_HEADER->DEDUP_vP8_L_HEADER->DEDUP_vP8_L_DATA->DONE for a lossless image.
// If there is any error the decoder goes into state ERROR.
typedef enum {
  STATE_WEBP_HEADER,  // All the data before that of the DEDUP_vP8_/DEDUP_vP8_L chunk.
  STATE_DEDUP_vP8__HEADER,   // The DEDUP_vP8_ Frame header (within the DEDUP_vP8_ chunk).
  STATE_DEDUP_vP8__PARTS0,
  STATE_DEDUP_vP8__DATA,
  STATE_DEDUP_vP8_L_HEADER,
  STATE_DEDUP_vP8_L_DATA,
  STATE_DONE,
  STATE_ERROR
} DecState;

// Operating state for the MemBuffer
typedef enum {
  MEM_MODE_NONE = 0,
  MEM_MODE_APPEND,
  MEM_MODE_MAP
} MemBufferMode;

// storage for partition #0 and partial data (in a rolling fashion)
typedef struct {
  MemBufferMode mode_;  // Operation mode
  size_t start_;        // start location of the data to be decoded
  size_t end_;          // end location
  size_t buf_size_;     // size of the allocated buffer
  uint8_t* buf_;        // We don't own this buffer in case DEDUP_WEBP_IUpdate()

  size_t part0_size_;         // size of partition #0
  const uint8_t* part0_buf_;  // buffer to store partition #0
} MemBuffer;

struct DEDUP_WEBP_IDecoder {
  DecState state_;         // current decoding state
  DEDUP_WEBP_DecParams params_;   // Params to store output info
  int is_lossless_;        // for down-casting 'dec_'.
  void* dec_;              // either a DEDUP_vP8_Decoder or a DEDUP_vP8_LDecoder instance
  DEDUP_vP8_Io io_;

  MemBuffer mem_;          // input memory buffer.
  DEDUP_WEBP_DecBuffer output_;   // output buffer (when no external one is supplied,
                           // or if the external one has slow-memory)
  DEDUP_WEBP_DecBuffer* final_output_;  // Slow-memory output to copy to eventually.
  size_t chunk_size_;      // Compressed DEDUP_vP8_/DEDUP_vP8_L size extracted from Header.

  int last_mb_y_;          // last row reached for intra-mode decoding
};

// MB context to restore in case DEDUP_vP8_DecodeMB() fails
typedef struct {
  DEDUP_vP8_MB left_;
  DEDUP_vP8_MB info_;
  DEDUP_vP8_BitReader token_br_;
} MBContext;

//------------------------------------------------------------------------------
// MemBuffer: incoming data handling

static WEBP_INLINE size_t MemDataSize(const MemBuffer* mem) {
  return (mem->end_ - mem->start_);
}

// Check if we need to preserve the compressed alpha data, as it may not have
// been decoded yet.
static int NeedCompressedAlpha(const DEDUP_WEBP_IDecoder* const idec) {
  if (idec->state_ == STATE_WEBP_HEADER) {
    // We haven't parsed the headers yet, so we don't know whether the image is
    // lossy or lossless. This also means that we haven't parsed the ALPH chunk.
    return 0;
  }
  if (idec->is_lossless_) {
    return 0;  // ALPH chunk is not present for lossless images.
  } else {
    const DEDUP_vP8_Decoder* const dec = (DEDUP_vP8_Decoder*)idec->dec_;
    assert(dec != NULL);  // Must be true as idec->state_ != STATE_WEBP_HEADER.
    return (dec->alpha_data_ != NULL) && !dec->is_alpha_decoded_;
  }
}

static void DoRemap(DEDUP_WEBP_IDecoder* const idec, ptrdiff_t offset) {
  MemBuffer* const mem = &idec->mem_;
  const uint8_t* const new_base = mem->buf_ + mem->start_;
  // note: for DEDUP_vP8_, setting up idec->io_ is only really needed at the beginning
  // of the decoding, till partition #0 is complete.
  idec->io_.data = new_base;
  idec->io_.data_size = MemDataSize(mem);

  if (idec->dec_ != NULL) {
    if (!idec->is_lossless_) {
      DEDUP_vP8_Decoder* const dec = (DEDUP_vP8_Decoder*)idec->dec_;
      const uint32_t last_part = dec->num_parts_minus_one_;
      if (offset != 0) {
        uint32_t p;
        for (p = 0; p <= last_part; ++p) {
          DEDUP_vP8_RemapBitReader(dec->parts_ + p, offset);
        }
        // Remap partition #0 data pointer to new offset, but only in MAP
        // mode (in APPEND mode, partition #0 is copied into a fixed memory).
        if (mem->mode_ == MEM_MODE_MAP) {
          DEDUP_vP8_RemapBitReader(&dec->br_, offset);
        }
      }
      {
        const uint8_t* const last_start = dec->parts_[last_part].buf_;
        DEDUP_vP8_BitReaderSetBuffer(&dec->parts_[last_part], last_start,
                              mem->buf_ + mem->end_ - last_start);
      }
      if (NeedCompressedAlpha(idec)) {
        ALPHDecoder* const alph_dec = dec->alph_dec_;
        dec->alpha_data_ += offset;
        if (alph_dec != NULL) {
          if (alph_dec->method_ == ALPHA_LOSSLESS_COMPRESSION) {
            DEDUP_vP8_LDecoder* const alph_vp8l_dec = alph_dec->vp8l_dec_;
            assert(alph_vp8l_dec != NULL);
            assert(dec->alpha_data_size_ >= ALPHA_HEADER_LEN);
            DEDUP_vP8_LBitReaderSetBuffer(&alph_vp8l_dec->br_,
                                   dec->alpha_data_ + ALPHA_HEADER_LEN,
                                   dec->alpha_data_size_ - ALPHA_HEADER_LEN);
          } else {  // alph_dec->method_ == ALPHA_NO_COMPRESSION
            // Nothing special to do in this case.
          }
        }
      }
    } else {    // Resize lossless bitreader
      DEDUP_vP8_LDecoder* const dec = (DEDUP_vP8_LDecoder*)idec->dec_;
      DEDUP_vP8_LBitReaderSetBuffer(&dec->br_, new_base, MemDataSize(mem));
    }
  }
}

// Appends data to the end of MemBuffer->buf_. It expands the allocated memory
// size if required and also updates DEDUP_vP8_BitReader's if new memory is allocated.
static int AppendToMemBuffer(DEDUP_WEBP_IDecoder* const idec,
                             const uint8_t* const data, size_t data_size) {
  DEDUP_vP8_Decoder* const dec = (DEDUP_vP8_Decoder*)idec->dec_;
  MemBuffer* const mem = &idec->mem_;
  const int need_compressed_alpha = NeedCompressedAlpha(idec);
  const uint8_t* const old_start = mem->buf_ + mem->start_;
  const uint8_t* const old_base =
      need_compressed_alpha ? dec->alpha_data_ : old_start;
  assert(mem->mode_ == MEM_MODE_APPEND);
  if (data_size > MAX_CHUNK_PAYLOAD) {
    // security safeguard: trying to allocate more than what the format
    // allows for a chunk should be considered a smoke smell.
    return 0;
  }

  if (mem->end_ + data_size > mem->buf_size_) {  // Need some free memory
    const size_t new_mem_start = old_start - old_base;
    const size_t current_size = MemDataSize(mem) + new_mem_start;
    const uint64_t new_size = (uint64_t)current_size + data_size;
    const uint64_t extra_size = (new_size + CHUNK_SIZE - 1) & ~(CHUNK_SIZE - 1);
    uint8_t* const new_buf =
        (uint8_t*)DEDUP_WEBP_SafeMalloc(extra_size, sizeof(*new_buf));
    if (new_buf == NULL) return 0;
    memcpy(new_buf, old_base, current_size);
    DEDUP_WEBP_SafeFree(mem->buf_);
    mem->buf_ = new_buf;
    mem->buf_size_ = (size_t)extra_size;
    mem->start_ = new_mem_start;
    mem->end_ = current_size;
  }

  memcpy(mem->buf_ + mem->end_, data, data_size);
  mem->end_ += data_size;
  assert(mem->end_ <= mem->buf_size_);

  DoRemap(idec, mem->buf_ + mem->start_ - old_start);
  return 1;
}

static int RemapMemBuffer(DEDUP_WEBP_IDecoder* const idec,
                          const uint8_t* const data, size_t data_size) {
  MemBuffer* const mem = &idec->mem_;
  const uint8_t* const old_buf = mem->buf_;
  const uint8_t* const old_start = old_buf + mem->start_;
  assert(mem->mode_ == MEM_MODE_MAP);

  if (data_size < mem->buf_size_) return 0;  // can't remap to a shorter buffer!

  mem->buf_ = (uint8_t*)data;
  mem->end_ = mem->buf_size_ = data_size;

  DoRemap(idec, mem->buf_ + mem->start_ - old_start);
  return 1;
}

static void InitMemBuffer(MemBuffer* const mem) {
  mem->mode_       = MEM_MODE_NONE;
  mem->buf_        = NULL;
  mem->buf_size_   = 0;
  mem->part0_buf_  = NULL;
  mem->part0_size_ = 0;
}

static void ClearMemBuffer(MemBuffer* const mem) {
  assert(mem);
  if (mem->mode_ == MEM_MODE_APPEND) {
    DEDUP_WEBP_SafeFree(mem->buf_);
    DEDUP_WEBP_SafeFree((void*)mem->part0_buf_);
  }
}

static int CheckMemBufferMode(MemBuffer* const mem, MemBufferMode expected) {
  if (mem->mode_ == MEM_MODE_NONE) {
    mem->mode_ = expected;    // switch to the expected mode
  } else if (mem->mode_ != expected) {
    return 0;         // we mixed the modes => error
  }
  assert(mem->mode_ == expected);   // mode is ok
  return 1;
}

// To be called last.
static DEDUP_vP8_StatusCode FinishDecoding(DEDUP_WEBP_IDecoder* const idec) {
  const DEDUP_WEBP_DecoderOptions* const options = idec->params_.options;
  DEDUP_WEBP_DecBuffer* const output = idec->params_.output;

  idec->state_ = STATE_DONE;
  if (options != NULL && options->flip) {
    const DEDUP_vP8_StatusCode status = DEDUP_WEBP_FlipBuffer(output);
    if (status != DEDUP_vP8__STATUS_OK) return status;
  }
  if (idec->final_output_ != NULL) {
    DEDUP_WEBP_CopyDecBufferPixels(output, idec->final_output_);  // do the slow-copy
    DEDUP_WEBP_FreeDecBuffer(&idec->output_);
    *output = *idec->final_output_;
    idec->final_output_ = NULL;
  }
  return DEDUP_vP8__STATUS_OK;
}

//------------------------------------------------------------------------------
// Macroblock-decoding contexts

static void SaveContext(const DEDUP_vP8_Decoder* dec, const DEDUP_vP8_BitReader* token_br,
                        MBContext* const context) {
  context->left_ = dec->mb_info_[-1];
  context->info_ = dec->mb_info_[dec->mb_x_];
  context->token_br_ = *token_br;
}

static void RestoreContext(const MBContext* context, DEDUP_vP8_Decoder* const dec,
                           DEDUP_vP8_BitReader* const token_br) {
  dec->mb_info_[-1] = context->left_;
  dec->mb_info_[dec->mb_x_] = context->info_;
  *token_br = context->token_br_;
}

//------------------------------------------------------------------------------

static DEDUP_vP8_StatusCode IDecError(DEDUP_WEBP_IDecoder* const idec, DEDUP_vP8_StatusCode error) {
  if (idec->state_ == STATE_DEDUP_vP8__DATA) {
    DEDUP_vP8_Io* const io = &idec->io_;
    if (io->teardown != NULL) {
      io->teardown(io);
    }
  }
  idec->state_ = STATE_ERROR;
  return error;
}

static void ChangeState(DEDUP_WEBP_IDecoder* const idec, DecState new_state,
                        size_t consumed_bytes) {
  MemBuffer* const mem = &idec->mem_;
  idec->state_ = new_state;
  mem->start_ += consumed_bytes;
  assert(mem->start_ <= mem->end_);
  idec->io_.data = mem->buf_ + mem->start_;
  idec->io_.data_size = MemDataSize(mem);
}

// Headers
static DEDUP_vP8_StatusCode DecodeDEDUP_WEBP_Headers(DEDUP_WEBP_IDecoder* const idec) {
  MemBuffer* const mem = &idec->mem_;
  const uint8_t* data = mem->buf_ + mem->start_;
  size_t curr_size = MemDataSize(mem);
  DEDUP_vP8_StatusCode status;
  DEDUP_WEBP_HeaderStructure headers;

  headers.data = data;
  headers.data_size = curr_size;
  headers.have_all_data = 0;
  status = DEDUP_WEBP_ParseHeaders(&headers);
  if (status == DEDUP_vP8__STATUS_NOT_ENOUGH_DATA) {
    return DEDUP_vP8__STATUS_SUSPENDED;  // We haven't found a DEDUP_vP8_ chunk yet.
  } else if (status != DEDUP_vP8__STATUS_OK) {
    return IDecError(idec, status);
  }

  idec->chunk_size_ = headers.compressed_size;
  idec->is_lossless_ = headers.is_lossless;
  if (!idec->is_lossless_) {
    DEDUP_vP8_Decoder* const dec = DEDUP_vP8_New();
    if (dec == NULL) {
      return DEDUP_vP8__STATUS_OUT_OF_MEMORY;
    }
    idec->dec_ = dec;
    dec->alpha_data_ = headers.alpha_data;
    dec->alpha_data_size_ = headers.alpha_data_size;
    ChangeState(idec, STATE_DEDUP_vP8__HEADER, headers.offset);
  } else {
    DEDUP_vP8_LDecoder* const dec = DEDUP_vP8_LNew();
    if (dec == NULL) {
      return DEDUP_vP8__STATUS_OUT_OF_MEMORY;
    }
    idec->dec_ = dec;
    ChangeState(idec, STATE_DEDUP_vP8_L_HEADER, headers.offset);
  }
  return DEDUP_vP8__STATUS_OK;
}

static DEDUP_vP8_StatusCode DecodeDEDUP_vP8_FrameHeader(DEDUP_WEBP_IDecoder* const idec) {
  const uint8_t* data = idec->mem_.buf_ + idec->mem_.start_;
  const size_t curr_size = MemDataSize(&idec->mem_);
  int width, height;
  uint32_t bits;

  if (curr_size < DEDUP_vP8__FRAME_HEADER_SIZE) {
    // Not enough data bytes to extract DEDUP_vP8_ Frame Header.
    return DEDUP_vP8__STATUS_SUSPENDED;
  }
  if (!DEDUP_vP8_GetInfo(data, curr_size, idec->chunk_size_, &width, &height)) {
    return IDecError(idec, DEDUP_vP8__STATUS_BITSTREAM_ERROR);
  }

  bits = data[0] | (data[1] << 8) | (data[2] << 16);
  idec->mem_.part0_size_ = (bits >> 5) + DEDUP_vP8__FRAME_HEADER_SIZE;

  idec->io_.data = data;
  idec->io_.data_size = curr_size;
  idec->state_ = STATE_DEDUP_vP8__PARTS0;
  return DEDUP_vP8__STATUS_OK;
}

// Partition #0
static DEDUP_vP8_StatusCode CopyParts0Data(DEDUP_WEBP_IDecoder* const idec) {
  DEDUP_vP8_Decoder* const dec = (DEDUP_vP8_Decoder*)idec->dec_;
  DEDUP_vP8_BitReader* const br = &dec->br_;
  const size_t part_size = br->buf_end_ - br->buf_;
  MemBuffer* const mem = &idec->mem_;
  assert(!idec->is_lossless_);
  assert(mem->part0_buf_ == NULL);
  // the following is a format limitation, no need for runtime check:
  assert(part_size <= mem->part0_size_);
  if (part_size == 0) {   // can't have zero-size partition #0
    return DEDUP_vP8__STATUS_BITSTREAM_ERROR;
  }
  if (mem->mode_ == MEM_MODE_APPEND) {
    // We copy and grab ownership of the partition #0 data.
    uint8_t* const part0_buf = (uint8_t*)DEDUP_WEBP_SafeMalloc(1ULL, part_size);
    if (part0_buf == NULL) {
      return DEDUP_vP8__STATUS_OUT_OF_MEMORY;
    }
    memcpy(part0_buf, br->buf_, part_size);
    mem->part0_buf_ = part0_buf;
    DEDUP_vP8_BitReaderSetBuffer(br, part0_buf, part_size);
  } else {
    // Else: just keep pointers to the partition #0's data in dec_->br_.
  }
  mem->start_ += part_size;
  return DEDUP_vP8__STATUS_OK;
}

static DEDUP_vP8_StatusCode DecodePartition0(DEDUP_WEBP_IDecoder* const idec) {
  DEDUP_vP8_Decoder* const dec = (DEDUP_vP8_Decoder*)idec->dec_;
  DEDUP_vP8_Io* const io = &idec->io_;
  const DEDUP_WEBP_DecParams* const params = &idec->params_;
  DEDUP_WEBP_DecBuffer* const output = params->output;

  // Wait till we have enough data for the whole partition #0
  if (MemDataSize(&idec->mem_) < idec->mem_.part0_size_) {
    return DEDUP_vP8__STATUS_SUSPENDED;
  }

  if (!DEDUP_vP8_GetHeaders(dec, io)) {
    const DEDUP_vP8_StatusCode status = dec->status_;
    if (status == DEDUP_vP8__STATUS_SUSPENDED ||
        status == DEDUP_vP8__STATUS_NOT_ENOUGH_DATA) {
      // treating NOT_ENOUGH_DATA as SUSPENDED state
      return DEDUP_vP8__STATUS_SUSPENDED;
    }
    return IDecError(idec, status);
  }

  // Allocate/Verify output buffer now
  dec->status_ = DEDUP_WEBP_AllocateDecBuffer(io->width, io->height, params->options,
                                       output);
  if (dec->status_ != DEDUP_vP8__STATUS_OK) {
    return IDecError(idec, dec->status_);
  }
  // This change must be done before calling DEDUP_vP8_InitFrame()
  dec->mt_method_ = DEDUP_vP8_GetThreadMethod(params->options, NULL,
                                       io->width, io->height);
  DEDUP_vP8_InitDithering(params->options, dec);

  dec->status_ = CopyParts0Data(idec);
  if (dec->status_ != DEDUP_vP8__STATUS_OK) {
    return IDecError(idec, dec->status_);
  }

  // Finish setting up the decoding parameters. Will call io->setup().
  if (DEDUP_vP8_EnterCritical(dec, io) != DEDUP_vP8__STATUS_OK) {
    return IDecError(idec, dec->status_);
  }

  // Note: past this point, teardown() must always be called
  // in case of error.
  idec->state_ = STATE_DEDUP_vP8__DATA;
  // Allocate memory and prepare everything.
  if (!DEDUP_vP8_InitFrame(dec, io)) {
    return IDecError(idec, dec->status_);
  }
  return DEDUP_vP8__STATUS_OK;
}

// Remaining partitions
static DEDUP_vP8_StatusCode DecodeRemaining(DEDUP_WEBP_IDecoder* const idec) {
  DEDUP_vP8_Decoder* const dec = (DEDUP_vP8_Decoder*)idec->dec_;
  DEDUP_vP8_Io* const io = &idec->io_;

  assert(dec->ready_);
  for (; dec->mb_y_ < dec->mb_h_; ++dec->mb_y_) {
    if (idec->last_mb_y_ != dec->mb_y_) {
      if (!DEDUP_vP8_ParseIntraModeRow(&dec->br_, dec)) {
        // note: normally, error shouldn't occur since we already have the whole
        // partition0 available here in DecodeRemaining(). Reaching EOF while
        // reading intra modes really means a BITSTREAM_ERROR.
        return IDecError(idec, DEDUP_vP8__STATUS_BITSTREAM_ERROR);
      }
      idec->last_mb_y_ = dec->mb_y_;
    }
    for (; dec->mb_x_ < dec->mb_w_; ++dec->mb_x_) {
      DEDUP_vP8_BitReader* const token_br =
          &dec->parts_[dec->mb_y_ & dec->num_parts_minus_one_];
      MBContext context;
      SaveContext(dec, token_br, &context);
      if (!DEDUP_vP8_DecodeMB(dec, token_br)) {
        // We shouldn't fail when MAX_MB data was available
        if (dec->num_parts_minus_one_ == 0 &&
            MemDataSize(&idec->mem_) > MAX_MB_SIZE) {
          return IDecError(idec, DEDUP_vP8__STATUS_BITSTREAM_ERROR);
        }
        RestoreContext(&context, dec, token_br);
        return DEDUP_vP8__STATUS_SUSPENDED;
      }
      // Release buffer only if there is only one partition
      if (dec->num_parts_minus_one_ == 0) {
        idec->mem_.start_ = token_br->buf_ - idec->mem_.buf_;
        assert(idec->mem_.start_ <= idec->mem_.end_);
      }
    }
    DEDUP_vP8_InitScanline(dec);   // Prepare for next scanline

    // Reconstruct, filter and emit the row.
    if (!DEDUP_vP8_ProcessRow(dec, io)) {
      return IDecError(idec, DEDUP_vP8__STATUS_USER_ABORT);
    }
  }
  // Synchronize the thread and check for errors.
  if (!DEDUP_vP8_ExitCritical(dec, io)) {
    return IDecError(idec, DEDUP_vP8__STATUS_USER_ABORT);
  }
  dec->ready_ = 0;
  return FinishDecoding(idec);
}

static DEDUP_vP8_StatusCode ErrorStatusLossless(DEDUP_WEBP_IDecoder* const idec,
                                         DEDUP_vP8_StatusCode status) {
  if (status == DEDUP_vP8__STATUS_SUSPENDED || status == DEDUP_vP8__STATUS_NOT_ENOUGH_DATA) {
    return DEDUP_vP8__STATUS_SUSPENDED;
  }
  return IDecError(idec, status);
}

static DEDUP_vP8_StatusCode DecodeDEDUP_vP8_LHeader(DEDUP_WEBP_IDecoder* const idec) {
  DEDUP_vP8_Io* const io = &idec->io_;
  DEDUP_vP8_LDecoder* const dec = (DEDUP_vP8_LDecoder*)idec->dec_;
  const DEDUP_WEBP_DecParams* const params = &idec->params_;
  DEDUP_WEBP_DecBuffer* const output = params->output;
  size_t curr_size = MemDataSize(&idec->mem_);
  assert(idec->is_lossless_);

  // Wait until there's enough data for decoding header.
  if (curr_size < (idec->chunk_size_ >> 3)) {
    dec->status_ = DEDUP_vP8__STATUS_SUSPENDED;
    return ErrorStatusLossless(idec, dec->status_);
  }

  if (!DEDUP_vP8_LDecodeHeader(dec, io)) {
    if (dec->status_ == DEDUP_vP8__STATUS_BITSTREAM_ERROR &&
        curr_size < idec->chunk_size_) {
      dec->status_ = DEDUP_vP8__STATUS_SUSPENDED;
    }
    return ErrorStatusLossless(idec, dec->status_);
  }
  // Allocate/verify output buffer now.
  dec->status_ = DEDUP_WEBP_AllocateDecBuffer(io->width, io->height, params->options,
                                       output);
  if (dec->status_ != DEDUP_vP8__STATUS_OK) {
    return IDecError(idec, dec->status_);
  }

  idec->state_ = STATE_DEDUP_vP8_L_DATA;
  return DEDUP_vP8__STATUS_OK;
}

static DEDUP_vP8_StatusCode DecodeDEDUP_vP8_LData(DEDUP_WEBP_IDecoder* const idec) {
  DEDUP_vP8_LDecoder* const dec = (DEDUP_vP8_LDecoder*)idec->dec_;
  const size_t curr_size = MemDataSize(&idec->mem_);
  assert(idec->is_lossless_);

  // Switch to incremental decoding if we don't have all the bytes available.
  dec->incremental_ = (curr_size < idec->chunk_size_);

  if (!DEDUP_vP8_LDecodeImage(dec)) {
    return ErrorStatusLossless(idec, dec->status_);
  }
  assert(dec->status_ == DEDUP_vP8__STATUS_OK || dec->status_ == DEDUP_vP8__STATUS_SUSPENDED);
  return (dec->status_ == DEDUP_vP8__STATUS_SUSPENDED) ? dec->status_
                                                : FinishDecoding(idec);
}

  // Main decoding loop
static DEDUP_vP8_StatusCode IDecode(DEDUP_WEBP_IDecoder* idec) {
  DEDUP_vP8_StatusCode status = DEDUP_vP8__STATUS_SUSPENDED;

  if (idec->state_ == STATE_WEBP_HEADER) {
    status = DecodeDEDUP_WEBP_Headers(idec);
  } else {
    if (idec->dec_ == NULL) {
      return DEDUP_vP8__STATUS_SUSPENDED;    // can't continue if we have no decoder.
    }
  }
  if (idec->state_ == STATE_DEDUP_vP8__HEADER) {
    status = DecodeDEDUP_vP8_FrameHeader(idec);
  }
  if (idec->state_ == STATE_DEDUP_vP8__PARTS0) {
    status = DecodePartition0(idec);
  }
  if (idec->state_ == STATE_DEDUP_vP8__DATA) {
    status = DecodeRemaining(idec);
  }
  if (idec->state_ == STATE_DEDUP_vP8_L_HEADER) {
    status = DecodeDEDUP_vP8_LHeader(idec);
  }
  if (idec->state_ == STATE_DEDUP_vP8_L_DATA) {
    status = DecodeDEDUP_vP8_LData(idec);
  }
  return status;
}

//------------------------------------------------------------------------------
// Internal constructor

static DEDUP_WEBP_IDecoder* NewDecoder(DEDUP_WEBP_DecBuffer* const output_buffer,
                                const DEDUP_WEBP_BitstreamFeatures* const features) {
  DEDUP_WEBP_IDecoder* idec = (DEDUP_WEBP_IDecoder*)DEDUP_WEBP_SafeCalloc(1ULL, sizeof(*idec));
  if (idec == NULL) {
    return NULL;
  }

  idec->state_ = STATE_WEBP_HEADER;
  idec->chunk_size_ = 0;

  idec->last_mb_y_ = -1;

  InitMemBuffer(&idec->mem_);
  DEDUP_WEBP_InitDecBuffer(&idec->output_);
  DEDUP_vP8_InitIo(&idec->io_);

  DEDUP_WEBP_ResetDecParams(&idec->params_);
  if (output_buffer == NULL || DEDUP_WEBP_AvoidSlowMemory(output_buffer, features)) {
    idec->params_.output = &idec->output_;
    idec->final_output_ = output_buffer;
    if (output_buffer != NULL) {
      idec->params_.output->colorspace = output_buffer->colorspace;
    }
  } else {
    idec->params_.output = output_buffer;
    idec->final_output_ = NULL;
  }
  DEDUP_WEBP_InitCustomIo(&idec->params_, &idec->io_);  // Plug the I/O functions.

  return idec;
}

//------------------------------------------------------------------------------
// Public functions

DEDUP_WEBP_IDecoder* DEDUP_WEBP_INewDecoder(DEDUP_WEBP_DecBuffer* output_buffer) {
  return NewDecoder(output_buffer, NULL);
}

DEDUP_WEBP_IDecoder* DEDUP_WEBP_IDecode(const uint8_t* data, size_t data_size,
                          DEDUP_WEBP_DecoderConfig* config) {
  DEDUP_WEBP_IDecoder* idec;
  DEDUP_WEBP_BitstreamFeatures tmp_features;
  DEDUP_WEBP_BitstreamFeatures* const features =
      (config == NULL) ? &tmp_features : &config->input;
  memset(&tmp_features, 0, sizeof(tmp_features));

  // Parse the bitstream's features, if requested:
  if (data != NULL && data_size > 0) {
    if (DEDUP_WEBP_GetFeatures(data, data_size, features) != DEDUP_vP8__STATUS_OK) {
      return NULL;
    }
  }

  // Create an instance of the incremental decoder
  idec = (config != NULL) ? NewDecoder(&config->output, features)
                          : NewDecoder(NULL, features);
  if (idec == NULL) {
    return NULL;
  }
  // Finish initialization
  if (config != NULL) {
    idec->params_.options = &config->options;
  }
  return idec;
}

void DEDUP_WEBP_IDelete(DEDUP_WEBP_IDecoder* idec) {
  if (idec == NULL) return;
  if (idec->dec_ != NULL) {
    if (!idec->is_lossless_) {
      if (idec->state_ == STATE_DEDUP_vP8__DATA) {
        // Synchronize the thread, clean-up and check for errors.
        DEDUP_vP8_ExitCritical((DEDUP_vP8_Decoder*)idec->dec_, &idec->io_);
      }
      DEDUP_vP8_Delete((DEDUP_vP8_Decoder*)idec->dec_);
    } else {
      DEDUP_vP8_LDelete((DEDUP_vP8_LDecoder*)idec->dec_);
    }
  }
  ClearMemBuffer(&idec->mem_);
  DEDUP_WEBP_FreeDecBuffer(&idec->output_);
  DEDUP_WEBP_SafeFree(idec);
}

//------------------------------------------------------------------------------
// Wrapper toward DEDUP_WEBP_INewDecoder

DEDUP_WEBP_IDecoder* DEDUP_WEBP_INewRGB(WEBP_CSP_MODE mode, uint8_t* output_buffer,
                          size_t output_buffer_size, int output_stride) {
  const int is_external_memory = (output_buffer != NULL) ? 1 : 0;
  DEDUP_WEBP_IDecoder* idec;

  if (mode >= MODE_YUV) return NULL;
  if (is_external_memory == 0) {    // Overwrite parameters to sane values.
    output_buffer_size = 0;
    output_stride = 0;
  } else {  // A buffer was passed. Validate the other params.
    if (output_stride == 0 || output_buffer_size == 0) {
      return NULL;   // invalid parameter.
    }
  }
  idec = DEDUP_WEBP_INewDecoder(NULL);
  if (idec == NULL) return NULL;
  idec->output_.colorspace = mode;
  idec->output_.is_external_memory = is_external_memory;
  idec->output_.u.RGBA.rgba = output_buffer;
  idec->output_.u.RGBA.stride = output_stride;
  idec->output_.u.RGBA.size = output_buffer_size;
  return idec;
}

DEDUP_WEBP_IDecoder* DEDUP_WEBP_INewYUVA(uint8_t* luma, size_t luma_size, int luma_stride,
                           uint8_t* u, size_t u_size, int u_stride,
                           uint8_t* v, size_t v_size, int v_stride,
                           uint8_t* a, size_t a_size, int a_stride) {
  const int is_external_memory = (luma != NULL) ? 1 : 0;
  DEDUP_WEBP_IDecoder* idec;
  WEBP_CSP_MODE colorspace;

  if (is_external_memory == 0) {    // Overwrite parameters to sane values.
    luma_size = u_size = v_size = a_size = 0;
    luma_stride = u_stride = v_stride = a_stride = 0;
    u = v = a = NULL;
    colorspace = MODE_YUVA;
  } else {  // A luma buffer was passed. Validate the other parameters.
    if (u == NULL || v == NULL) return NULL;
    if (luma_size == 0 || u_size == 0 || v_size == 0) return NULL;
    if (luma_stride == 0 || u_stride == 0 || v_stride == 0) return NULL;
    if (a != NULL) {
      if (a_size == 0 || a_stride == 0) return NULL;
    }
    colorspace = (a == NULL) ? MODE_YUV : MODE_YUVA;
  }

  idec = DEDUP_WEBP_INewDecoder(NULL);
  if (idec == NULL) return NULL;

  idec->output_.colorspace = colorspace;
  idec->output_.is_external_memory = is_external_memory;
  idec->output_.u.YUVA.y = luma;
  idec->output_.u.YUVA.y_stride = luma_stride;
  idec->output_.u.YUVA.y_size = luma_size;
  idec->output_.u.YUVA.u = u;
  idec->output_.u.YUVA.u_stride = u_stride;
  idec->output_.u.YUVA.u_size = u_size;
  idec->output_.u.YUVA.v = v;
  idec->output_.u.YUVA.v_stride = v_stride;
  idec->output_.u.YUVA.v_size = v_size;
  idec->output_.u.YUVA.a = a;
  idec->output_.u.YUVA.a_stride = a_stride;
  idec->output_.u.YUVA.a_size = a_size;
  return idec;
}

DEDUP_WEBP_IDecoder* DEDUP_WEBP_INewYUV(uint8_t* luma, size_t luma_size, int luma_stride,
                          uint8_t* u, size_t u_size, int u_stride,
                          uint8_t* v, size_t v_size, int v_stride) {
  return DEDUP_WEBP_INewYUVA(luma, luma_size, luma_stride,
                      u, u_size, u_stride,
                      v, v_size, v_stride,
                      NULL, 0, 0);
}

//------------------------------------------------------------------------------

static DEDUP_vP8_StatusCode IDecCheckStatus(const DEDUP_WEBP_IDecoder* const idec) {
  assert(idec);
  if (idec->state_ == STATE_ERROR) {
    return DEDUP_vP8__STATUS_BITSTREAM_ERROR;
  }
  if (idec->state_ == STATE_DONE) {
    return DEDUP_vP8__STATUS_OK;
  }
  return DEDUP_vP8__STATUS_SUSPENDED;
}

DEDUP_vP8_StatusCode DEDUP_WEBP_IAppend(DEDUP_WEBP_IDecoder* idec,
                          const uint8_t* data, size_t data_size) {
  DEDUP_vP8_StatusCode status;
  if (idec == NULL || data == NULL) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }
  status = IDecCheckStatus(idec);
  if (status != DEDUP_vP8__STATUS_SUSPENDED) {
    return status;
  }
  // Check mixed calls between RemapMemBuffer and AppendToMemBuffer.
  if (!CheckMemBufferMode(&idec->mem_, MEM_MODE_APPEND)) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }
  // Append data to memory buffer
  if (!AppendToMemBuffer(idec, data, data_size)) {
    return DEDUP_vP8__STATUS_OUT_OF_MEMORY;
  }
  return IDecode(idec);
}

DEDUP_vP8_StatusCode DEDUP_WEBP_IUpdate(DEDUP_WEBP_IDecoder* idec,
                          const uint8_t* data, size_t data_size) {
  DEDUP_vP8_StatusCode status;
  if (idec == NULL || data == NULL) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }
  status = IDecCheckStatus(idec);
  if (status != DEDUP_vP8__STATUS_SUSPENDED) {
    return status;
  }
  // Check mixed calls between RemapMemBuffer and AppendToMemBuffer.
  if (!CheckMemBufferMode(&idec->mem_, MEM_MODE_MAP)) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }
  // Make the memory buffer point to the new buffer
  if (!RemapMemBuffer(idec, data, data_size)) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }
  return IDecode(idec);
}

//------------------------------------------------------------------------------

static const DEDUP_WEBP_DecBuffer* GetOutputBuffer(const DEDUP_WEBP_IDecoder* const idec) {
  if (idec == NULL || idec->dec_ == NULL) {
    return NULL;
  }
  if (idec->state_ <= STATE_DEDUP_vP8__PARTS0) {
    return NULL;
  }
  if (idec->final_output_ != NULL) {
    return NULL;   // not yet slow-copied
  }
  return idec->params_.output;
}

const DEDUP_WEBP_DecBuffer* DEDUP_WEBP_IDecodedArea(const DEDUP_WEBP_IDecoder* idec,
                                      int* left, int* top,
                                      int* width, int* height) {
  const DEDUP_WEBP_DecBuffer* const src = GetOutputBuffer(idec);
  if (left != NULL) *left = 0;
  if (top != NULL) *top = 0;
  if (src != NULL) {
    if (width != NULL) *width = src->width;
    if (height != NULL) *height = idec->params_.last_y;
  } else {
    if (width != NULL) *width = 0;
    if (height != NULL) *height = 0;
  }
  return src;
}

uint8_t* DEDUP_WEBP_IDecGetRGB(const DEDUP_WEBP_IDecoder* idec, int* last_y,
                        int* width, int* height, int* stride) {
  const DEDUP_WEBP_DecBuffer* const src = GetOutputBuffer(idec);
  if (src == NULL) return NULL;
  if (src->colorspace >= MODE_YUV) {
    return NULL;
  }

  if (last_y != NULL) *last_y = idec->params_.last_y;
  if (width != NULL) *width = src->width;
  if (height != NULL) *height = src->height;
  if (stride != NULL) *stride = src->u.RGBA.stride;

  return src->u.RGBA.rgba;
}

uint8_t* DEDUP_WEBP_IDecGetYUVA(const DEDUP_WEBP_IDecoder* idec, int* last_y,
                         uint8_t** u, uint8_t** v, uint8_t** a,
                         int* width, int* height,
                         int* stride, int* uv_stride, int* a_stride) {
  const DEDUP_WEBP_DecBuffer* const src = GetOutputBuffer(idec);
  if (src == NULL) return NULL;
  if (src->colorspace < MODE_YUV) {
    return NULL;
  }

  if (last_y != NULL) *last_y = idec->params_.last_y;
  if (u != NULL) *u = src->u.YUVA.u;
  if (v != NULL) *v = src->u.YUVA.v;
  if (a != NULL) *a = src->u.YUVA.a;
  if (width != NULL) *width = src->width;
  if (height != NULL) *height = src->height;
  if (stride != NULL) *stride = src->u.YUVA.y_stride;
  if (uv_stride != NULL) *uv_stride = src->u.YUVA.u_stride;
  if (a_stride != NULL) *a_stride = src->u.YUVA.a_stride;

  return src->u.YUVA.y;
}

int DEDUP_WEBP_ISetIOHooks(DEDUP_WEBP_IDecoder* const idec,
                    DEDUP_vP8_IoPutHook put,
                    DEDUP_vP8_IoSetupHook setup,
                    DEDUP_vP8_IoTeardownHook teardown,
                    void* user_data) {
  if (idec == NULL || idec->state_ > STATE_WEBP_HEADER) {
    return 0;
  }

  idec->io_.put = put;
  idec->io_.setup = setup;
  idec->io_.teardown = teardown;
  idec->io_.opaque = user_data;

  return 1;
}
