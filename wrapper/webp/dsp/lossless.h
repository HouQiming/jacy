// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Image transforms and color space conversion methods for lossless decoder.
//
// Authors: Vikas Arora (vikaas.arora@gmail.com)
//          Jyrki Alakuijala (jyrki@google.com)

#ifndef WEBP_DSP_LOSSLESS_H_
#define WEBP_DSP_LOSSLESS_H_

#include "../webp/types.h"
#include "../webp/decode.h"

#include "../enc/histogram.h"
#include "../utils/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WEBP_EXPERIMENTAL_FEATURES
#include "../enc/delta_palettization.h"
#endif  // WEBP_EXPERIMENTAL_FEATURES

//------------------------------------------------------------------------------
// Decoding

typedef uint32_t (*DEDUP_vP8_LPredictorFunc)(uint32_t left, const uint32_t* const top);
extern DEDUP_vP8_LPredictorFunc DEDUP_vP8_LPredictors[16];

typedef void (*DEDUP_vP8_LProcessBlueAndRedFunc)(uint32_t* argb_data, int num_pixels);
extern DEDUP_vP8_LProcessBlueAndRedFunc DEDUP_vP8_LAddGreenToBlueAndRed;

typedef struct {
  // Note: the members are uint8_t, so that any negative values are
  // automatically converted to "mod 256" values.
  uint8_t green_to_red_;
  uint8_t green_to_blue_;
  uint8_t red_to_blue_;
} DEDUP_vP8_LMultipliers;
typedef void (*DEDUP_vP8_LTransformColorFunc)(const DEDUP_vP8_LMultipliers* const m,
                                       uint32_t* argb_data, int num_pixels);
extern DEDUP_vP8_LTransformColorFunc DEDUP_vP8_LTransformColorInverse;

struct DEDUP_vP8_LTransform;  // Defined in dec/vp8li.h.

// Performs inverse transform of data given transform information, start and end
// rows. Transform will be applied to rows [row_start, row_end[.
// The *in and *out pointers refer to source and destination data respectively
// corresponding to the intermediate row (row_start).
void DEDUP_vP8_LInverseTransform(const struct DEDUP_vP8_LTransform* const transform,
                          int row_start, int row_end,
                          const uint32_t* const in, uint32_t* const out);

// Color space conversion.
typedef void (*DEDUP_vP8_LConvertFunc)(const uint32_t* src, int num_pixels,
                                uint8_t* dst);
extern DEDUP_vP8_LConvertFunc DEDUP_vP8_LConvertBGRAToRGB;
extern DEDUP_vP8_LConvertFunc DEDUP_vP8_LConvertBGRAToRGBA;
extern DEDUP_vP8_LConvertFunc DEDUP_vP8_LConvertBGRAToRGBA4444;
extern DEDUP_vP8_LConvertFunc DEDUP_vP8_LConvertBGRAToRGB565;
extern DEDUP_vP8_LConvertFunc DEDUP_vP8_LConvertBGRAToBGR;

// Converts from BGRA to other color spaces.
void DEDUP_vP8_LConvertFromBGRA(const uint32_t* const in_data, int num_pixels,
                         WEBP_CSP_MODE out_colorspace, uint8_t* const rgba);

typedef void (*DEDUP_vP8_LMapARGBFunc)(const uint32_t* src,
                                const uint32_t* const color_map,
                                uint32_t* dst, int y_start,
                                int y_end, int width);
typedef void (*DEDUP_vP8_LMapAlphaFunc)(const uint8_t* src,
                                 const uint32_t* const color_map,
                                 uint8_t* dst, int y_start,
                                 int y_end, int width);

extern DEDUP_vP8_LMapARGBFunc DEDUP_vP8_LMapColor32b;
extern DEDUP_vP8_LMapAlphaFunc DEDUP_vP8_LMapColor8b;

// Similar to the static method ColorIndexInverseTransform() that is part of
// lossless.c, but used only for alpha decoding. It takes uint8_t (rather than
// uint32_t) arguments for 'src' and 'dst'.
void DEDUP_vP8_LColorIndexInverseTransformAlpha(
    const struct DEDUP_vP8_LTransform* const transform, int y_start, int y_end,
    const uint8_t* src, uint8_t* dst);

// Expose some C-only fallback functions
void DEDUP_vP8_LTransformColorInverse_C(const DEDUP_vP8_LMultipliers* const m,
                                 uint32_t* data, int num_pixels);

void DEDUP_vP8_LConvertBGRAToRGB_C(const uint32_t* src, int num_pixels, uint8_t* dst);
void DEDUP_vP8_LConvertBGRAToRGBA_C(const uint32_t* src, int num_pixels, uint8_t* dst);
void DEDUP_vP8_LConvertBGRAToRGBA4444_C(const uint32_t* src,
                                 int num_pixels, uint8_t* dst);
void DEDUP_vP8_LConvertBGRAToRGB565_C(const uint32_t* src,
                               int num_pixels, uint8_t* dst);
void DEDUP_vP8_LConvertBGRAToBGR_C(const uint32_t* src, int num_pixels, uint8_t* dst);
void DEDUP_vP8_LAddGreenToBlueAndRed_C(uint32_t* data, int num_pixels);

// Must be called before calling any of the above methods.
void DEDUP_vP8_LDspInit(void);

//------------------------------------------------------------------------------
// Encoding

extern DEDUP_vP8_LProcessBlueAndRedFunc DEDUP_vP8_LSubtractGreenFromBlueAndRed;
extern DEDUP_vP8_LTransformColorFunc DEDUP_vP8_LTransformColor;
typedef void (*DEDUP_vP8_LCollectColorBlueTransformsFunc)(
    const uint32_t* argb, int stride,
    int tile_width, int tile_height,
    int green_to_blue, int red_to_blue, int histo[]);
extern DEDUP_vP8_LCollectColorBlueTransformsFunc DEDUP_vP8_LCollectColorBlueTransforms;

typedef void (*DEDUP_vP8_LCollectColorRedTransformsFunc)(
    const uint32_t* argb, int stride,
    int tile_width, int tile_height,
    int green_to_red, int histo[]);
extern DEDUP_vP8_LCollectColorRedTransformsFunc DEDUP_vP8_LCollectColorRedTransforms;

// Expose some C-only fallback functions
void DEDUP_vP8_LTransformColor_C(const DEDUP_vP8_LMultipliers* const m,
                          uint32_t* data, int num_pixels);
void DEDUP_vP8_LSubtractGreenFromBlueAndRed_C(uint32_t* argb_data, int num_pixels);
void DEDUP_vP8_LCollectColorRedTransforms_C(const uint32_t* argb, int stride,
                                     int tile_width, int tile_height,
                                     int green_to_red, int histo[]);
void DEDUP_vP8_LCollectColorBlueTransforms_C(const uint32_t* argb, int stride,
                                      int tile_width, int tile_height,
                                      int green_to_blue, int red_to_blue,
                                      int histo[]);

// -----------------------------------------------------------------------------
// Huffman-cost related functions.

typedef double (*DEDUP_vP8_LCostFunc)(const uint32_t* population, int length);
typedef double (*DEDUP_vP8_LCostCombinedFunc)(const uint32_t* X, const uint32_t* Y,
                                       int length);
typedef float (*DEDUP_vP8_LCombinedShannonEntropyFunc)(const int X[256],
                                                const int Y[256]);

extern DEDUP_vP8_LCostFunc DEDUP_vP8_LExtraCost;
extern DEDUP_vP8_LCostCombinedFunc DEDUP_vP8_LExtraCostCombined;
extern DEDUP_vP8_LCombinedShannonEntropyFunc DEDUP_vP8_LCombinedShannonEntropy;

typedef struct {        // small struct to hold counters
  int counts[2];        // index: 0=zero steak, 1=non-zero streak
  int streaks[2][2];    // [zero/non-zero][streak<3 / streak>=3]
} DEDUP_vP8_LStreaks;

typedef struct {            // small struct to hold bit entropy results
  double entropy;           // entropy
  uint32_t sum;             // sum of the population
  int nonzeros;             // number of non-zero elements in the population
  uint32_t max_val;         // maximum value in the population
  uint32_t nonzero_code;    // index of the last non-zero in the population
} DEDUP_vP8_LBitEntropy;

void DEDUP_vP8_LBitEntropyInit(DEDUP_vP8_LBitEntropy* const entropy);

// Get the combined symbol bit entropy and Huffman cost stats for the
// distributions 'X' and 'Y'. Those results can then be refined according to
// codec specific heuristics.
typedef void (*DEDUP_vP8_LGetCombinedEntropyUnrefinedFunc)(
    const uint32_t X[], const uint32_t Y[], int length,
    DEDUP_vP8_LBitEntropy* const bit_entropy, DEDUP_vP8_LStreaks* const stats);
extern DEDUP_vP8_LGetCombinedEntropyUnrefinedFunc DEDUP_vP8_LGetCombinedEntropyUnrefined;

// Get the entropy for the distribution 'X'.
typedef void (*DEDUP_vP8_LGetEntropyUnrefinedFunc)(const uint32_t X[], int length,
                                            DEDUP_vP8_LBitEntropy* const bit_entropy,
                                            DEDUP_vP8_LStreaks* const stats);
extern DEDUP_vP8_LGetEntropyUnrefinedFunc DEDUP_vP8_LGetEntropyUnrefined;

void DEDUP_vP8_LBitsEntropyUnrefined(const uint32_t* const array, int n,
                              DEDUP_vP8_LBitEntropy* const entropy);

typedef void (*DEDUP_vP8_LHistogramAddFunc)(const DEDUP_vP8_LHistogram* const a,
                                     const DEDUP_vP8_LHistogram* const b,
                                     DEDUP_vP8_LHistogram* const out);
extern DEDUP_vP8_LHistogramAddFunc DEDUP_vP8_LHistogramAdd;

// -----------------------------------------------------------------------------
// PrefixEncode()

typedef int (*DEDUP_vP8_LVectorMismatchFunc)(const uint32_t* const array1,
                                      const uint32_t* const array2, int length);
// Returns the first index where array1 and array2 are different.
extern DEDUP_vP8_LVectorMismatchFunc DEDUP_vP8_LVectorMismatch;

void DEDUP_vP8_LBundleColorMap(const uint8_t* const row, int width,
                        int xbits, uint32_t* const dst);

// Must be called before calling any of the above methods.
void DEDUP_vP8_LEncDspInit(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  // WEBP_DSP_LOSSLESS_H_
