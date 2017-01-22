// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Lossless decoder: internal header.
//
// Author: Skal (pascal.massimino@gmail.com)
//         Vikas Arora(vikaas.arora@gmail.com)

#ifndef WEBP_DEC_DEDUP_vP8_LI_H_
#define WEBP_DEC_DEDUP_vP8_LI_H_

#include <string.h>     // for memcpy()
#include "./webpi.h"
#include "../utils/bit_reader.h"
#include "../utils/color_cache.h"
#include "../utils/huffman.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  READ_DATA = 0,
  READ_HDR = 1,
  READ_DIM = 2
} DEDUP_vP8_LDecodeState;

typedef struct DEDUP_vP8_LTransform DEDUP_vP8_LTransform;
struct DEDUP_vP8_LTransform {
  DEDUP_vP8_LImageTransformType type_;   // transform type.
  int                    bits_;   // subsampling bits defining transform window.
  int                    xsize_;  // transform window X index.
  int                    ysize_;  // transform window Y index.
  uint32_t              *data_;   // transform data.
};

typedef struct {
  int             color_cache_size_;
  DEDUP_vP8_LColorCache  color_cache_;
  DEDUP_vP8_LColorCache  saved_color_cache_;  // for incremental

  int             huffman_mask_;
  int             huffman_subsample_bits_;
  int             huffman_xsize_;
  uint32_t       *huffman_image_;
  int             num_htree_groups_;
  HTreeGroup     *htree_groups_;
  HuffmanCode    *huffman_tables_;
} DEDUP_vP8_LMetadata;

typedef struct DEDUP_vP8_LDecoder DEDUP_vP8_LDecoder;
struct DEDUP_vP8_LDecoder {
  DEDUP_vP8_StatusCode    status_;
  DEDUP_vP8_LDecodeState  state_;
  DEDUP_vP8_Io           *io_;

  const DEDUP_WEBP_DecBuffer *output_;    // shortcut to io->opaque->output

  uint32_t        *pixels_;        // Internal data: either uint8_t* for alpha
                                   // or uint32_t* for BGRA.
  uint32_t        *argb_cache_;    // Scratch buffer for temporary BGRA storage.

  DEDUP_vP8_LBitReader    br_;
  int              incremental_;   // if true, incremental decoding is expected
  DEDUP_vP8_LBitReader    saved_br_;      // note: could be local variables too
  int              saved_last_pixel_;

  int              width_;
  int              height_;
  int              last_row_;      // last input row decoded so far.
  int              last_pixel_;    // last pixel decoded so far. However, it may
                                   // not be transformed, scaled and
                                   // color-converted yet.
  int              last_out_row_;  // last row output so far.

  DEDUP_vP8_LMetadata     hdr_;

  int              next_transform_;
  DEDUP_vP8_LTransform    transforms_[NUM_TRANSFORMS];
  // or'd bitset storing the transforms types.
  uint32_t         transforms_seen_;

  uint8_t         *rescaler_memory;  // Working memory for rescaling work.
  DEDUP_WEBP_Rescaler    *rescaler;         // Common rescaler for all channels.
};

//------------------------------------------------------------------------------
// internal functions. Not public.

struct ALPHDecoder;  // Defined in dec/alphai.h.

// in vp8l.c

// Decodes image header for alpha data stored using lossless compression.
// Returns false in case of error.
int DEDUP_vP8_LDecodeAlphaHeader(struct ALPHDecoder* const alph_dec,
                          const uint8_t* const data, size_t data_size);

// Decodes *at least* 'last_row' rows of alpha. If some of the initial rows are
// already decoded in previous call(s), it will resume decoding from where it
// was paused.
// Returns false in case of bitstream error.
int DEDUP_vP8_LDecodeAlphaImageStream(struct ALPHDecoder* const alph_dec,
                               int last_row);

// Allocates and initialize a new lossless decoder instance.
DEDUP_vP8_LDecoder* DEDUP_vP8_LNew(void);

// Decodes the image header. Returns false in case of error.
int DEDUP_vP8_LDecodeHeader(DEDUP_vP8_LDecoder* const dec, DEDUP_vP8_Io* const io);

// Decodes an image. It's required to decode the lossless header before calling
// this function. Returns false in case of error, with updated dec->status_.
int DEDUP_vP8_LDecodeImage(DEDUP_vP8_LDecoder* const dec);

// Resets the decoder in its initial state, reclaiming memory.
// Preserves the dec->status_ value.
void DEDUP_vP8_LClear(DEDUP_vP8_LDecoder* const dec);

// Clears and deallocate a lossless decoder instance.
void DEDUP_vP8_LDelete(DEDUP_vP8_LDecoder* const dec);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_DEC_DEDUP_vP8_LI_H_ */
