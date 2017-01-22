// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Everything about DEDUP_WEBP_DecBuffer
//
// Author: Skal (pascal.massimino@gmail.com)

#include <stdlib.h>

#include "./vp8i.h"
#include "./webpi.h"
#include "../utils/utils.h"

//------------------------------------------------------------------------------
// DEDUP_WEBP_DecBuffer

// Number of bytes per pixel for the different color-spaces.
static const int kModeBpp[MODE_LAST] = {
  3, 4, 3, 4, 4, 2, 2,
  4, 4, 4, 2,    // pre-multiplied modes
  1, 1 };

// Check that webp_csp_mode is within the bounds of WEBP_CSP_MODE.
// Convert to an integer to handle both the unsigned/signed enum cases
// without the need for casting to remove type limit warnings.
static int IsValidColorspace(int webp_csp_mode) {
  return (webp_csp_mode >= MODE_RGB && webp_csp_mode < MODE_LAST);
}

// strictly speaking, the very last (or first, if flipped) row
// doesn't require padding.
#define MIN_BUFFER_SIZE(WIDTH, HEIGHT, STRIDE)       \
    (uint64_t)(STRIDE) * ((HEIGHT) - 1) + (WIDTH)

static DEDUP_vP8_StatusCode CheckDecBuffer(const DEDUP_WEBP_DecBuffer* const buffer) {
  int ok = 1;
  const WEBP_CSP_MODE mode = buffer->colorspace;
  const int width = buffer->width;
  const int height = buffer->height;
  if (!IsValidColorspace(mode)) {
    ok = 0;
  } else if (!DEDUP_WEBP_IsRGBMode(mode)) {   // YUV checks
    const DEDUP_WEBP_YUVABuffer* const buf = &buffer->u.YUVA;
    const int uv_width  = (width  + 1) / 2;
    const int uv_height = (height + 1) / 2;
    const int y_stride = abs(buf->y_stride);
    const int u_stride = abs(buf->u_stride);
    const int v_stride = abs(buf->v_stride);
    const int a_stride = abs(buf->a_stride);
    const uint64_t y_size = MIN_BUFFER_SIZE(width, height, y_stride);
    const uint64_t u_size = MIN_BUFFER_SIZE(uv_width, uv_height, u_stride);
    const uint64_t v_size = MIN_BUFFER_SIZE(uv_width, uv_height, v_stride);
    const uint64_t a_size = MIN_BUFFER_SIZE(width, height, a_stride);
    ok &= (y_size <= buf->y_size);
    ok &= (u_size <= buf->u_size);
    ok &= (v_size <= buf->v_size);
    ok &= (y_stride >= width);
    ok &= (u_stride >= uv_width);
    ok &= (v_stride >= uv_width);
    ok &= (buf->y != NULL);
    ok &= (buf->u != NULL);
    ok &= (buf->v != NULL);
    if (mode == MODE_YUVA) {
      ok &= (a_stride >= width);
      ok &= (a_size <= buf->a_size);
      ok &= (buf->a != NULL);
    }
  } else {    // RGB checks
    const DEDUP_WEBP_RGBABuffer* const buf = &buffer->u.RGBA;
    const int stride = abs(buf->stride);
    const uint64_t size = MIN_BUFFER_SIZE(width, height, stride);
    ok &= (size <= buf->size);
    ok &= (stride >= width * kModeBpp[mode]);
    ok &= (buf->rgba != NULL);
  }
  return ok ? DEDUP_vP8__STATUS_OK : DEDUP_vP8__STATUS_INVALID_PARAM;
}
#undef MIN_BUFFER_SIZE

static DEDUP_vP8_StatusCode AllocateBuffer(DEDUP_WEBP_DecBuffer* const buffer) {
  const int w = buffer->width;
  const int h = buffer->height;
  const WEBP_CSP_MODE mode = buffer->colorspace;

  if (w <= 0 || h <= 0 || !IsValidColorspace(mode)) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }

  if (buffer->is_external_memory <= 0 && buffer->private_memory == NULL) {
    uint8_t* output;
    int uv_stride = 0, a_stride = 0;
    uint64_t uv_size = 0, a_size = 0, total_size;
    // We need memory and it hasn't been allocated yet.
    // => initialize output buffer, now that dimensions are known.
    const int stride = w * kModeBpp[mode];
    const uint64_t size = (uint64_t)stride * h;

    if (!DEDUP_WEBP_IsRGBMode(mode)) {
      uv_stride = (w + 1) / 2;
      uv_size = (uint64_t)uv_stride * ((h + 1) / 2);
      if (mode == MODE_YUVA) {
        a_stride = w;
        a_size = (uint64_t)a_stride * h;
      }
    }
    total_size = size + 2 * uv_size + a_size;

    // Security/sanity checks
    output = (uint8_t*)DEDUP_WEBP_SafeMalloc(total_size, sizeof(*output));
    if (output == NULL) {
      return DEDUP_vP8__STATUS_OUT_OF_MEMORY;
    }
    buffer->private_memory = output;

    if (!DEDUP_WEBP_IsRGBMode(mode)) {   // YUVA initialization
      DEDUP_WEBP_YUVABuffer* const buf = &buffer->u.YUVA;
      buf->y = output;
      buf->y_stride = stride;
      buf->y_size = (size_t)size;
      buf->u = output + size;
      buf->u_stride = uv_stride;
      buf->u_size = (size_t)uv_size;
      buf->v = output + size + uv_size;
      buf->v_stride = uv_stride;
      buf->v_size = (size_t)uv_size;
      if (mode == MODE_YUVA) {
        buf->a = output + size + 2 * uv_size;
      }
      buf->a_size = (size_t)a_size;
      buf->a_stride = a_stride;
    } else {  // RGBA initialization
      DEDUP_WEBP_RGBABuffer* const buf = &buffer->u.RGBA;
      buf->rgba = output;
      buf->stride = stride;
      buf->size = (size_t)size;
    }
  }
  return CheckDecBuffer(buffer);
}

DEDUP_vP8_StatusCode DEDUP_WEBP_FlipBuffer(DEDUP_WEBP_DecBuffer* const buffer) {
  if (buffer == NULL) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }
  if (DEDUP_WEBP_IsRGBMode(buffer->colorspace)) {
    DEDUP_WEBP_RGBABuffer* const buf = &buffer->u.RGBA;
    buf->rgba += (buffer->height - 1) * buf->stride;
    buf->stride = -buf->stride;
  } else {
    DEDUP_WEBP_YUVABuffer* const buf = &buffer->u.YUVA;
    const int H = buffer->height;
    buf->y += (H - 1) * buf->y_stride;
    buf->y_stride = -buf->y_stride;
    buf->u += ((H - 1) >> 1) * buf->u_stride;
    buf->u_stride = -buf->u_stride;
    buf->v += ((H - 1) >> 1) * buf->v_stride;
    buf->v_stride = -buf->v_stride;
    if (buf->a != NULL) {
      buf->a += (H - 1) * buf->a_stride;
      buf->a_stride = -buf->a_stride;
    }
  }
  return DEDUP_vP8__STATUS_OK;
}

DEDUP_vP8_StatusCode DEDUP_WEBP_AllocateDecBuffer(int w, int h,
                                    const DEDUP_WEBP_DecoderOptions* const options,
                                    DEDUP_WEBP_DecBuffer* const out) {
  DEDUP_vP8_StatusCode status;
  if (out == NULL || w <= 0 || h <= 0) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }
  if (options != NULL) {    // First, apply options if there is any.
    if (options->use_cropping) {
      const int cw = options->crop_width;
      const int ch = options->crop_height;
      const int x = options->crop_left & ~1;
      const int y = options->crop_top & ~1;
      if (x < 0 || y < 0 || cw <= 0 || ch <= 0 || x + cw > w || y + ch > h) {
        return DEDUP_vP8__STATUS_INVALID_PARAM;   // out of frame boundary.
      }
      w = cw;
      h = ch;
    }
    if (options->use_scaling) {
      int scaled_width = options->scaled_width;
      int scaled_height = options->scaled_height;
      if (!DEDUP_WEBP_RescalerGetScaledDimensions(
              w, h, &scaled_width, &scaled_height)) {
        return DEDUP_vP8__STATUS_INVALID_PARAM;
      }
      w = scaled_width;
      h = scaled_height;
    }
  }
  out->width = w;
  out->height = h;

  // Then, allocate buffer for real.
  status = AllocateBuffer(out);
  if (status != DEDUP_vP8__STATUS_OK) return status;

  // Use the stride trick if vertical flip is needed.
  if (options != NULL && options->flip) {
    status = DEDUP_WEBP_FlipBuffer(out);
  }
  return status;
}

//------------------------------------------------------------------------------
// constructors / destructors

int DEDUP_WEBP_InitDecBufferInternal(DEDUP_WEBP_DecBuffer* buffer, int version) {
  if (WEBP_ABI_IS_INCOMPATIBLE(version, WEBP_DECODER_ABI_VERSION)) {
    return 0;  // version mismatch
  }
  if (buffer == NULL) return 0;
  memset(buffer, 0, sizeof(*buffer));
  return 1;
}

void DEDUP_WEBP_FreeDecBuffer(DEDUP_WEBP_DecBuffer* buffer) {
  if (buffer != NULL) {
    if (buffer->is_external_memory <= 0) {
      DEDUP_WEBP_SafeFree(buffer->private_memory);
    }
    buffer->private_memory = NULL;
  }
}

void DEDUP_WEBP_CopyDecBuffer(const DEDUP_WEBP_DecBuffer* const src,
                       DEDUP_WEBP_DecBuffer* const dst) {
  if (src != NULL && dst != NULL) {
    *dst = *src;
    if (src->private_memory != NULL) {
      dst->is_external_memory = 1;   // dst buffer doesn't own the memory.
      dst->private_memory = NULL;
    }
  }
}

// Copy and transfer ownership from src to dst (beware of parameter order!)
void DEDUP_WEBP_GrabDecBuffer(DEDUP_WEBP_DecBuffer* const src, DEDUP_WEBP_DecBuffer* const dst) {
  if (src != NULL && dst != NULL) {
    *dst = *src;
    if (src->private_memory != NULL) {
      src->is_external_memory = 1;   // src relinquishes ownership
      src->private_memory = NULL;
    }
  }
}

DEDUP_vP8_StatusCode DEDUP_WEBP_CopyDecBufferPixels(const DEDUP_WEBP_DecBuffer* const src_buf,
                                      DEDUP_WEBP_DecBuffer* const dst_buf) {
  assert(src_buf != NULL && dst_buf != NULL);
  assert(src_buf->colorspace == dst_buf->colorspace);

  dst_buf->width = src_buf->width;
  dst_buf->height = src_buf->height;
  if (CheckDecBuffer(dst_buf) != DEDUP_vP8__STATUS_OK) {
    return DEDUP_vP8__STATUS_INVALID_PARAM;
  }
  if (DEDUP_WEBP_IsRGBMode(src_buf->colorspace)) {
    const DEDUP_WEBP_RGBABuffer* const src = &src_buf->u.RGBA;
    const DEDUP_WEBP_RGBABuffer* const dst = &dst_buf->u.RGBA;
    DEDUP_WEBP_CopyPlane(src->rgba, src->stride, dst->rgba, dst->stride,
                  src_buf->width * kModeBpp[src_buf->colorspace],
                  src_buf->height);
  } else {
    const DEDUP_WEBP_YUVABuffer* const src = &src_buf->u.YUVA;
    const DEDUP_WEBP_YUVABuffer* const dst = &dst_buf->u.YUVA;
    DEDUP_WEBP_CopyPlane(src->y, src->y_stride, dst->y, dst->y_stride,
                  src_buf->width, src_buf->height);
    DEDUP_WEBP_CopyPlane(src->u, src->u_stride, dst->u, dst->u_stride,
                  (src_buf->width + 1) / 2, (src_buf->height + 1) / 2);
    DEDUP_WEBP_CopyPlane(src->v, src->v_stride, dst->v, dst->v_stride,
                  (src_buf->width + 1) / 2, (src_buf->height + 1) / 2);
    if (DEDUP_WEBP_IsAlphaMode(src_buf->colorspace)) {
      DEDUP_WEBP_CopyPlane(src->a, src->a_stride, dst->a, dst->a_stride,
                    src_buf->width, src_buf->height);
    }
  }
  return DEDUP_vP8__STATUS_OK;
}

int DEDUP_WEBP_AvoidSlowMemory(const DEDUP_WEBP_DecBuffer* const output,
                        const DEDUP_WEBP_BitstreamFeatures* const features) {
  assert(output != NULL);
  return (output->is_external_memory >= 2) &&
         DEDUP_WEBP_IsPremultipliedMode(output->colorspace) &&
         (features != NULL && features->has_alpha);
}

//------------------------------------------------------------------------------
