// Copyright 2010 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Main decoding functions for WEBP images.
//
// Author: Skal (pascal.massimino@gmail.com)

#include <stdlib.h>

#include "./vp8i.h"
#include "./vp8li.h"
#include "./webpi.h"
#include "../utils/utils.h"
#include "../webp/mux_types.h"  // ALPHA_FLAG

//------------------------------------------------------------------------------
// RIFF layout is:
//   Offset  tag
//   0...3   "RIFF" 4-byte tag
//   4...7   size of image data (including metadata) starting at offset 8
//   8...11  "WEBP"   our form-type signature
// The RIFF container (12 bytes) is followed by appropriate chunks:
//   12..15  "VP8 ": 4-bytes tags, signaling the use of DEDUP_vP8_ video format
//   16..19  size of the raw DEDUP_vP8_ image data, starting at offset 20
//   20....  the DEDUP_vP8_ bytes
// Or,
//   12..15  "VP8L": 4-bytes tags, signaling the use of DEDUP_vP8_L lossless format
//   16..19  size of the raw DEDUP_vP8_L image data, starting at offset 20
//   20....  the DEDUP_vP8_L bytes
// Or,
//   12..15  "VP8X": 4-bytes tags, describing the extended-DEDUP_vP8_ chunk.
//   16..19  size of the DEDUP_vP8_X chunk starting at offset 20.
//   20..23  DEDUP_vP8_X flags bit-map corresponding to the chunk-types present.
//   24..26  Width of the Canvas Image.
//   27..29  Height of the Canvas Image.
// There can be extra chunks after the "VP8X" chunk (ICCP, ANMF, DEDUP_vP8_, DEDUP_vP8_L,
// XMP, EXIF  ...)
// All sizes are in little-endian order.
// Note: chunk data size must be padded to multiple of 2 when written.

// Validates the RIFF container (if detected) and skips over it.
// If a RIFF container is detected, returns:
//     DEDUP_vP8__STATUS_BITSTREAM_ERROR for invalid header,
//     DEDUP_vP8__STATUS_NOT_ENOUGH_DATA for truncated data if have_all_data is true,
// and DEDUP_vP8__STATUS_OK otherwise.
// In case there are not enough bytes (partial RIFF container), return 0 for
// *riff_size. Else return the RIFF size extracted from the header.
static DEDUP_vP8_StatusCode ParseRIFF(const uint8_t** const data,
                               size_t* const data_size, int have_all_data,
                               size_t* const riff_size) {
  assert(data != NULL);
  assert(data_size != NULL);
  assert(riff_size != NULL);

  *riff_size = 0;  // Default: no RIFF present.
  if (*data_size >= RIFF_HEADER_SIZE && !memcmp(*data, "RIFF", TAG_SIZE)) {
    if (memcmp(*data + 8, "WEBP", TAG_SIZE)) {
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;  // Wrong image file signature.
    } else {
      const uint32_t size = GetLE32(*data + TAG_SIZE);
      // Check that we have at least one chunk (i.e "WEBP" + "VP8?nnnn").
      if (size < TAG_SIZE + CHUNK_HEADER_SIZE) {
        return DEDUP_vP8__STATUS_BITSTREAM_ERROR;
      }
      if (size > MAX_CHUNK_PAYLOAD) {
        return DEDUP_vP8__STATUS_BITSTREAM_ERROR;
      }
      if (have_all_data && (size > *data_size - CHUNK_HEADER_SIZE)) {
        return DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;  // Truncated bitstream.
      }
      // We have a RIFF container. Skip it.
      *riff_size = size;
      *data += RIFF_HEADER_SIZE;
      *data_size -= RIFF_HEADER_SIZE;
    }
  }
  return DEDUP_vP8__STATUS_OK;
}

// Validates the DEDUP_vP8_X header and skips over it.
// Returns DEDUP_vP8__STATUS_BITSTREAM_ERROR for invalid DEDUP_vP8_X header,
//         DEDUP_vP8__STATUS_NOT_ENOUGH_DATA in case of insufficient data, and
//         DEDUP_vP8__STATUS_OK otherwise.
// If a DEDUP_vP8_X chunk is found, found_vp8x is set to true and *width_ptr,
// *height_ptr and *flags_ptr are set to the corresponding values extracted
// from the DEDUP_vP8_X chunk.
static DEDUP_vP8_StatusCode ParseDEDUP_vP8_X(const uint8_t** const data,
                               size_t* const data_size,
                               int* const found_vp8x,
                               int* const width_ptr, int* const height_ptr,
                               uint32_t* const flags_ptr) {
  const uint32_t vp8x_size = CHUNK_HEADER_SIZE + DEDUP_vP8_X_CHUNK_SIZE;
  assert(data != NULL);
  assert(data_size != NULL);
  assert(found_vp8x != NULL);

  *found_vp8x = 0;

  if (*data_size < CHUNK_HEADER_SIZE) {
    return DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;  // Insufficient data.
  }

  if (!memcmp(*data, "VP8X", TAG_SIZE)) {
    int width, height;
    uint32_t flags;
    const uint32_t chunk_size = GetLE32(*data + TAG_SIZE);
    if (chunk_size != DEDUP_vP8_X_CHUNK_SIZE) {
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;  // Wrong chunk size.
    }

    // Verify if enough data is available to validate the DEDUP_vP8_X chunk.
    if (*data_size < vp8x_size) {
      return DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;  // Insufficient data.
    }
    flags = GetLE32(*data + 8);
    width = 1 + GetLE24(*data + 12);
    height = 1 + GetLE24(*data + 15);
    if (width * (uint64_t)height >= MAX_IMAGE_AREA) {
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;  // image is too large
    }

    if (flags_ptr != NULL) *flags_ptr = flags;
    if (width_ptr != NULL) *width_ptr = width;
    if (height_ptr != NULL) *height_ptr = height;
    // Skip over DEDUP_vP8_X header bytes.
    *data += vp8x_size;
    *data_size -= vp8x_size;
    *found_vp8x = 1;
  }
  return DEDUP_vP8__STATUS_OK;
}

// Skips to the next DEDUP_vP8_/DEDUP_vP8_L chunk header in the data given the size of the
// RIFF chunk 'riff_size'.
// Returns DEDUP_vP8__STATUS_BITSTREAM_ERROR if any invalid chunk size is encountered,
//         DEDUP_vP8__STATUS_NOT_ENOUGH_DATA in case of insufficient data, and
//         DEDUP_vP8__STATUS_OK otherwise.
// If an alpha chunk is found, *alpha_data and *alpha_size are set
// appropriately.
static DEDUP_vP8_StatusCode ParseOptionalChunks(const uint8_t** const data,
                                         size_t* const data_size,
                                         size_t const riff_size,
                                         const uint8_t** const alpha_data,
                                         size_t* const alpha_size) {
  const uint8_t* buf;
  size_t buf_size;
  uint32_t total_size = TAG_SIZE +           // "WEBP".
                        CHUNK_HEADER_SIZE +  // "VP8Xnnnn".
                        DEDUP_vP8_X_CHUNK_SIZE;     // data.
  assert(data != NULL);
  assert(data_size != NULL);
  buf = *data;
  buf_size = *data_size;

  assert(alpha_data != NULL);
  assert(alpha_size != NULL);
  *alpha_data = NULL;
  *alpha_size = 0;

  while (1) {
    uint32_t chunk_size;
    uint32_t disk_chunk_size;   // chunk_size with padding

    *data = buf;
    *data_size = buf_size;

    if (buf_size < CHUNK_HEADER_SIZE) {  // Insufficient data.
      return DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;
    }

    chunk_size = GetLE32(buf + TAG_SIZE);
    if (chunk_size > MAX_CHUNK_PAYLOAD) {
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;          // Not a valid chunk size.
    }
    // For odd-sized chunk-payload, there's one byte padding at the end.
    disk_chunk_size = (CHUNK_HEADER_SIZE + chunk_size + 1) & ~1;
    total_size += disk_chunk_size;

    // Check that total bytes skipped so far does not exceed riff_size.
    if (riff_size > 0 && (total_size > riff_size)) {
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;          // Not a valid chunk size.
    }

    // Start of a (possibly incomplete) DEDUP_vP8_/DEDUP_vP8_L chunk implies that we have
    // parsed all the optional chunks.
    // Note: This check must occur before the check 'buf_size < disk_chunk_size'
    // below to allow incomplete DEDUP_vP8_/DEDUP_vP8_L chunks.
    if (!memcmp(buf, "VP8 ", TAG_SIZE) ||
        !memcmp(buf, "VP8L", TAG_SIZE)) {
      return DEDUP_vP8__STATUS_OK;
    }

    if (buf_size < disk_chunk_size) {             // Insufficient data.
      return DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;
    }

    if (!memcmp(buf, "ALPH", TAG_SIZE)) {         // A valid ALPH header.
      *alpha_data = buf + CHUNK_HEADER_SIZE;
      *alpha_size = chunk_size;
    }

    // We have a full and valid chunk; skip it.
    buf += disk_chunk_size;
    buf_size -= disk_chunk_size;
  }
}

// Validates the DEDUP_vP8_/DEDUP_vP8_L Header ("VP8 nnnn" or "VP8L nnnn") and skips over it.
// Returns DEDUP_vP8__STATUS_BITSTREAM_ERROR for invalid (chunk larger than
//         riff_size) DEDUP_vP8_/DEDUP_vP8_L header,
//         DEDUP_vP8__STATUS_NOT_ENOUGH_DATA in case of insufficient data, and
//         DEDUP_vP8__STATUS_OK otherwise.
// If a DEDUP_vP8_/DEDUP_vP8_L chunk is found, *chunk_size is set to the total number of bytes
// extracted from the DEDUP_vP8_/DEDUP_vP8_L chunk header.
// The flag '*is_lossless' is set to 1 in case of DEDUP_vP8_L chunk / raw DEDUP_vP8_L data.
static DEDUP_vP8_StatusCode ParseDEDUP_vP8_Header(const uint8_t** const data_ptr,
                                    size_t* const data_size, int have_all_data,
                                    size_t riff_size, size_t* const chunk_size,
                                    int* const is_lossless) {
  const uint8_t* const data = *data_ptr;
  const int is_vp8 = !memcmp(data, "VP8 ", TAG_SIZE);
  const int is_vp8l = !memcmp(data, "VP8L", TAG_SIZE);
  const uint32_t minimal_size =
      TAG_SIZE + CHUNK_HEADER_SIZE;  // "WEBP" + "VP8 nnnn" OR
                                     // "WEBP" + "VP8Lnnnn"
  assert(data != NULL);
  assert(data_size != NULL);
  assert(chunk_size != NULL);
  assert(is_lossless != NULL);

  if (*data_size < CHUNK_HEADER_SIZE) {
    return DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;  // Insufficient data.
  }

  if (is_vp8 || is_vp8l) {
    // Bitstream contains DEDUP_vP8_/DEDUP_vP8_L header.
    const uint32_t size = GetLE32(data + TAG_SIZE);
    if ((riff_size >= minimal_size) && (size > riff_size - minimal_size)) {
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;  // Inconsistent size information.
    }
    if (have_all_data && (size > *data_size - CHUNK_HEADER_SIZE)) {
      return DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;  // Truncated bitstream.
    }
    // Skip over CHUNK_HEADER_SIZE bytes from DEDUP_vP8_/DEDUP_vP8_L Header.
    *chunk_size = size;
    *data_ptr += CHUNK_HEADER_SIZE;
    *data_size -= CHUNK_HEADER_SIZE;
    *is_lossless = is_vp8l;
  } else {
    // Raw DEDUP_vP8_/DEDUP_vP8_L bitstream (no header).
    *is_lossless = DEDUP_vP8_LCheckSignature(data, *data_size);
    *chunk_size = *data_size;
  }

  return DEDUP_vP8__STATUS_OK;
}

//------------------------------------------------------------------------------

// Fetch '*width', '*height', '*has_alpha' and fill out 'headers' based on
// 'data'. All the output parameters may be NULL. If 'headers' is NULL only the
// minimal amount will be read to fetch the remaining parameters.
// If 'headers' is non-NULL this function will attempt to locate both alpha
// data (with or without a DEDUP_vP8_X chunk) and the bitstream chunk (DEDUP_vP8_/DEDUP_vP8_L).
// Note: The following chunk sequences (before the raw DEDUP_vP8_/DEDUP_vP8_L data) are
// considered valid by this function:
// RIFF + DEDUP_vP8_(L)
// RIFF + DEDUP_vP8_X + (optional chunks) + DEDUP_vP8_(L)
// ALPH + DEDUP_vP8_ <-- Not a valid DEDUP_WEBP_ format: only allowed for internal purpose.
// DEDUP_vP8_(L)     <-- Not a valid DEDUP_WEBP_ format: only allowed for internal purpose.
static DEDUP_vP8_StatusCode ParseHeadersInternal(const uint8_t* data,
                                          size_t data_size,
                                          int* const width,
                                          int* const height,
                                          int* const has_alpha,
                                          int* const has_animation,
                                          int* const format,
                                          DEDUP_WEBP_HeaderStructure* const headers) {
  int canvas_width = 0;
  int canvas_height = 0;
  int image_width = 0;
  int image_height = 0;
  int found_riff = 0;
  int found_vp8x = 0;
  int animation_present = 0;
  const int have_all_data = (headers != NULL) ? headers->have_all_data : 0;

  DEDUP_vP8_StatusCode status;
  DEDUP_WEBP_HeaderStructure hdrs;

  if (data == NULL || data_size < RIFF_HEADER_SIZE) {
    return DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;
  }
  memset(&hdrs, 0, sizeof(hdrs));
  hdrs.data = data;
  hdrs.data_size = data_size;

  // Skip over RIFF header.
  status = ParseRIFF(&data, &data_size, have_all_data, &hdrs.riff_size);
  if (status != DEDUP_vP8__STATUS_OK) {
    return status;   // Wrong RIFF header / insufficient data.
  }
  found_riff = (hdrs.riff_size > 0);

  // Skip over DEDUP_vP8_X.
  {
    uint32_t flags = 0;
    status = ParseDEDUP_vP8_X(&data, &data_size, &found_vp8x,
                       &canvas_width, &canvas_height, &flags);
    if (status != DEDUP_vP8__STATUS_OK) {
      return status;  // Wrong DEDUP_vP8_X / insufficient data.
    }
    animation_present = !!(flags & ANIMATION_FLAG);
    if (!found_riff && found_vp8x) {
      // Note: This restriction may be removed in the future, if it becomes
      // necessary to send DEDUP_vP8_X chunk to the decoder.
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;
    }
    if (has_alpha != NULL) *has_alpha = !!(flags & ALPHA_FLAG);
    if (has_animation != NULL) *has_animation = animation_present;
    if (format != NULL) *format = 0;   // default = undefined

    image_width = canvas_width;
    image_height = canvas_height;
    if (found_vp8x && animation_present && headers == NULL) {
      status = DEDUP_vP8__STATUS_OK;
      goto ReturnWidthHeight;  // Just return features from DEDUP_vP8_X header.
    }
  }

  if (data_size < TAG_SIZE) {
    status = DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;
    goto ReturnWidthHeight;
  }

  // Skip over optional chunks if data started with "RIFF + DEDUP_vP8_X" or "ALPH".
  if ((found_riff && found_vp8x) ||
      (!found_riff && !found_vp8x && !memcmp(data, "ALPH", TAG_SIZE))) {
    status = ParseOptionalChunks(&data, &data_size, hdrs.riff_size,
                                 &hdrs.alpha_data, &hdrs.alpha_data_size);
    if (status != DEDUP_vP8__STATUS_OK) {
      goto ReturnWidthHeight;  // Invalid chunk size / insufficient data.
    }
  }

  // Skip over DEDUP_vP8_/DEDUP_vP8_L header.
  status = ParseDEDUP_vP8_Header(&data, &data_size, have_all_data, hdrs.riff_size,
                          &hdrs.compressed_size, &hdrs.is_lossless);
  if (status != DEDUP_vP8__STATUS_OK) {
    goto ReturnWidthHeight;  // Wrong DEDUP_vP8_/DEDUP_vP8_L chunk-header / insufficient data.
  }
  if (hdrs.compressed_size > MAX_CHUNK_PAYLOAD) {
    return DEDUP_vP8__STATUS_BITSTREAM_ERROR;
  }

  if (format != NULL && !animation_present) {
    *format = hdrs.is_lossless ? 2 : 1;
  }

  if (!hdrs.is_lossless) {
    if (data_size < DEDUP_vP8__FRAME_HEADER_SIZE) {
      status = DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;
      goto ReturnWidthHeight;
    }
    // Validates raw DEDUP_vP8_ data.
    if (!DEDUP_vP8_GetInfo(data, data_size, (uint32_t)hdrs.compressed_size,
                    &image_width, &image_height)) {
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;
    }
  } else {
    if (data_size < DEDUP_vP8_L_FRAME_HEADER_SIZE) {
      status = DEDUP_vP8__STATUS_NOT_ENOUGH_DATA;
      goto ReturnWidthHeight;
    }
    // Validates raw DEDUP_vP8_L data.
    if (!DEDUP_vP8_LGetInfo(data, data_size, &image_width, &image_height, has_alpha)) {
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;
    }
  }
  // Validates image size coherency.
  if (found_vp8x) {
    if (canvas_width != image_width || canvas_height != image_height) {
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;
    }
  }
  if (headers != NULL) {
    *headers = hdrs;
    headers->offset = data - headers->data;
    assert((uint64_t)(data - headers->data) < MAX_CHUNK_PAYLOAD);
    assert(headers->offset == headers->data_size - data_size);
  }
 ReturnWidthHeight:
  if (status == DEDUP_vP8__STATUS_OK ||
      (status == DEDUP_vP8__STATUS_NOT_ENOUGH_DATA && found_vp8x && headers == NULL)) {
    if (has_alpha != NULL) {
      // If the data did not contain a DEDUP_vP8_X/DEDUP_vP8_L chunk the only definitive way
      // to set this is by looking for alpha data (from an ALPH chunk).
      *has_alpha |= (hdrs.alpha_data != NULL);
    }
    if (width != NULL) *width = image_width;
    if (height != NULL) *height = image_height;
    return DEDUP_vP8__STATUS_OK;
  } else {
    return status;
  }
}

DEDUP_vP8_StatusCode DEDUP_WEBP_ParseHeaders(DEDUP_WEBP_HeaderStructure* const headers) {
  // status is marked volatile as a workaround for a clang-3.8 (aarch64) bug
  volatile DEDUP_vP8_StatusCode status;
  int has_animation = 0;
  assert(headers != NULL);
  // fill out headers, ignore width/height/has_alpha.
  status = ParseHeadersInternal(headers->data, headers->data_size,
                                NULL, NULL, NULL, &has_animation,
                                NULL, headers);
  if (status == DEDUP_vP8__STATUS_OK || status == DEDUP_vP8__STATUS_NOT_ENOUGH_DATA) {
    // TODO(jzern): full support of animation frames will require API additions.
    if (has_animation) {
      status = DEDUP_vP8__STATUS_UNSUPPORTED_FEATURE;
    }
  }
  return status;
}

//------------------------------------------------------------------------------
// DEDUP_WEBP_DecParams

void DEDUP_WEBP_ResetDecParams(DEDUP_WEBP_DecParams* const params) {
  if (params != NULL) {
    memset(params, 0, sizeof(*params));
  }
}

//------------------------------------------------------------------------------
// "Into" decoding variants

// Main flow
static DEDUP_vP8_StatusCode DecodeInto(const uint8_t* const data, size_t data_size,
                                DEDUP_WEBP_DecParams* const params) {
  DEDUP_vP8_StatusCode status;
  DEDUP_vP8_Io io;
  DEDUP_WEBP_HeaderStructure headers;

  headers.data = data;
  headers.data_size = data_size;
  headers.have_all_data = 1;
  status = DEDUP_WEBP_ParseHeaders(&headers);   // Process Pre-DEDUP_vP8_ chunks.
  if (status != DEDUP_vP8__STATUS_OK) {
    return status;
  }

  assert(params != NULL);
  DEDUP_vP8_InitIo(&io);
  io.data = headers.data + headers.offset;
  io.data_size = headers.data_size - headers.offset;
  DEDUP_WEBP_InitCustomIo(params, &io);  // Plug the I/O functions.

  if (!headers.is_lossless) {
    DEDUP_vP8_Decoder* const dec = DEDUP_vP8_New();
    if (dec == NULL) {
      return DEDUP_vP8__STATUS_OUT_OF_MEMORY;
    }
    dec->alpha_data_ = headers.alpha_data;
    dec->alpha_data_size_ = headers.alpha_data_size;

    // Decode bitstream header, update io->width/io->height.
    if (!DEDUP_vP8_GetHeaders(dec, &io)) {
      status = dec->status_;   // An error occurred. Grab error status.
    } else {
      // Allocate/check output buffers.
      status = DEDUP_WEBP_AllocateDecBuffer(io.width, io.height, params->options,
                                     params->output);
      if (status == DEDUP_vP8__STATUS_OK) {  // Decode
        // This change must be done before calling DEDUP_vP8_Decode()
        dec->mt_method_ = DEDUP_vP8_GetThreadMethod(params->options, &headers,
                                             io.width, io.height);
        DEDUP_vP8_InitDithering(params->options, dec);
        if (!DEDUP_vP8_Decode(dec, &io)) {
          status = dec->status_;
        }
      }
    }
    DEDUP_vP8_Delete(dec);
  } else {
    DEDUP_vP8_LDecoder* const dec = DEDUP_vP8_LNew();
    if (dec == NULL) {
      return DEDUP_vP8__STATUS_OUT_OF_MEMORY;
    }
    if (!DEDUP_vP8_LDecodeHeader(dec, &io)) {
      status = dec->status_;   // An error occurred. Grab error status.
    } else {
      // Allocate/check output buffers.
      status = DEDUP_WEBP_AllocateDecBuffer(io.width, io.height, params->options,
                                     params->output);
      if (status == DEDUP_vP8__STATUS_OK) {  // Decode
        if (!DEDUP_vP8_LDecodeImage(dec)) {
          status = dec->status_;
        }
      }
    }
    DEDUP_vP8_LDelete(dec);
  }

  if (status != DEDUP_vP8__STATUS_OK) {
    DEDUP_WEBP_FreeDecBuffer(params->output);
  } else {
    if (params->options != NULL && params->options->flip) {
      // This restores the original stride values if options->flip was used
      // during the call to DEDUP_WEBP_AllocateDecBuffer above.
      status = DEDUP_WEBP_FlipBuffer(params->output);
    }
  }
  return status;
}

// Helpers
static uint8_t* DecodeIntoRGBABuffer(WEBP_CSP_MODE colorspace,
                                     const uint8_t* const data,
                                     size_t data_size,
                                     uint8_t* const rgba,
                                     int stride, size_t size) {
  DEDUP_WEBP_DecParams params;
  DEDUP_WEBP_DecBuffer buf;
  if (rgba == NULL) {
    return NULL;
  }
  DEDUP_WEBP_InitDecBuffer(&buf);
  DEDUP_WEBP_ResetDecParams(&params);
  params.output = &buf;
  buf.colorspace    = colorspace;
  buf.u.RGBA.rgba   = rgba;
  buf.u.RGBA.stride = stride;
  buf.u.RGBA.size   = size;
  buf.is_external_memory = 1;
  if (DecodeInto(data, data_size, &params) != DEDUP_vP8__STATUS_OK) {
    return NULL;
  }
  return rgba;
}

uint8_t* DEDUP_WEBP_DecodeRGBInto(const uint8_t* data, size_t data_size,
                           uint8_t* output, size_t size, int stride) {
  return DecodeIntoRGBABuffer(MODE_RGB, data, data_size, output, stride, size);
}

uint8_t* DEDUP_WEBP_DecodeRGBAInto(const uint8_t* data, size_t data_size,
                            uint8_t* output, size_t size, int stride) {
  return DecodeIntoRGBABuffer(MODE_RGBA, data, data_size, output, stride, size);
}

uint8_t* DEDUP_WEBP_DecodeARGBInto(const uint8_t* data, size_t data_size,
                            uint8_t* output, size_t size, int stride) {
  return DecodeIntoRGBABuffer(MODE_ARGB, data, data_size, output, stride, size);
}

uint8_t* DEDUP_WEBP_DecodeBGRInto(const uint8_t* data, size_t data_size,
                           uint8_t* output, size_t size, int stride) {
  return DecodeIntoRGBABuffer(MODE_BGR, data, data_size, output, stride, size);
}

uint8_t* DEDUP_WEBP_DecodeBGRAInto(const uint8_t* data, size_t data_size,
                            uint8_t* output, size_t size, int stride) {
  return DecodeIntoRGBABuffer(MODE_BGRA, data, data_size, output, stride, size);
}

uint8_t* DEDUP_WEBP_DecodeYUVInto(const uint8_t* data, size_t data_size,
                           uint8_t* luma, size_t luma_size, int luma_stride,
                           uint8_t* u, size_t u_size, int u_stride,
                           uint8_t* v, size_t v_size, int v_stride) {
  DEDUP_WEBP_DecParams params;
  DEDUP_WEBP_DecBuffer output;
  if (luma == NULL) return NULL;
  DEDUP_WEBP_InitDecBuffer(&output);
  DEDUP_WEBP_ResetDecParams(&params);
  params.output = &output;
  output.colorspace      = MODE_YUV;
  output.u.YUVA.y        = luma;
  output.u.YUVA.y_stride = luma_stride;
  output.u.YUVA.y_size   = luma_size;
  output.u.YUVA.u        = u;
  output.u.YUVA.u_stride = u_stride;
  output.u.YUVA.u_size   = u_size;
  output.u.YUVA.v        = v;
  output.u.YUVA.v_stride = v_stride;
  output.u.YUVA.v_size   = v_size;
  output.is_external_memory = 1;
  if (DecodeInto(data, data_size, &params) != DEDUP_vP8__STATUS_OK) {
    return NULL;
  }
  return luma;
}

//------------------------------------------------------------------------------

static uint8_t* Decode(WEBP_CSP_MODE mode, const uint8_t* const data,
                       size_t data_size, int* const width, int* const height,
                       DEDUP_WEBP_DecBuffer* const keep_info) {
  DEDUP_WEBP_DecParams params;
  DEDUP_WEBP_DecBuffer output;

  DEDUP_WEBP_InitDecBuffer(&output);
  DEDUP_WEBP_ResetDecParams(&params);
  params.output = &output;
  output.colorspace = mode;

  // Retrieve (and report back) the required dimensions from bitstream.
  if (!DEDUP_WEBP_GetInfo(data, data_size, &output.width, &output.height)) {
    return NULL;
  }
  if (width != NULL) *width = output.width;
  if (height != NULL) *height = output.height;

  // Decode
  if (DecodeInto(data, data_size, &params) != DEDUP_vP8__STATUS_OK) {
    return NULL;
  }
  if (keep_info != NULL) {    // keep track of the side-info
    DEDUP_WEBP_CopyDecBuffer(&output, keep_info);
  }
  // return decoded samples (don't clear 'output'!)
  return DEDUP_WEBP_IsRGBMode(mode) ? output.u.RGBA.rgba : output.u.YUVA.y;
}

uint8_t* DEDUP_WEBP_DecodeRGB(const uint8_t* data, size_t data_size,
                       int* width, int* height) {
  return Decode(MODE_RGB, data, data_size, width, height, NULL);
}

uint8_t* DEDUP_WEBP_DecodeRGBA(const uint8_t* data, size_t data_size,
                        int* width, int* height) {
  return Decode(MODE_RGBA, data, data_size, width, height, NULL);
}

uint8_t* DEDUP_WEBP_DecodeARGB(const uint8_t* data, size_t data_size,
                        int* width, int* height) {
  return Decode(MODE_ARGB, data, data_size, width, height, NULL);
}

uint8_t* DEDUP_WEBP_DecodeBGR(const uint8_t* data, size_t data_size,
                       int* width, int* height) {
  return Decode(MODE_BGR, data, data_size, width, height, NULL);
}

uint8_t* DEDUP_WEBP_DecodeBGRA(const uint8_t* data, size_t data_size,
                        int* width, int* height) {
  return Decode(MODE_BGRA, data, data_size, width, height, NULL);
}

uint8_t* DEDUP_WEBP_DecodeYUV(const uint8_t* data, size_t data_size,
                       int* width, int* height, uint8_t** u, uint8_t** v,
                       int* stride, int* uv_stride) {
  DEDUP_WEBP_DecBuffer output;   // only to preserve the side-infos
  uint8_t* const out = Decode(MODE_YUV, data, data_size,
                              width, height, &output);

  if (out != NULL) {
    const DEDUP_WEBP_YUVABuffer* const buf = &output.u.YUVA;
    *u = buf->u;
    *v = buf->v;
    *stride = buf->y_stride;
    *uv_stride = buf->u_stride;
    assert(buf->u_stride == buf->v_stride);
  }
  return out;
}

static void DefaultFeatures(DEDUP_WEBP_BitstreamFeatures* const features) {
  assert(features != NULL);
  memset(features, 0, sizeof(*features));
}

static DEDUP_vP8_StatusCode GetFeatures(const uint8_t* const data, size_t data_size,
                                 DEDUP_WEBP_BitstreamFeatures* const features) {
  if (features == NULL || data == NULL) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }
  DefaultFeatures(features);

  // Only parse enough of the data to retrieve the features.
  return ParseHeadersInternal(data, data_size,
                              &features->width, &features->height,
                              &features->has_alpha, &features->has_animation,
                              &features->format, NULL);
}

//------------------------------------------------------------------------------
// DEDUP_WEBP_GetInfo()

int DEDUP_WEBP_GetInfo(const uint8_t* data, size_t data_size,
                int* width, int* height) {
  DEDUP_WEBP_BitstreamFeatures features;

  if (GetFeatures(data, data_size, &features) != DEDUP_vP8__STATUS_OK) {
    return 0;
  }

  if (width != NULL) {
    *width  = features.width;
  }
  if (height != NULL) {
    *height = features.height;
  }

  return 1;
}

//------------------------------------------------------------------------------
// Advance decoding API

int DEDUP_WEBP_InitDecoderConfigInternal(DEDUP_WEBP_DecoderConfig* config,
                                  int version) {
  if (WEBP_ABI_IS_INCOMPATIBLE(version, WEBP_DECODER_ABI_VERSION)) {
    return 0;   // version mismatch
  }
  if (config == NULL) {
    return 0;
  }
  memset(config, 0, sizeof(*config));
  DefaultFeatures(&config->input);
  DEDUP_WEBP_InitDecBuffer(&config->output);
  return 1;
}

DEDUP_vP8_StatusCode DEDUP_WEBP_GetFeaturesInternal(const uint8_t* data, size_t data_size,
                                      DEDUP_WEBP_BitstreamFeatures* features,
                                      int version) {
  if (WEBP_ABI_IS_INCOMPATIBLE(version, WEBP_DECODER_ABI_VERSION)) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;   // version mismatch
  }
  if (features == NULL) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }
  return GetFeatures(data, data_size, features);
}

DEDUP_vP8_StatusCode DEDUP_WEBP_Decode(const uint8_t* data, size_t data_size,
                         DEDUP_WEBP_DecoderConfig* config) {
  DEDUP_WEBP_DecParams params;
  DEDUP_vP8_StatusCode status;

  if (config == NULL) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }

  status = GetFeatures(data, data_size, &config->input);
  if (status != DEDUP_vP8__STATUS_OK) {
    if (status == DEDUP_vP8__STATUS_NOT_ENOUGH_DATA) {
      return DEDUP_vP8__STATUS_BITSTREAM_ERROR;  // Not-enough-data treated as error.
    }
    return status;
  }

  DEDUP_WEBP_ResetDecParams(&params);
  params.options = &config->options;
  params.output = &config->output;
  if (DEDUP_WEBP_AvoidSlowMemory(params.output, &config->input)) {
    // decoding to slow memory: use a temporary in-mem buffer to decode into.
    DEDUP_WEBP_DecBuffer in_mem_buffer;
    DEDUP_WEBP_InitDecBuffer(&in_mem_buffer);
    in_mem_buffer.colorspace = config->output.colorspace;
    in_mem_buffer.width = config->input.width;
    in_mem_buffer.height = config->input.height;
    params.output = &in_mem_buffer;
    status = DecodeInto(data, data_size, &params);
    if (status == DEDUP_vP8__STATUS_OK) {  // do the slow-copy
      status = DEDUP_WEBP_CopyDecBufferPixels(&in_mem_buffer, &config->output);
    }
    DEDUP_WEBP_FreeDecBuffer(&in_mem_buffer);
  } else {
    status = DecodeInto(data, data_size, &params);
  }

  return status;
}

//------------------------------------------------------------------------------
// Cropping and rescaling.

int DEDUP_WEBP_IoInitFromOptions(const DEDUP_WEBP_DecoderOptions* const options,
                          DEDUP_vP8_Io* const io, WEBP_CSP_MODE src_colorspace) {
  const int W = io->width;
  const int H = io->height;
  int x = 0, y = 0, w = W, h = H;

  // Cropping
  io->use_cropping = (options != NULL) && (options->use_cropping > 0);
  if (io->use_cropping) {
    w = options->crop_width;
    h = options->crop_height;
    x = options->crop_left;
    y = options->crop_top;
    if (!DEDUP_WEBP_IsRGBMode(src_colorspace)) {   // only snap for YUV420
      x &= ~1;
      y &= ~1;
    }
    if (x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > W || y + h > H) {
      return 0;  // out of frame boundary error
    }
  }
  io->crop_left   = x;
  io->crop_top    = y;
  io->crop_right  = x + w;
  io->crop_bottom = y + h;
  io->mb_w = w;
  io->mb_h = h;

  // Scaling
  io->use_scaling = (options != NULL) && (options->use_scaling > 0);
  if (io->use_scaling) {
    int scaled_width = options->scaled_width;
    int scaled_height = options->scaled_height;
    if (!DEDUP_WEBP_RescalerGetScaledDimensions(w, h, &scaled_width, &scaled_height)) {
      return 0;
    }
    io->scaled_width = scaled_width;
    io->scaled_height = scaled_height;
  }

  // Filter
  io->bypass_filtering = (options != NULL) && options->bypass_filtering;

  // Fancy upsampler
#ifdef FANCY_UPSAMPLING
  io->fancy_upsampling = (options == NULL) || (!options->no_fancy_upsampling);
#endif

  if (io->use_scaling) {
    // disable filter (only for large downscaling ratio).
    io->bypass_filtering = (io->scaled_width < W * 3 / 4) &&
                           (io->scaled_height < H * 3 / 4);
    io->fancy_upsampling = 0;
  }
  return 1;
}

//------------------------------------------------------------------------------
