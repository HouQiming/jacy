// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Author: Jyrki Alakuijala (jyrki@google.com)
//
// Models the histograms of literal and distance codes.

#ifndef WEBP_ENC_HISTOGRAM_H_
#define WEBP_ENC_HISTOGRAM_H_

#include <string.h>

#include "./backward_references.h"
#include "../webp/format_constants.h"
#include "../webp/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Not a trivial literal symbol.
#define DEDUP_vP8_L_NON_TRIVIAL_SYM (0xffffffff)

// A simple container for histograms of data.
typedef struct {
  // literal_ contains green literal, palette-code and
  // copy-length-prefix histogram
  uint32_t* literal_;         // Pointer to the allocated buffer for literal.
  uint32_t red_[NUM_LITERAL_CODES];
  uint32_t blue_[NUM_LITERAL_CODES];
  uint32_t alpha_[NUM_LITERAL_CODES];
  // Backward reference prefix-code histogram.
  uint32_t distance_[NUM_DISTANCE_CODES];
  int palette_code_bits_;
  uint32_t trivial_symbol_;  // True, if histograms for Red, Blue & Alpha
                             // literal symbols are single valued.
  double bit_cost_;          // cached value of bit cost.
  double literal_cost_;      // Cached values of dominant entropy costs:
  double red_cost_;          // literal, red & blue.
  double blue_cost_;
} DEDUP_vP8_LHistogram;

// Collection of histograms with fixed capacity, allocated as one
// big memory chunk. Can be destroyed by calling DEDUP_WEBP_SafeFree().
typedef struct {
  int size;         // number of slots currently in use
  int max_size;     // maximum capacity
  DEDUP_vP8_LHistogram** histograms;
} DEDUP_vP8_LHistogramSet;

// Create the histogram.
//
// The input data is the PixOrCopy data, which models the literals, stop
// codes and backward references (both distances and lengths).  Also: if
// palette_code_bits is >= 0, initialize the histogram with this value.
void DEDUP_vP8_LHistogramCreate(DEDUP_vP8_LHistogram* const p,
                         const DEDUP_vP8_LBackwardRefs* const refs,
                         int palette_code_bits);

// Return the size of the histogram for a given palette_code_bits.
int DEDUP_vP8_LGetHistogramSize(int palette_code_bits);

// Set the palette_code_bits and reset the stats.
void DEDUP_vP8_LHistogramInit(DEDUP_vP8_LHistogram* const p, int palette_code_bits);

// Collect all the references into a histogram (without reset)
void DEDUP_vP8_LHistogramStoreRefs(const DEDUP_vP8_LBackwardRefs* const refs,
                            DEDUP_vP8_LHistogram* const histo);

// Free the memory allocated for the histogram.
void DEDUP_vP8_LFreeHistogram(DEDUP_vP8_LHistogram* const histo);

// Free the memory allocated for the histogram set.
void DEDUP_vP8_LFreeHistogramSet(DEDUP_vP8_LHistogramSet* const histo);

// Allocate an array of pointer to histograms, allocated and initialized
// using 'cache_bits'. Return NULL in case of memory error.
DEDUP_vP8_LHistogramSet* DEDUP_vP8_LAllocateHistogramSet(int size, int cache_bits);

// Allocate and initialize histogram object with specified 'cache_bits'.
// Returns NULL in case of memory error.
// Special case of DEDUP_vP8_LAllocateHistogramSet, with size equals 1.
DEDUP_vP8_LHistogram* DEDUP_vP8_LAllocateHistogram(int cache_bits);

// Accumulate a token 'v' into a histogram.
void DEDUP_vP8_LHistogramAddSinglePixOrCopy(DEDUP_vP8_LHistogram* const histo,
                                     const PixOrCopy* const v);

static WEBP_INLINE int DEDUP_vP8_LHistogramNumCodes(int palette_code_bits) {
  return NUM_LITERAL_CODES + NUM_LENGTH_CODES +
      ((palette_code_bits > 0) ? (1 << palette_code_bits) : 0);
}

// Builds the histogram image.
int DEDUP_vP8_LGetHistoImageSymbols(int xsize, int ysize,
                             const DEDUP_vP8_LBackwardRefs* const refs,
                             int quality, int low_effort,
                             int histogram_bits, int cache_bits,
                             DEDUP_vP8_LHistogramSet* const image_in,
                             DEDUP_vP8_LHistogramSet* const tmp_histos,
                             uint16_t* const histogram_symbols);

// Returns the entropy for the symbols in the input array.
// Also sets trivial_symbol to the code value, if the array has only one code
// value. Otherwise, set it to DEDUP_vP8_L_NON_TRIVIAL_SYM.
double DEDUP_vP8_LBitsEntropy(const uint32_t* const array, int n,
                       uint32_t* const trivial_symbol);

// Estimate how many bits the combined entropy of literals and distance
// approximately maps to.
double DEDUP_vP8_LHistogramEstimateBits(const DEDUP_vP8_LHistogram* const p);

#ifdef __cplusplus
}
#endif

#endif  // WEBP_ENC_HISTOGRAM_H_
