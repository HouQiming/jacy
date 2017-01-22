// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// YUV to RGB upsampling functions.
//
// Author: somnath@google.com (Somnath Banerjee)

#include "./dsp.h"
#include "./yuv.h"

#include <assert.h>

//------------------------------------------------------------------------------
// Fancy upsampler

#ifdef FANCY_UPSAMPLING

// Fancy upsampling functions to convert YUV to RGB
DEDUP_WEBP_UpsampleLinePairFunc DEDUP_WEBP_Upsamplers[MODE_LAST];

// Given samples laid out in a square as:
//  [a b]
//  [c d]
// we interpolate u/v as:
//  ([9*a + 3*b + 3*c +   d    3*a + 9*b + 3*c +   d] + [8 8]) / 16
//  ([3*a +   b + 9*c + 3*d      a + 3*b + 3*c + 9*d]   [8 8]) / 16

// We process u and v together stashed into 32bit (16bit each).
#define LOAD_UV(u, v) ((u) | ((v) << 16))

#define UPSAMPLE_FUNC(FUNC_NAME, FUNC, XSTEP)                                  \
static void FUNC_NAME(const uint8_t* top_y, const uint8_t* bottom_y,           \
                      const uint8_t* top_u, const uint8_t* top_v,              \
                      const uint8_t* cur_u, const uint8_t* cur_v,              \
                      uint8_t* top_dst, uint8_t* bottom_dst, int len) {        \
  int x;                                                                       \
  const int last_pixel_pair = (len - 1) >> 1;                                  \
  uint32_t tl_uv = LOAD_UV(top_u[0], top_v[0]);   /* top-left sample */        \
  uint32_t l_uv  = LOAD_UV(cur_u[0], cur_v[0]);   /* left-sample */            \
  assert(top_y != NULL);                                                       \
  {                                                                            \
    const uint32_t uv0 = (3 * tl_uv + l_uv + 0x00020002u) >> 2;                \
    FUNC(top_y[0], uv0 & 0xff, (uv0 >> 16), top_dst);                          \
  }                                                                            \
  if (bottom_y != NULL) {                                                      \
    const uint32_t uv0 = (3 * l_uv + tl_uv + 0x00020002u) >> 2;                \
    FUNC(bottom_y[0], uv0 & 0xff, (uv0 >> 16), bottom_dst);                    \
  }                                                                            \
  for (x = 1; x <= last_pixel_pair; ++x) {                                     \
    const uint32_t t_uv = LOAD_UV(top_u[x], top_v[x]);  /* top sample */       \
    const uint32_t uv   = LOAD_UV(cur_u[x], cur_v[x]);  /* sample */           \
    /* precompute invariant values associated with first and second diagonals*/\
    const uint32_t avg = tl_uv + t_uv + l_uv + uv + 0x00080008u;               \
    const uint32_t diag_12 = (avg + 2 * (t_uv + l_uv)) >> 3;                   \
    const uint32_t diag_03 = (avg + 2 * (tl_uv + uv)) >> 3;                    \
    {                                                                          \
      const uint32_t uv0 = (diag_12 + tl_uv) >> 1;                             \
      const uint32_t uv1 = (diag_03 + t_uv) >> 1;                              \
      FUNC(top_y[2 * x - 1], uv0 & 0xff, (uv0 >> 16),                          \
           top_dst + (2 * x - 1) * XSTEP);                                     \
      FUNC(top_y[2 * x - 0], uv1 & 0xff, (uv1 >> 16),                          \
           top_dst + (2 * x - 0) * XSTEP);                                     \
    }                                                                          \
    if (bottom_y != NULL) {                                                    \
      const uint32_t uv0 = (diag_03 + l_uv) >> 1;                              \
      const uint32_t uv1 = (diag_12 + uv) >> 1;                                \
      FUNC(bottom_y[2 * x - 1], uv0 & 0xff, (uv0 >> 16),                       \
           bottom_dst + (2 * x - 1) * XSTEP);                                  \
      FUNC(bottom_y[2 * x + 0], uv1 & 0xff, (uv1 >> 16),                       \
           bottom_dst + (2 * x + 0) * XSTEP);                                  \
    }                                                                          \
    tl_uv = t_uv;                                                              \
    l_uv = uv;                                                                 \
  }                                                                            \
  if (!(len & 1)) {                                                            \
    {                                                                          \
      const uint32_t uv0 = (3 * tl_uv + l_uv + 0x00020002u) >> 2;              \
      FUNC(top_y[len - 1], uv0 & 0xff, (uv0 >> 16),                            \
           top_dst + (len - 1) * XSTEP);                                       \
    }                                                                          \
    if (bottom_y != NULL) {                                                    \
      const uint32_t uv0 = (3 * l_uv + tl_uv + 0x00020002u) >> 2;              \
      FUNC(bottom_y[len - 1], uv0 & 0xff, (uv0 >> 16),                         \
           bottom_dst + (len - 1) * XSTEP);                                    \
    }                                                                          \
  }                                                                            \
}

// All variants implemented.
UPSAMPLE_FUNC(UpsampleRgbLinePair,  DEDUP_vP8_YuvToRgb,  3)
UPSAMPLE_FUNC(UpsampleBgrLinePair,  DEDUP_vP8_YuvToBgr,  3)
UPSAMPLE_FUNC(UpsampleRgbaLinePair, DEDUP_vP8_YuvToRgba, 4)
UPSAMPLE_FUNC(UpsampleBgraLinePair, DEDUP_vP8_YuvToBgra, 4)
UPSAMPLE_FUNC(UpsampleArgbLinePair, DEDUP_vP8_YuvToArgb, 4)
UPSAMPLE_FUNC(UpsampleRgba4444LinePair, DEDUP_vP8_YuvToRgba4444, 2)
UPSAMPLE_FUNC(UpsampleRgb565LinePair,  DEDUP_vP8_YuvToRgb565,  2)

#undef LOAD_UV
#undef UPSAMPLE_FUNC

#endif  // FANCY_UPSAMPLING

//------------------------------------------------------------------------------

#if !defined(FANCY_UPSAMPLING)
#define DUAL_SAMPLE_FUNC(FUNC_NAME, FUNC)                                      \
static void FUNC_NAME(const uint8_t* top_y, const uint8_t* bot_y,              \
                      const uint8_t* top_u, const uint8_t* top_v,              \
                      const uint8_t* bot_u, const uint8_t* bot_v,              \
                      uint8_t* top_dst, uint8_t* bot_dst, int len) {           \
  const int half_len = len >> 1;                                               \
  int x;                                                                       \
  assert(top_dst != NULL);                                                     \
  {                                                                            \
    for (x = 0; x < half_len; ++x) {                                           \
      FUNC(top_y[2 * x + 0], top_u[x], top_v[x], top_dst + 8 * x + 0);         \
      FUNC(top_y[2 * x + 1], top_u[x], top_v[x], top_dst + 8 * x + 4);         \
    }                                                                          \
    if (len & 1) FUNC(top_y[2 * x + 0], top_u[x], top_v[x], top_dst + 8 * x);  \
  }                                                                            \
  if (bot_dst != NULL) {                                                       \
    for (x = 0; x < half_len; ++x) {                                           \
      FUNC(bot_y[2 * x + 0], bot_u[x], bot_v[x], bot_dst + 8 * x + 0);         \
      FUNC(bot_y[2 * x + 1], bot_u[x], bot_v[x], bot_dst + 8 * x + 4);         \
    }                                                                          \
    if (len & 1) FUNC(bot_y[2 * x + 0], bot_u[x], bot_v[x], bot_dst + 8 * x);  \
  }                                                                            \
}

DUAL_SAMPLE_FUNC(DualLineSamplerBGRA, DEDUP_vP8_YuvToBgra)
DUAL_SAMPLE_FUNC(DualLineSamplerARGB, DEDUP_vP8_YuvToArgb)
#undef DUAL_SAMPLE_FUNC

#endif  // !FANCY_UPSAMPLING

DEDUP_WEBP_UpsampleLinePairFunc DEDUP_WEBP_GetLinePairConverter(int alpha_is_last) {
  DEDUP_WEBP_InitUpsamplers();
  DEDUP_vP8_YUVInit();
#ifdef FANCY_UPSAMPLING
  return DEDUP_WEBP_Upsamplers[alpha_is_last ? MODE_BGRA : MODE_ARGB];
#else
  return (alpha_is_last ? DualLineSamplerBGRA : DualLineSamplerARGB);
#endif
}

//------------------------------------------------------------------------------
// YUV444 converter

#define YUV444_FUNC(FUNC_NAME, FUNC, XSTEP)                                    \
extern void FUNC_NAME(const uint8_t* y, const uint8_t* u, const uint8_t* v,    \
                      uint8_t* dst, int len);                                  \
void FUNC_NAME(const uint8_t* y, const uint8_t* u, const uint8_t* v,           \
               uint8_t* dst, int len) {                                        \
  int i;                                                                       \
  for (i = 0; i < len; ++i) FUNC(y[i], u[i], v[i], &dst[i * XSTEP]);           \
}

YUV444_FUNC(DEDUP_WEBP_Yuv444ToRgbC,      DEDUP_vP8_YuvToRgb,  3)
YUV444_FUNC(DEDUP_WEBP_Yuv444ToBgrC,      DEDUP_vP8_YuvToBgr,  3)
YUV444_FUNC(DEDUP_WEBP_Yuv444ToRgbaC,     DEDUP_vP8_YuvToRgba, 4)
YUV444_FUNC(DEDUP_WEBP_Yuv444ToBgraC,     DEDUP_vP8_YuvToBgra, 4)
YUV444_FUNC(DEDUP_WEBP_Yuv444ToArgbC,     DEDUP_vP8_YuvToArgb, 4)
YUV444_FUNC(DEDUP_WEBP_Yuv444ToRgba4444C, DEDUP_vP8_YuvToRgba4444, 2)
YUV444_FUNC(DEDUP_WEBP_Yuv444ToRgb565C,   DEDUP_vP8_YuvToRgb565, 2)

#undef YUV444_FUNC

DEDUP_WEBP_YUV444Converter DEDUP_WEBP_YUV444Converters[MODE_LAST];

extern void DEDUP_WEBP_InitYUV444ConvertersMIPSdspR2(void);
extern void DEDUP_WEBP_InitYUV444ConvertersSSE2(void);

static volatile DEDUP_vP8_CPUInfo upsampling_last_cpuinfo_used1 =
    (DEDUP_vP8_CPUInfo)&upsampling_last_cpuinfo_used1;

WEBP_TSAN_IGNORE_FUNCTION void DEDUP_WEBP_InitYUV444Converters(void) {
  if (upsampling_last_cpuinfo_used1 == DEDUP_vP8_GetCPUInfo) return;

  DEDUP_WEBP_YUV444Converters[MODE_RGB]       = DEDUP_WEBP_Yuv444ToRgbC;
  DEDUP_WEBP_YUV444Converters[MODE_RGBA]      = DEDUP_WEBP_Yuv444ToRgbaC;
  DEDUP_WEBP_YUV444Converters[MODE_BGR]       = DEDUP_WEBP_Yuv444ToBgrC;
  DEDUP_WEBP_YUV444Converters[MODE_BGRA]      = DEDUP_WEBP_Yuv444ToBgraC;
  DEDUP_WEBP_YUV444Converters[MODE_ARGB]      = DEDUP_WEBP_Yuv444ToArgbC;
  DEDUP_WEBP_YUV444Converters[MODE_RGBA_4444] = DEDUP_WEBP_Yuv444ToRgba4444C;
  DEDUP_WEBP_YUV444Converters[MODE_RGB_565]   = DEDUP_WEBP_Yuv444ToRgb565C;
  DEDUP_WEBP_YUV444Converters[MODE_rgbA]      = DEDUP_WEBP_Yuv444ToRgbaC;
  DEDUP_WEBP_YUV444Converters[MODE_bgrA]      = DEDUP_WEBP_Yuv444ToBgraC;
  DEDUP_WEBP_YUV444Converters[MODE_Argb]      = DEDUP_WEBP_Yuv444ToArgbC;
  DEDUP_WEBP_YUV444Converters[MODE_rgbA_4444] = DEDUP_WEBP_Yuv444ToRgba4444C;

  if (DEDUP_vP8_GetCPUInfo != NULL) {
#if defined(WEBP_USE_SSE2)
    if (DEDUP_vP8_GetCPUInfo(kSSE2)) {
      DEDUP_WEBP_InitYUV444ConvertersSSE2();
    }
#endif
#if defined(WEBP_USE_MIPS_DSP_R2)
    if (DEDUP_vP8_GetCPUInfo(kMIPSdspR2)) {
      DEDUP_WEBP_InitYUV444ConvertersMIPSdspR2();
    }
#endif
  }
  upsampling_last_cpuinfo_used1 = DEDUP_vP8_GetCPUInfo;
}

//------------------------------------------------------------------------------
// Main calls

extern void DEDUP_WEBP_InitUpsamplersSSE2(void);
extern void DEDUP_WEBP_InitUpsamplersNEON(void);
extern void DEDUP_WEBP_InitUpsamplersMIPSdspR2(void);
extern void DEDUP_WEBP_InitUpsamplersMSA(void);

static volatile DEDUP_vP8_CPUInfo upsampling_last_cpuinfo_used2 =
    (DEDUP_vP8_CPUInfo)&upsampling_last_cpuinfo_used2;

WEBP_TSAN_IGNORE_FUNCTION void DEDUP_WEBP_InitUpsamplers(void) {
  if (upsampling_last_cpuinfo_used2 == DEDUP_vP8_GetCPUInfo) return;

#ifdef FANCY_UPSAMPLING
  DEDUP_WEBP_Upsamplers[MODE_RGB]       = UpsampleRgbLinePair;
  DEDUP_WEBP_Upsamplers[MODE_RGBA]      = UpsampleRgbaLinePair;
  DEDUP_WEBP_Upsamplers[MODE_BGR]       = UpsampleBgrLinePair;
  DEDUP_WEBP_Upsamplers[MODE_BGRA]      = UpsampleBgraLinePair;
  DEDUP_WEBP_Upsamplers[MODE_ARGB]      = UpsampleArgbLinePair;
  DEDUP_WEBP_Upsamplers[MODE_RGBA_4444] = UpsampleRgba4444LinePair;
  DEDUP_WEBP_Upsamplers[MODE_RGB_565]   = UpsampleRgb565LinePair;
  DEDUP_WEBP_Upsamplers[MODE_rgbA]      = UpsampleRgbaLinePair;
  DEDUP_WEBP_Upsamplers[MODE_bgrA]      = UpsampleBgraLinePair;
  DEDUP_WEBP_Upsamplers[MODE_Argb]      = UpsampleArgbLinePair;
  DEDUP_WEBP_Upsamplers[MODE_rgbA_4444] = UpsampleRgba4444LinePair;

  // If defined, use CPUInfo() to overwrite some pointers with faster versions.
  if (DEDUP_vP8_GetCPUInfo != NULL) {
#if defined(WEBP_USE_SSE2)
    if (DEDUP_vP8_GetCPUInfo(kSSE2)) {
      DEDUP_WEBP_InitUpsamplersSSE2();
    }
#endif
#if defined(WEBP_USE_NEON)
    if (DEDUP_vP8_GetCPUInfo(kNEON)) {
      DEDUP_WEBP_InitUpsamplersNEON();
    }
#endif
#if defined(WEBP_USE_MIPS_DSP_R2)
    if (DEDUP_vP8_GetCPUInfo(kMIPSdspR2)) {
      DEDUP_WEBP_InitUpsamplersMIPSdspR2();
    }
#endif
#if defined(WEBP_USE_MSA)
    if (DEDUP_vP8_GetCPUInfo(kMSA)) {
      DEDUP_WEBP_InitUpsamplersMSA();
    }
#endif
  }
#endif  // FANCY_UPSAMPLING
  upsampling_last_cpuinfo_used2 = DEDUP_vP8_GetCPUInfo;
}

//------------------------------------------------------------------------------
