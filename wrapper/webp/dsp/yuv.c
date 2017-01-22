// Copyright 2010 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// YUV->RGB conversion functions
//
// Author: Skal (pascal.massimino@gmail.com)

#include "./yuv.h"

#if defined(WEBP_YUV_USE_TABLE)

static int done = 0;

static WEBP_INLINE uint8_t clip(int v, int max_value) {
  return v < 0 ? 0 : v > max_value ? max_value : v;
}

int16_t DEDUP_vP8_kVToR[256], DEDUP_vP8_kUToB[256];
int32_t DEDUP_vP8_kVToG[256], DEDUP_vP8_kUToG[256];
uint8_t DEDUP_vP8_kClip[YUV_RANGE_MAX - YUV_RANGE_MIN];
uint8_t DEDUP_vP8_kClip4Bits[YUV_RANGE_MAX - YUV_RANGE_MIN];

WEBP_TSAN_IGNORE_FUNCTION void DEDUP_vP8_YUVInit(void) {
  int i;
  if (done) {
    return;
  }
#ifndef USE_YUVj
  for (i = 0; i < 256; ++i) {
    DEDUP_vP8_kVToR[i] = (89858 * (i - 128) + YUV_HALF) >> YUV_FIX;
    DEDUP_vP8_kUToG[i] = -22014 * (i - 128) + YUV_HALF;
    DEDUP_vP8_kVToG[i] = -45773 * (i - 128);
    DEDUP_vP8_kUToB[i] = (113618 * (i - 128) + YUV_HALF) >> YUV_FIX;
  }
  for (i = YUV_RANGE_MIN; i < YUV_RANGE_MAX; ++i) {
    const int k = ((i - 16) * 76283 + YUV_HALF) >> YUV_FIX;
    DEDUP_vP8_kClip[i - YUV_RANGE_MIN] = clip(k, 255);
    DEDUP_vP8_kClip4Bits[i - YUV_RANGE_MIN] = clip((k + 8) >> 4, 15);
  }
#else
  for (i = 0; i < 256; ++i) {
    DEDUP_vP8_kVToR[i] = (91881 * (i - 128) + YUV_HALF) >> YUV_FIX;
    DEDUP_vP8_kUToG[i] = -22554 * (i - 128) + YUV_HALF;
    DEDUP_vP8_kVToG[i] = -46802 * (i - 128);
    DEDUP_vP8_kUToB[i] = (116130 * (i - 128) + YUV_HALF) >> YUV_FIX;
  }
  for (i = YUV_RANGE_MIN; i < YUV_RANGE_MAX; ++i) {
    const int k = i;
    DEDUP_vP8_kClip[i - YUV_RANGE_MIN] = clip(k, 255);
    DEDUP_vP8_kClip4Bits[i - YUV_RANGE_MIN] = clip((k + 8) >> 4, 15);
  }
#endif

  done = 1;
}

#else

WEBP_TSAN_IGNORE_FUNCTION void DEDUP_vP8_YUVInit(void) {}

#endif  // WEBP_YUV_USE_TABLE

//-----------------------------------------------------------------------------
// Plain-C version

#define ROW_FUNC(FUNC_NAME, FUNC, XSTEP)                                       \
static void FUNC_NAME(const uint8_t* y,                                        \
                      const uint8_t* u, const uint8_t* v,                      \
                      uint8_t* dst, int len) {                                 \
  const uint8_t* const end = dst + (len & ~1) * XSTEP;                         \
  while (dst != end) {                                                         \
    FUNC(y[0], u[0], v[0], dst);                                               \
    FUNC(y[1], u[0], v[0], dst + XSTEP);                                       \
    y += 2;                                                                    \
    ++u;                                                                       \
    ++v;                                                                       \
    dst += 2 * XSTEP;                                                          \
  }                                                                            \
  if (len & 1) {                                                               \
    FUNC(y[0], u[0], v[0], dst);                                               \
  }                                                                            \
}                                                                              \

// All variants implemented.
ROW_FUNC(YuvToRgbRow,      DEDUP_vP8_YuvToRgb,  3)
ROW_FUNC(YuvToBgrRow,      DEDUP_vP8_YuvToBgr,  3)
ROW_FUNC(YuvToRgbaRow,     DEDUP_vP8_YuvToRgba, 4)
ROW_FUNC(YuvToBgraRow,     DEDUP_vP8_YuvToBgra, 4)
ROW_FUNC(YuvToArgbRow,     DEDUP_vP8_YuvToArgb, 4)
ROW_FUNC(YuvToRgba4444Row, DEDUP_vP8_YuvToRgba4444, 2)
ROW_FUNC(YuvToRgb565Row,   DEDUP_vP8_YuvToRgb565, 2)

#undef ROW_FUNC

// Main call for processing a plane with a DEDUP_WEBP_SamplerRowFunc function:
void DEDUP_WEBP_SamplerProcessPlane(const uint8_t* y, int y_stride,
                             const uint8_t* u, const uint8_t* v, int uv_stride,
                             uint8_t* dst, int dst_stride,
                             int width, int height, DEDUP_WEBP_SamplerRowFunc func) {
  int j;
  for (j = 0; j < height; ++j) {
    func(y, u, v, dst, width);
    y += y_stride;
    if (j & 1) {
      u += uv_stride;
      v += uv_stride;
    }
    dst += dst_stride;
  }
}

//-----------------------------------------------------------------------------
// Main call

DEDUP_WEBP_SamplerRowFunc DEDUP_WEBP_Samplers[MODE_LAST];

extern void DEDUP_WEBP_InitSamplersSSE2(void);
extern void DEDUP_WEBP_InitSamplersMIPS32(void);
extern void DEDUP_WEBP_InitSamplersMIPSdspR2(void);

static volatile DEDUP_vP8_CPUInfo yuv_last_cpuinfo_used =
    (DEDUP_vP8_CPUInfo)&yuv_last_cpuinfo_used;

WEBP_TSAN_IGNORE_FUNCTION void DEDUP_WEBP_InitSamplers(void) {
  if (yuv_last_cpuinfo_used == DEDUP_vP8_GetCPUInfo) return;

  DEDUP_WEBP_Samplers[MODE_RGB]       = YuvToRgbRow;
  DEDUP_WEBP_Samplers[MODE_RGBA]      = YuvToRgbaRow;
  DEDUP_WEBP_Samplers[MODE_BGR]       = YuvToBgrRow;
  DEDUP_WEBP_Samplers[MODE_BGRA]      = YuvToBgraRow;
  DEDUP_WEBP_Samplers[MODE_ARGB]      = YuvToArgbRow;
  DEDUP_WEBP_Samplers[MODE_RGBA_4444] = YuvToRgba4444Row;
  DEDUP_WEBP_Samplers[MODE_RGB_565]   = YuvToRgb565Row;
  DEDUP_WEBP_Samplers[MODE_rgbA]      = YuvToRgbaRow;
  DEDUP_WEBP_Samplers[MODE_bgrA]      = YuvToBgraRow;
  DEDUP_WEBP_Samplers[MODE_Argb]      = YuvToArgbRow;
  DEDUP_WEBP_Samplers[MODE_rgbA_4444] = YuvToRgba4444Row;

  // If defined, use CPUInfo() to overwrite some pointers with faster versions.
  if (DEDUP_vP8_GetCPUInfo != NULL) {
#if defined(WEBP_USE_SSE2)
    if (DEDUP_vP8_GetCPUInfo(kSSE2)) {
      DEDUP_WEBP_InitSamplersSSE2();
    }
#endif  // WEBP_USE_SSE2
#if defined(WEBP_USE_MIPS32)
    if (DEDUP_vP8_GetCPUInfo(kMIPS32)) {
      DEDUP_WEBP_InitSamplersMIPS32();
    }
#endif  // WEBP_USE_MIPS32
#if defined(WEBP_USE_MIPS_DSP_R2)
    if (DEDUP_vP8_GetCPUInfo(kMIPSdspR2)) {
      DEDUP_WEBP_InitSamplersMIPSdspR2();
    }
#endif  // WEBP_USE_MIPS_DSP_R2
  }
  yuv_last_cpuinfo_used = DEDUP_vP8_GetCPUInfo;
}

//-----------------------------------------------------------------------------
// ARGB -> YUV converters

static void ConvertARGBToY(const uint32_t* argb, uint8_t* y, int width) {
  int i;
  for (i = 0; i < width; ++i) {
    const uint32_t p = argb[i];
    y[i] = DEDUP_vP8_RGBToY((p >> 16) & 0xff, (p >> 8) & 0xff, (p >>  0) & 0xff,
                     YUV_HALF);
  }
}

void DEDUP_WEBP_ConvertARGBToUV_C(const uint32_t* argb, uint8_t* u, uint8_t* v,
                           int src_width, int do_store) {
  // No rounding. Last pixel is dealt with separately.
  const int uv_width = src_width >> 1;
  int i;
  for (i = 0; i < uv_width; ++i) {
    const uint32_t v0 = argb[2 * i + 0];
    const uint32_t v1 = argb[2 * i + 1];
    // DEDUP_vP8_RGBToU/V expects four accumulated pixels. Hence we need to
    // scale r/g/b value by a factor 2. We just shift v0/v1 one bit less.
    const int r = ((v0 >> 15) & 0x1fe) + ((v1 >> 15) & 0x1fe);
    const int g = ((v0 >>  7) & 0x1fe) + ((v1 >>  7) & 0x1fe);
    const int b = ((v0 <<  1) & 0x1fe) + ((v1 <<  1) & 0x1fe);
    const int tmp_u = DEDUP_vP8_RGBToU(r, g, b, YUV_HALF << 2);
    const int tmp_v = DEDUP_vP8_RGBToV(r, g, b, YUV_HALF << 2);
    if (do_store) {
      u[i] = tmp_u;
      v[i] = tmp_v;
    } else {
      // Approximated average-of-four. But it's an acceptable diff.
      u[i] = (u[i] + tmp_u + 1) >> 1;
      v[i] = (v[i] + tmp_v + 1) >> 1;
    }
  }
  if (src_width & 1) {       // last pixel
    const uint32_t v0 = argb[2 * i + 0];
    const int r = (v0 >> 14) & 0x3fc;
    const int g = (v0 >>  6) & 0x3fc;
    const int b = (v0 <<  2) & 0x3fc;
    const int tmp_u = DEDUP_vP8_RGBToU(r, g, b, YUV_HALF << 2);
    const int tmp_v = DEDUP_vP8_RGBToV(r, g, b, YUV_HALF << 2);
    if (do_store) {
      u[i] = tmp_u;
      v[i] = tmp_v;
    } else {
      u[i] = (u[i] + tmp_u + 1) >> 1;
      v[i] = (v[i] + tmp_v + 1) >> 1;
    }
  }
}

//-----------------------------------------------------------------------------

static void ConvertRGB24ToY(const uint8_t* rgb, uint8_t* y, int width) {
  int i;
  for (i = 0; i < width; ++i, rgb += 3) {
    y[i] = DEDUP_vP8_RGBToY(rgb[0], rgb[1], rgb[2], YUV_HALF);
  }
}

static void ConvertBGR24ToY(const uint8_t* bgr, uint8_t* y, int width) {
  int i;
  for (i = 0; i < width; ++i, bgr += 3) {
    y[i] = DEDUP_vP8_RGBToY(bgr[2], bgr[1], bgr[0], YUV_HALF);
  }
}

void DEDUP_WEBP_ConvertRGBA32ToUV_C(const uint16_t* rgb,
                             uint8_t* u, uint8_t* v, int width) {
  int i;
  for (i = 0; i < width; i += 1, rgb += 4) {
    const int r = rgb[0], g = rgb[1], b = rgb[2];
    u[i] = DEDUP_vP8_RGBToU(r, g, b, YUV_HALF << 2);
    v[i] = DEDUP_vP8_RGBToV(r, g, b, YUV_HALF << 2);
  }
}

//-----------------------------------------------------------------------------

void (*DEDUP_WEBP_ConvertRGB24ToY)(const uint8_t* rgb, uint8_t* y, int width);
void (*DEDUP_WEBP_ConvertBGR24ToY)(const uint8_t* bgr, uint8_t* y, int width);
void (*DEDUP_WEBP_ConvertRGBA32ToUV)(const uint16_t* rgb,
                              uint8_t* u, uint8_t* v, int width);

void (*DEDUP_WEBP_ConvertARGBToY)(const uint32_t* argb, uint8_t* y, int width);
void (*DEDUP_WEBP_ConvertARGBToUV)(const uint32_t* argb, uint8_t* u, uint8_t* v,
                            int src_width, int do_store);

static volatile DEDUP_vP8_CPUInfo rgba_to_yuv_last_cpuinfo_used =
    (DEDUP_vP8_CPUInfo)&rgba_to_yuv_last_cpuinfo_used;

extern void DEDUP_WEBP_InitConvertARGBToYUVSSE2(void);

WEBP_TSAN_IGNORE_FUNCTION void DEDUP_WEBP_InitConvertARGBToYUV(void) {
  if (rgba_to_yuv_last_cpuinfo_used == DEDUP_vP8_GetCPUInfo) return;

  DEDUP_WEBP_ConvertARGBToY = ConvertARGBToY;
  DEDUP_WEBP_ConvertARGBToUV = DEDUP_WEBP_ConvertARGBToUV_C;

  DEDUP_WEBP_ConvertRGB24ToY = ConvertRGB24ToY;
  DEDUP_WEBP_ConvertBGR24ToY = ConvertBGR24ToY;

  DEDUP_WEBP_ConvertRGBA32ToUV = DEDUP_WEBP_ConvertRGBA32ToUV_C;

  if (DEDUP_vP8_GetCPUInfo != NULL) {
#if defined(WEBP_USE_SSE2)
    if (DEDUP_vP8_GetCPUInfo(kSSE2)) {
      DEDUP_WEBP_InitConvertARGBToYUVSSE2();
    }
#endif  // WEBP_USE_SSE2
  }
  rgba_to_yuv_last_cpuinfo_used = DEDUP_vP8_GetCPUInfo;
}
