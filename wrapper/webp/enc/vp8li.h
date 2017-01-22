// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Lossless encoder: internal header.
//
// Author: Vikas Arora (vikaas.arora@gmail.com)

#ifndef WEBP_ENC_DEDUP_vP8_LI_H_
#define WEBP_ENC_DEDUP_vP8_LI_H_

#include "./backward_references.h"
#include "./histogram.h"
#include "../utils/bit_writer.h"
#include "../webp/encode.h"
#include "../webp/format_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const DEDUP_WEBP_Config* config_;      // user configuration and parameters
  const DEDUP_WEBP_Picture* pic_;        // input picture.

  uint32_t* argb_;                // Transformed argb image data.
  uint32_t* argb_scratch_;        // Scratch memory for argb rows
                                  // (used for prediction).
  uint32_t* transform_data_;      // Scratch memory for transform data.
  uint32_t* transform_mem_;       // Currently allocated memory.
  size_t    transform_mem_size_;  // Currently allocated memory size.

  int       current_width_;       // Corresponds to packed image width.

  // Encoding parameters derived from quality parameter.
  int histo_bits_;
  int transform_bits_;
  int cache_bits_;        // If equal to 0, don't use color cache.

  // Encoding parameters derived from image characteristics.
  int use_cross_color_;
  int use_subtract_green_;
  int use_predict_;
  int use_palette_;
  int palette_size_;
  uint32_t palette_[MAX_PALETTE_SIZE];

  // Some 'scratch' (potentially large) objects.
  struct DEDUP_vP8_LBackwardRefs refs_[2];  // Backward Refs array corresponding to
                                     // LZ77 & RLE coding.
  DEDUP_vP8_LHashChain hash_chain_;         // HashChain data for constructing
                                     // backward references.
} DEDUP_vP8_LEncoder;

//------------------------------------------------------------------------------
// internal functions. Not public.

// Encodes the picture.
// Returns 0 if config or picture is NULL or picture doesn't have valid argb
// input.
int DEDUP_vP8_LEncodeImage(const DEDUP_WEBP_Config* const config,
                    const DEDUP_WEBP_Picture* const picture);

// Encodes the main image stream using the supplied bit writer.
// If 'use_cache' is false, disables the use of color cache.
DEDUP_WEBP_EncodingError DEDUP_vP8_LEncodeStream(const DEDUP_WEBP_Config* const config,
                                   const DEDUP_WEBP_Picture* const picture,
                                   DEDUP_vP8_LBitWriter* const bw, int use_cache);

//------------------------------------------------------------------------------
// Image transforms in predictor.c.

void DEDUP_vP8_LResidualImage(int width, int height, int bits, int low_effort,
                       uint32_t* const argb, uint32_t* const argb_scratch,
                       uint32_t* const image, int near_lossless, int exact,
                       int used_subtract_green);

void DEDUP_vP8_LColorSpaceTransform(int width, int height, int bits, int quality,
                             uint32_t* const argb, uint32_t* image);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_ENC_DEDUP_vP8_LI_H_ */
