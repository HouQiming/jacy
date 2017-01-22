// Copyright 2010 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// main entry for the decoder
//
// Author: Skal (pascal.massimino@gmail.com)

#include <stdlib.h>

#include "./alphai.h"
#include "./vp8i.h"
#include "./vp8li.h"
#include "./webpi.h"
#include "../utils/bit_reader_inl.h"
#include "../utils/utils.h"

//------------------------------------------------------------------------------

int DEDUP_WEBP_GetDecoderVersion(void) {
  return (DEC_MAJ_VERSION << 16) | (DEC_MIN_VERSION << 8) | DEC_REV_VERSION;
}

//------------------------------------------------------------------------------
// DEDUP_vP8_Decoder

static void SetOk(DEDUP_vP8_Decoder* const dec) {
  dec->status_ = DEDUP_vP8__STATUS_OK;
  dec->error_msg_ = "OK";
}

int DEDUP_vP8_InitIoInternal(DEDUP_vP8_Io* const io, int version) {
  if (WEBP_ABI_IS_INCOMPATIBLE(version, WEBP_DECODER_ABI_VERSION)) {
    return 0;  // mismatch error
  }
  if (io != NULL) {
    memset(io, 0, sizeof(*io));
  }
  return 1;
}

DEDUP_vP8_Decoder* DEDUP_vP8_New(void) {
  DEDUP_vP8_Decoder* const dec = (DEDUP_vP8_Decoder*)DEDUP_WEBP_SafeCalloc(1ULL, sizeof(*dec));
  if (dec != NULL) {
    SetOk(dec);
    DEDUP_WEBP_GetWorkerInterface()->Init(&dec->worker_);
    dec->ready_ = 0;
    dec->num_parts_minus_one_ = 0;
  }
  return dec;
}

DEDUP_vP8_StatusCode DEDUP_vP8_Status(DEDUP_vP8_Decoder* const dec) {
  if (!dec) return DEDUP_vP8__STATUS_INVALID_PARAM;
  return dec->status_;
}

const char* DEDUP_vP8_StatusMessage(DEDUP_vP8_Decoder* const dec) {
  if (dec == NULL) return "no object";
  if (!dec->error_msg_) return "OK";
  return dec->error_msg_;
}

void DEDUP_vP8_Delete(DEDUP_vP8_Decoder* const dec) {
  if (dec != NULL) {
    DEDUP_vP8_Clear(dec);
    DEDUP_WEBP_SafeFree(dec);
  }
}

int DEDUP_vP8_SetError(DEDUP_vP8_Decoder* const dec,
                DEDUP_vP8_StatusCode error, const char* const msg) {
  // The oldest error reported takes precedence over the new one.
  if (dec->status_ == DEDUP_vP8__STATUS_OK) {
    dec->status_ = error;
    dec->error_msg_ = msg;
    dec->ready_ = 0;
  }
  return 0;
}

//------------------------------------------------------------------------------

int DEDUP_vP8_CheckSignature(const uint8_t* const data, size_t data_size) {
  return (data_size >= 3 &&
          data[0] == 0x9d && data[1] == 0x01 && data[2] == 0x2a);
}

int DEDUP_vP8_GetInfo(const uint8_t* data, size_t data_size, size_t chunk_size,
               int* const width, int* const height) {
  if (data == NULL || data_size < DEDUP_vP8__FRAME_HEADER_SIZE) {
    return 0;         // not enough data
  }
  // check signature
  if (!DEDUP_vP8_CheckSignature(data + 3, data_size - 3)) {
    return 0;         // Wrong signature.
  } else {
    const uint32_t bits = data[0] | (data[1] << 8) | (data[2] << 16);
    const int key_frame = !(bits & 1);
    const int w = ((data[7] << 8) | data[6]) & 0x3fff;
    const int h = ((data[9] << 8) | data[8]) & 0x3fff;

    if (!key_frame) {   // Not a keyframe.
      return 0;
    }

    if (((bits >> 1) & 7) > 3) {
      return 0;         // unknown profile
    }
    if (!((bits >> 4) & 1)) {
      return 0;         // first frame is invisible!
    }
    if (((bits >> 5)) >= chunk_size) {  // partition_length
      return 0;         // inconsistent size information.
    }
    if (w == 0 || h == 0) {
      return 0;         // We don't support both width and height to be zero.
    }

    if (width) {
      *width = w;
    }
    if (height) {
      *height = h;
    }

    return 1;
  }
}

//------------------------------------------------------------------------------
// Header parsing

static void ResetSegmentHeader(DEDUP_vP8_SegmentHeader* const hdr) {
  assert(hdr != NULL);
  hdr->use_segment_ = 0;
  hdr->update_map_ = 0;
  hdr->absolute_delta_ = 1;
  memset(hdr->quantizer_, 0, sizeof(hdr->quantizer_));
  memset(hdr->filter_strength_, 0, sizeof(hdr->filter_strength_));
}

// Paragraph 9.3
static int ParseSegmentHeader(DEDUP_vP8_BitReader* br,
                              DEDUP_vP8_SegmentHeader* hdr, DEDUP_vP8_Proba* proba) {
  assert(br != NULL);
  assert(hdr != NULL);
  hdr->use_segment_ = DEDUP_vP8_Get(br);
  if (hdr->use_segment_) {
    hdr->update_map_ = DEDUP_vP8_Get(br);
    if (DEDUP_vP8_Get(br)) {   // update data
      int s;
      hdr->absolute_delta_ = DEDUP_vP8_Get(br);
      for (s = 0; s < NUM_MB_SEGMENTS; ++s) {
        hdr->quantizer_[s] = DEDUP_vP8_Get(br) ? DEDUP_vP8_GetSignedValue(br, 7) : 0;
      }
      for (s = 0; s < NUM_MB_SEGMENTS; ++s) {
        hdr->filter_strength_[s] = DEDUP_vP8_Get(br) ? DEDUP_vP8_GetSignedValue(br, 6) : 0;
      }
    }
    if (hdr->update_map_) {
      int s;
      for (s = 0; s < MB_FEATURE_TREE_PROBS; ++s) {
        proba->segments_[s] = DEDUP_vP8_Get(br) ? DEDUP_vP8_GetValue(br, 8) : 255u;
      }
    }
  } else {
    hdr->update_map_ = 0;
  }
  return !br->eof_;
}

// Paragraph 9.5
// This function returns DEDUP_vP8__STATUS_SUSPENDED if we don't have all the
// necessary data in 'buf'.
// This case is not necessarily an error (for incremental decoding).
// Still, no bitreader is ever initialized to make it possible to read
// unavailable memory.
// If we don't even have the partitions' sizes, than DEDUP_vP8__STATUS_NOT_ENOUGH_DATA
// is returned, and this is an unrecoverable error.
// If the partitions were positioned ok, DEDUP_vP8__STATUS_OK is returned.
static DEDUP_vP8_StatusCode ParsePartitions(DEDUP_vP8_Decoder* const dec,
                                     const uint8_t* buf, size_t size) {
  DEDUP_vP8_BitReader* const br = &dec->br_;
  const uint8_t* sz = buf;
  const uint8_t* buf_end = buf + size;
  const uint8_t* part_start;
  size_t size_left = size;
  size_t last_part;
  size_t p;

  dec->num_parts_minus_one_ = (1 << DEDUP_vP8_GetValue(br, 2)) - 1;
  last_part = dec->num_parts_minus_one_;
  if (size < 3 * last_part) {
    // we can't even read the sizes with sz[]! That's a failure.
    return DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;
  }
  part_start = buf + last_part * 3;
  size_left -= last_part * 3;
  for (p = 0; p < last_part; ++p) {
    size_t psize = sz[0] | (sz[1] << 8) | (sz[2] << 16);
    if (psize > size_left) psize = size_left;
    DEDUP_vP8_InitBitReader(dec->parts_ + p, part_start, psize);
    part_start += psize;
    size_left -= psize;
    sz += 3;
  }
  DEDUP_vP8_InitBitReader(dec->parts_ + last_part, part_start, size_left);
  return (part_start < buf_end) ? DEDUP_vP8__STATUS_OK :
           DEDUP_vP8__STATUS_SUSPENDED;   // Init is ok, but there's not enough data
}

// Paragraph 9.4
static int ParseFilterHeader(DEDUP_vP8_BitReader* br, DEDUP_vP8_Decoder* const dec) {
  DEDUP_vP8_FilterHeader* const hdr = &dec->filter_hdr_;
  hdr->simple_    = DEDUP_vP8_Get(br);
  hdr->level_     = DEDUP_vP8_GetValue(br, 6);
  hdr->sharpness_ = DEDUP_vP8_GetValue(br, 3);
  hdr->use_lf_delta_ = DEDUP_vP8_Get(br);
  if (hdr->use_lf_delta_) {
    if (DEDUP_vP8_Get(br)) {   // update lf-delta?
      int i;
      for (i = 0; i < NUM_REF_LF_DELTAS; ++i) {
        if (DEDUP_vP8_Get(br)) {
          hdr->ref_lf_delta_[i] = DEDUP_vP8_GetSignedValue(br, 6);
        }
      }
      for (i = 0; i < NUM_MODE_LF_DELTAS; ++i) {
        if (DEDUP_vP8_Get(br)) {
          hdr->mode_lf_delta_[i] = DEDUP_vP8_GetSignedValue(br, 6);
        }
      }
    }
  }
  dec->filter_type_ = (hdr->level_ == 0) ? 0 : hdr->simple_ ? 1 : 2;
  return !br->eof_;
}

// Topmost call
int DEDUP_vP8_GetHeaders(DEDUP_vP8_Decoder* const dec, DEDUP_vP8_Io* const io) {
  const uint8_t* buf;
  size_t buf_size;
  DEDUP_vP8_FrameHeader* frm_hdr;
  DEDUP_vP8_PictureHeader* pic_hdr;
  DEDUP_vP8_BitReader* br;
  DEDUP_vP8_StatusCode status;

  if (dec == NULL) {
    return 0;
  }
  SetOk(dec);
  if (io == NULL) {
    return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_INVALID_PARAM,
                       "null DEDUP_vP8_Io passed to DEDUP_vP8_GetHeaders()");
  }
  buf = io->data;
  buf_size = io->data_size;
  if (buf_size < 4) {
    return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_NOT_ENOUGH_DATA,
                       "Truncated header.");
  }

  // Paragraph 9.1
  {
    const uint32_t bits = buf[0] | (buf[1] << 8) | (buf[2] << 16);
    frm_hdr = &dec->frm_hdr_;
    frm_hdr->key_frame_ = !(bits & 1);
    frm_hdr->profile_ = (bits >> 1) & 7;
    frm_hdr->show_ = (bits >> 4) & 1;
    frm_hdr->partition_length_ = (bits >> 5);
    if (frm_hdr->profile_ > 3) {
      return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_BITSTREAM_ERROR,
                         "Incorrect keyframe parameters.");
    }
    if (!frm_hdr->show_) {
      return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_UNSUPPORTED_FEATURE,
                         "Frame not displayable.");
    }
    buf += 3;
    buf_size -= 3;
  }

  pic_hdr = &dec->pic_hdr_;
  if (frm_hdr->key_frame_) {
    // Paragraph 9.2
    if (buf_size < 7) {
      return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_NOT_ENOUGH_DATA,
                         "cannot parse picture header");
    }
    if (!DEDUP_vP8_CheckSignature(buf, buf_size)) {
      return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_BITSTREAM_ERROR,
                         "Bad code word");
    }
    pic_hdr->width_ = ((buf[4] << 8) | buf[3]) & 0x3fff;
    pic_hdr->xscale_ = buf[4] >> 6;   // ratio: 1, 5/4 5/3 or 2
    pic_hdr->height_ = ((buf[6] << 8) | buf[5]) & 0x3fff;
    pic_hdr->yscale_ = buf[6] >> 6;
    buf += 7;
    buf_size -= 7;

    dec->mb_w_ = (pic_hdr->width_ + 15) >> 4;
    dec->mb_h_ = (pic_hdr->height_ + 15) >> 4;

    // Setup default output area (can be later modified during io->setup())
    io->width = pic_hdr->width_;
    io->height = pic_hdr->height_;
    // IMPORTANT! use some sane dimensions in crop_* and scaled_* fields.
    // So they can be used interchangeably without always testing for
    // 'use_cropping'.
    io->use_cropping = 0;
    io->crop_top  = 0;
    io->crop_left = 0;
    io->crop_right  = io->width;
    io->crop_bottom = io->height;
    io->use_scaling  = 0;
    io->scaled_width = io->width;
    io->scaled_height = io->height;

    io->mb_w = io->width;   // sanity check
    io->mb_h = io->height;  // ditto

    DEDUP_vP8_ResetProba(&dec->proba_);
    ResetSegmentHeader(&dec->segment_hdr_);
  }

  // Check if we have all the partition #0 available, and initialize dec->br_
  // to read this partition (and this partition only).
  if (frm_hdr->partition_length_ > buf_size) {
    return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_NOT_ENOUGH_DATA,
                       "bad partition length");
  }

  br = &dec->br_;
  DEDUP_vP8_InitBitReader(br, buf, frm_hdr->partition_length_);
  buf += frm_hdr->partition_length_;
  buf_size -= frm_hdr->partition_length_;

  if (frm_hdr->key_frame_) {
    pic_hdr->colorspace_ = DEDUP_vP8_Get(br);
    pic_hdr->clamp_type_ = DEDUP_vP8_Get(br);
  }
  if (!ParseSegmentHeader(br, &dec->segment_hdr_, &dec->proba_)) {
    return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_BITSTREAM_ERROR,
                       "cannot parse segment header");
  }
  // Filter specs
  if (!ParseFilterHeader(br, dec)) {
    return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_BITSTREAM_ERROR,
                       "cannot parse filter header");
  }
  status = ParsePartitions(dec, buf, buf_size);
  if (status != DEDUP_vP8__STATUS_OK) {
    return DEDUP_vP8_SetError(dec, status, "cannot parse partitions");
  }

  // quantizer change
  DEDUP_vP8_ParseQuant(dec);

  // Frame buffer marking
  if (!frm_hdr->key_frame_) {
    return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_UNSUPPORTED_FEATURE,
                       "Not a key frame.");
  }

  DEDUP_vP8_Get(br);   // ignore the value of update_proba_

  DEDUP_vP8_ParseProba(br, dec);

  // sanitized state
  dec->ready_ = 1;
  return 1;
}

//------------------------------------------------------------------------------
// Residual decoding (Paragraph 13.2 / 13.3)

static const uint8_t kCat3[] = { 173, 148, 140, 0 };
static const uint8_t kCat4[] = { 176, 155, 140, 135, 0 };
static const uint8_t kCat5[] = { 180, 157, 141, 134, 130, 0 };
static const uint8_t kCat6[] =
  { 254, 254, 243, 230, 196, 177, 153, 140, 133, 130, 129, 0 };
static const uint8_t* const kCat3456[] = { kCat3, kCat4, kCat5, kCat6 };
static const uint8_t kZigzag[16] = {
  0, 1, 4, 8,  5, 2, 3, 6,  9, 12, 13, 10,  7, 11, 14, 15
};

// See section 13-2: http://tools.ietf.org/html/rfc6386#section-13.2
static int GetLargeValue(DEDUP_vP8_BitReader* const br, const uint8_t* const p) {
  int v;
  if (!DEDUP_vP8_GetBit(br, p[3])) {
    if (!DEDUP_vP8_GetBit(br, p[4])) {
      v = 2;
    } else {
      v = 3 + DEDUP_vP8_GetBit(br, p[5]);
    }
  } else {
    if (!DEDUP_vP8_GetBit(br, p[6])) {
      if (!DEDUP_vP8_GetBit(br, p[7])) {
        v = 5 + DEDUP_vP8_GetBit(br, 159);
      } else {
        v = 7 + 2 * DEDUP_vP8_GetBit(br, 165);
        v += DEDUP_vP8_GetBit(br, 145);
      }
    } else {
      const uint8_t* tab;
      const int bit1 = DEDUP_vP8_GetBit(br, p[8]);
      const int bit0 = DEDUP_vP8_GetBit(br, p[9 + bit1]);
      const int cat = 2 * bit1 + bit0;
      v = 0;
      for (tab = kCat3456[cat]; *tab; ++tab) {
        v += v + DEDUP_vP8_GetBit(br, *tab);
      }
      v += 3 + (8 << cat);
    }
  }
  return v;
}

// Returns the position of the last non-zero coeff plus one
static int GetCoeffs(DEDUP_vP8_BitReader* const br, const DEDUP_vP8_BandProbas* const prob[],
                     int ctx, const quant_t dq, int n, int16_t* out) {
  const uint8_t* p = prob[n]->probas_[ctx];
  for (; n < 16; ++n) {
    if (!DEDUP_vP8_GetBit(br, p[0])) {
      return n;  // previous coeff was last non-zero coeff
    }
    while (!DEDUP_vP8_GetBit(br, p[1])) {       // sequence of zero coeffs
      p = prob[++n]->probas_[0];
      if (n == 16) return 16;
    }
    {        // non zero coeff
      const DEDUP_vP8_ProbaArray* const p_ctx = &prob[n + 1]->probas_[0];
      int v;
      if (!DEDUP_vP8_GetBit(br, p[2])) {
        v = 1;
        p = p_ctx[1];
      } else {
        v = GetLargeValue(br, p);
        p = p_ctx[2];
      }
      out[kZigzag[n]] = DEDUP_vP8_GetSigned(br, v) * dq[n > 0];
    }
  }
  return 16;
}

static WEBP_INLINE uint32_t NzCodeBits(uint32_t nz_coeffs, int nz, int dc_nz) {
  nz_coeffs <<= 2;
  nz_coeffs |= (nz > 3) ? 3 : (nz > 1) ? 2 : dc_nz;
  return nz_coeffs;
}

static int ParseResiduals(DEDUP_vP8_Decoder* const dec,
                          DEDUP_vP8_MB* const mb, DEDUP_vP8_BitReader* const token_br) {
  const DEDUP_vP8_BandProbas* (* const bands)[16 + 1] = dec->proba_.bands_ptr_;
  const DEDUP_vP8_BandProbas* const * ac_proba;
  DEDUP_vP8_MBData* const block = dec->mb_data_ + dec->mb_x_;
  const DEDUP_vP8_QuantMatrix* const q = &dec->dqm_[block->segment_];
  int16_t* dst = block->coeffs_;
  DEDUP_vP8_MB* const left_mb = dec->mb_info_ - 1;
  uint8_t tnz, lnz;
  uint32_t non_zero_y = 0;
  uint32_t non_zero_uv = 0;
  int x, y, ch;
  uint32_t out_t_nz, out_l_nz;
  int first;

  memset(dst, 0, 384 * sizeof(*dst));
  if (!block->is_i4x4_) {    // parse DC
    int16_t dc[16] = { 0 };
    const int ctx = mb->nz_dc_ + left_mb->nz_dc_;
    const int nz = GetCoeffs(token_br, bands[1], ctx, q->y2_mat_, 0, dc);
    mb->nz_dc_ = left_mb->nz_dc_ = (nz > 0);
    if (nz > 1) {   // more than just the DC -> perform the full transform
      DEDUP_vP8_TransformWHT(dc, dst);
    } else {        // only DC is non-zero -> inlined simplified transform
      int i;
      const int dc0 = (dc[0] + 3) >> 3;
      for (i = 0; i < 16 * 16; i += 16) dst[i] = dc0;
    }
    first = 1;
    ac_proba = bands[0];
  } else {
    first = 0;
    ac_proba = bands[3];
  }

  tnz = mb->nz_ & 0x0f;
  lnz = left_mb->nz_ & 0x0f;
  for (y = 0; y < 4; ++y) {
    int l = lnz & 1;
    uint32_t nz_coeffs = 0;
    for (x = 0; x < 4; ++x) {
      const int ctx = l + (tnz & 1);
      const int nz = GetCoeffs(token_br, ac_proba, ctx, q->y1_mat_, first, dst);
      l = (nz > first);
      tnz = (tnz >> 1) | (l << 7);
      nz_coeffs = NzCodeBits(nz_coeffs, nz, dst[0] != 0);
      dst += 16;
    }
    tnz >>= 4;
    lnz = (lnz >> 1) | (l << 7);
    non_zero_y = (non_zero_y << 8) | nz_coeffs;
  }
  out_t_nz = tnz;
  out_l_nz = lnz >> 4;

  for (ch = 0; ch < 4; ch += 2) {
    uint32_t nz_coeffs = 0;
    tnz = mb->nz_ >> (4 + ch);
    lnz = left_mb->nz_ >> (4 + ch);
    for (y = 0; y < 2; ++y) {
      int l = lnz & 1;
      for (x = 0; x < 2; ++x) {
        const int ctx = l + (tnz & 1);
        const int nz = GetCoeffs(token_br, bands[2], ctx, q->uv_mat_, 0, dst);
        l = (nz > 0);
        tnz = (tnz >> 1) | (l << 3);
        nz_coeffs = NzCodeBits(nz_coeffs, nz, dst[0] != 0);
        dst += 16;
      }
      tnz >>= 2;
      lnz = (lnz >> 1) | (l << 5);
    }
    // Note: we don't really need the per-4x4 details for U/V blocks.
    non_zero_uv |= nz_coeffs << (4 * ch);
    out_t_nz |= (tnz << 4) << ch;
    out_l_nz |= (lnz & 0xf0) << ch;
  }
  mb->nz_ = out_t_nz;
  left_mb->nz_ = out_l_nz;

  block->non_zero_y_ = non_zero_y;
  block->non_zero_uv_ = non_zero_uv;

  // We look at the mode-code of each block and check if some blocks have less
  // than three non-zero coeffs (code < 2). This is to avoid dithering flat and
  // empty blocks.
  block->dither_ = (non_zero_uv & 0xaaaa) ? 0 : q->dither_;

  return !(non_zero_y | non_zero_uv);  // will be used for further optimization
}

//------------------------------------------------------------------------------
// Main loop

int DEDUP_vP8_DecodeMB(DEDUP_vP8_Decoder* const dec, DEDUP_vP8_BitReader* const token_br) {
  DEDUP_vP8_MB* const left = dec->mb_info_ - 1;
  DEDUP_vP8_MB* const mb = dec->mb_info_ + dec->mb_x_;
  DEDUP_vP8_MBData* const block = dec->mb_data_ + dec->mb_x_;
  int skip = dec->use_skip_proba_ ? block->skip_ : 0;

  if (!skip) {
    skip = ParseResiduals(dec, mb, token_br);
  } else {
    left->nz_ = mb->nz_ = 0;
    if (!block->is_i4x4_) {
      left->nz_dc_ = mb->nz_dc_ = 0;
    }
    block->non_zero_y_ = 0;
    block->non_zero_uv_ = 0;
    block->dither_ = 0;
  }

  if (dec->filter_type_ > 0) {  // store filter info
    DEDUP_vP8_FInfo* const finfo = dec->f_info_ + dec->mb_x_;
    *finfo = dec->fstrengths_[block->segment_][block->is_i4x4_];
    finfo->f_inner_ |= !skip;
  }

  return !token_br->eof_;
}

void DEDUP_vP8_InitScanline(DEDUP_vP8_Decoder* const dec) {
  DEDUP_vP8_MB* const left = dec->mb_info_ - 1;
  left->nz_ = 0;
  left->nz_dc_ = 0;
  memset(dec->intra_l_, B_DC_PRED, sizeof(dec->intra_l_));
  dec->mb_x_ = 0;
}

static int ParseFrame(DEDUP_vP8_Decoder* const dec, DEDUP_vP8_Io* io) {
  for (dec->mb_y_ = 0; dec->mb_y_ < dec->br_mb_y_; ++dec->mb_y_) {
    // Parse bitstream for this row.
    DEDUP_vP8_BitReader* const token_br =
        &dec->parts_[dec->mb_y_ & dec->num_parts_minus_one_];
    if (!DEDUP_vP8_ParseIntraModeRow(&dec->br_, dec)) {
      return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_NOT_ENOUGH_DATA,
                         "Premature end-of-partition0 encountered.");
    }
    for (; dec->mb_x_ < dec->mb_w_; ++dec->mb_x_) {
      if (!DEDUP_vP8_DecodeMB(dec, token_br)) {
        return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_NOT_ENOUGH_DATA,
                           "Premature end-of-file encountered.");
      }
    }
    DEDUP_vP8_InitScanline(dec);   // Prepare for next scanline

    // Reconstruct, filter and emit the row.
    if (!DEDUP_vP8_ProcessRow(dec, io)) {
      return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_USER_ABORT, "Output aborted.");
    }
  }
  if (dec->mt_method_ > 0) {
    if (!DEDUP_WEBP_GetWorkerInterface()->Sync(&dec->worker_)) return 0;
  }

  return 1;
}

// Main entry point
int DEDUP_vP8_Decode(DEDUP_vP8_Decoder* const dec, DEDUP_vP8_Io* const io) {
  int ok = 0;
  if (dec == NULL) {
    return 0;
  }
  if (io == NULL) {
    return DEDUP_vP8_SetError(dec, DEDUP_vP8__STATUS_INVALID_PARAM,
                       "NULL DEDUP_vP8_Io parameter in DEDUP_vP8_Decode().");
  }

  if (!dec->ready_) {
    if (!DEDUP_vP8_GetHeaders(dec, io)) {
      return 0;
    }
  }
  assert(dec->ready_);

  // Finish setting up the decoding parameter. Will call io->setup().
  ok = (DEDUP_vP8_EnterCritical(dec, io) == DEDUP_vP8__STATUS_OK);
  if (ok) {   // good to go.
    // Will allocate memory and prepare everything.
    if (ok) ok = DEDUP_vP8_InitFrame(dec, io);

    // Main decoding loop
    if (ok) ok = ParseFrame(dec, io);

    // Exit.
    ok &= DEDUP_vP8_ExitCritical(dec, io);
  }

  if (!ok) {
    DEDUP_vP8_Clear(dec);
    return 0;
  }

  dec->ready_ = 0;
  return ok;
}

void DEDUP_vP8_Clear(DEDUP_vP8_Decoder* const dec) {
  if (dec == NULL) {
    return;
  }
  DEDUP_WEBP_GetWorkerInterface()->End(&dec->worker_);
  DEDUP_WEBP_DeallocateAlphaMemory(dec);
  DEDUP_WEBP_SafeFree(dec->mem_);
  dec->mem_ = NULL;
  dec->mem_size_ = 0;
  memset(&dec->br_, 0, sizeof(dec->br_));
  dec->ready_ = 0;
}

//------------------------------------------------------------------------------
