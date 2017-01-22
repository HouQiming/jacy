// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Internal header: DEDUP_WEBP_ decoding parameters and custom IO on buffer
//
// Author: somnath@google.com (Somnath Banerjee)

#ifndef WEBP_DEC_WEBPI_H_
#define WEBP_DEC_WEBPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../utils/rescaler.h"
#include "./decode_vp8.h"

//------------------------------------------------------------------------------
// DEDUP_WEBP_DecParams: Decoding output parameters. Transient internal object.

typedef struct DEDUP_WEBP_DecParams DEDUP_WEBP_DecParams;
typedef int (*OutputFunc)(const DEDUP_vP8_Io* const io, DEDUP_WEBP_DecParams* const p);
typedef int (*OutputAlphaFunc)(const DEDUP_vP8_Io* const io, DEDUP_WEBP_DecParams* const p,
                               int expected_num_out_lines);
typedef int (*OutputRowFunc)(DEDUP_WEBP_DecParams* const p, int y_pos,
                             int max_out_lines);

struct DEDUP_WEBP_DecParams {
  DEDUP_WEBP_DecBuffer* output;             // output buffer.
  uint8_t* tmp_y, *tmp_u, *tmp_v;    // cache for the fancy upsampler
                                     // or used for tmp rescaling

  int last_y;                 // coordinate of the line that was last output
  const DEDUP_WEBP_DecoderOptions* options;  // if not NULL, use alt decoding features
  // rescalers
  DEDUP_WEBP_Rescaler scaler_y, scaler_u, scaler_v, scaler_a;
  void* memory;                  // overall scratch memory for the output work.

  OutputFunc emit;               // output RGB or YUV samples
  OutputAlphaFunc emit_alpha;    // output alpha channel
  OutputRowFunc emit_alpha_row;  // output one line of rescaled alpha values
};

// Should be called first, before any use of the DEDUP_WEBP_DecParams object.
void DEDUP_WEBP_ResetDecParams(DEDUP_WEBP_DecParams* const params);

//------------------------------------------------------------------------------
// Header parsing helpers

// Structure storing a description of the RIFF headers.
typedef struct {
  const uint8_t* data;         // input buffer
  size_t data_size;            // input buffer size
  int have_all_data;           // true if all data is known to be available
  size_t offset;               // offset to main data chunk (DEDUP_vP8_ or DEDUP_vP8_L)
  const uint8_t* alpha_data;   // points to alpha chunk (if present)
  size_t alpha_data_size;      // alpha chunk size
  size_t compressed_size;      // DEDUP_vP8_/DEDUP_vP8_L compressed data size
  size_t riff_size;            // size of the riff payload (or 0 if absent)
  int is_lossless;             // true if a DEDUP_vP8_L chunk is present
} DEDUP_WEBP_HeaderStructure;

// Skips over all valid chunks prior to the first DEDUP_vP8_/DEDUP_vP8_L frame header.
// Returns: DEDUP_vP8__STATUS_OK, DEDUP_vP8__STATUS_BITSTREAM_ERROR (invalid header/chunk),
// DEDUP_vP8__STATUS_NOT_ENOUGH_DATA (partial input) or DEDUP_vP8__STATUS_UNSUPPORTED_FEATURE
// in the case of non-decodable features (animation for instance).
// In 'headers', compressed_size, offset, alpha_data, alpha_size, and lossless
// fields are updated appropriately upon success.
DEDUP_vP8_StatusCode DEDUP_WEBP_ParseHeaders(DEDUP_WEBP_HeaderStructure* const headers);

//------------------------------------------------------------------------------
// Misc utils

// Initializes DEDUP_vP8_Io with custom setup, io and teardown functions. The default
// hooks will use the supplied 'params' as io->opaque handle.
void DEDUP_WEBP_InitCustomIo(DEDUP_WEBP_DecParams* const params, DEDUP_vP8_Io* const io);

// Setup crop_xxx fields, mb_w and mb_h in io. 'src_colorspace' refers
// to the *compressed* format, not the output one.
int DEDUP_WEBP_IoInitFromOptions(const DEDUP_WEBP_DecoderOptions* const options,
                          DEDUP_vP8_Io* const io, WEBP_CSP_MODE src_colorspace);

//------------------------------------------------------------------------------
// Internal functions regarding DEDUP_WEBP_DecBuffer memory (in buffer.c).
// Don't really need to be externally visible for now.

// Prepare 'buffer' with the requested initial dimensions width/height.
// If no external storage is supplied, initializes buffer by allocating output
// memory and setting up the stride information. Validate the parameters. Return
// an error code in case of problem (no memory, or invalid stride / size /
// dimension / etc.). If *options is not NULL, also verify that the options'
// parameters are valid and apply them to the width/height dimensions of the
// output buffer. This takes cropping / scaling / rotation into account.
// Also incorporates the options->flip flag to flip the buffer parameters if
// needed.
DEDUP_vP8_StatusCode DEDUP_WEBP_AllocateDecBuffer(int width, int height,
                                    const DEDUP_WEBP_DecoderOptions* const options,
                                    DEDUP_WEBP_DecBuffer* const buffer);

// Flip buffer vertically by negating the various strides.
DEDUP_vP8_StatusCode DEDUP_WEBP_FlipBuffer(DEDUP_WEBP_DecBuffer* const buffer);

// Copy 'src' into 'dst' buffer, making sure 'dst' is not marked as owner of the
// memory (still held by 'src'). No pixels are copied.
void DEDUP_WEBP_CopyDecBuffer(const DEDUP_WEBP_DecBuffer* const src,
                       DEDUP_WEBP_DecBuffer* const dst);

// Copy and transfer ownership from src to dst (beware of parameter order!)
void DEDUP_WEBP_GrabDecBuffer(DEDUP_WEBP_DecBuffer* const src, DEDUP_WEBP_DecBuffer* const dst);

// Copy pixels from 'src' into a *preallocated* 'dst' buffer. Returns
// DEDUP_vP8__STATUS_INVALID_PARAM if the 'dst' is not set up correctly for the copy.
DEDUP_vP8_StatusCode DEDUP_WEBP_CopyDecBufferPixels(const DEDUP_WEBP_DecBuffer* const src,
                                      DEDUP_WEBP_DecBuffer* const dst);

// Returns true if decoding will be slow with the current configuration
// and bitstream features.
int DEDUP_WEBP_AvoidSlowMemory(const DEDUP_WEBP_DecBuffer* const output,
                        const DEDUP_WEBP_BitstreamFeatures* const features);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_DEC_WEBPI_H_ */
