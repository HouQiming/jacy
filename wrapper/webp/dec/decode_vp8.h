// Copyright 2010 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
//  Low-level API for DEDUP_vP8_ decoder
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef WEBP_WEBP_DECODE_DEDUP_vP8__H_
#define WEBP_WEBP_DECODE_DEDUP_vP8__H_

#include "../webp/decode.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Lower-level API
//
// These functions provide fine-grained control of the decoding process.
// The call flow should resemble:
//
//   DEDUP_vP8_Io io;
//   DEDUP_vP8_InitIo(&io);
//   io.data = data;
//   io.data_size = size;
//   /* customize io's functions (setup()/put()/teardown()) if needed. */
//
//   DEDUP_vP8_Decoder* dec = DEDUP_vP8_New();
//   bool ok = DEDUP_vP8_Decode(dec);
//   if (!ok) printf("Error: %s\n", DEDUP_vP8_StatusMessage(dec));
//   DEDUP_vP8_Delete(dec);
//   return ok;

// Input / Output
typedef struct DEDUP_vP8_Io DEDUP_vP8_Io;
typedef int (*DEDUP_vP8_IoPutHook)(const DEDUP_vP8_Io* io);
typedef int (*DEDUP_vP8_IoSetupHook)(DEDUP_vP8_Io* io);
typedef void (*DEDUP_vP8_IoTeardownHook)(const DEDUP_vP8_Io* io);

struct DEDUP_vP8_Io {
  // set by DEDUP_vP8_GetHeaders()
  int width, height;         // picture dimensions, in pixels (invariable).
                             // These are the original, uncropped dimensions.
                             // The actual area passed to put() is stored
                             // in mb_w / mb_h fields.

  // set before calling put()
  int mb_y;                  // position of the current rows (in pixels)
  int mb_w;                  // number of columns in the sample
  int mb_h;                  // number of rows in the sample
  const uint8_t* y, *u, *v;  // rows to copy (in yuv420 format)
  int y_stride;              // row stride for luma
  int uv_stride;             // row stride for chroma

  void* opaque;              // user data

  // called when fresh samples are available. Currently, samples are in
  // YUV420 format, and can be up to width x 24 in size (depending on the
  // in-loop filtering level, e.g.). Should return false in case of error
  // or abort request. The actual size of the area to update is mb_w x mb_h
  // in size, taking cropping into account.
  DEDUP_vP8_IoPutHook put;

  // called just before starting to decode the blocks.
  // Must return false in case of setup error, true otherwise. If false is
  // returned, teardown() will NOT be called. But if the setup succeeded
  // and true is returned, then teardown() will always be called afterward.
  DEDUP_vP8_IoSetupHook setup;

  // Called just after block decoding is finished (or when an error occurred
  // during put()). Is NOT called if setup() failed.
  DEDUP_vP8_IoTeardownHook teardown;

  // this is a recommendation for the user-side yuv->rgb converter. This flag
  // is set when calling setup() hook and can be overwritten by it. It then
  // can be taken into consideration during the put() method.
  int fancy_upsampling;

  // Input buffer.
  size_t data_size;
  const uint8_t* data;

  // If true, in-loop filtering will not be performed even if present in the
  // bitstream. Switching off filtering may speed up decoding at the expense
  // of more visible blocking. Note that output will also be non-compliant
  // with the DEDUP_vP8_ specifications.
  int bypass_filtering;

  // Cropping parameters.
  int use_cropping;
  int crop_left, crop_right, crop_top, crop_bottom;

  // Scaling parameters.
  int use_scaling;
  int scaled_width, scaled_height;

  // If non NULL, pointer to the alpha data (if present) corresponding to the
  // start of the current row (That is: it is pre-offset by mb_y and takes
  // cropping into account).
  const uint8_t* a;
};

// Internal, version-checked, entry point
int DEDUP_vP8_InitIoInternal(DEDUP_vP8_Io* const, int);

// Set the custom IO function pointers and user-data. The setter for IO hooks
// should be called before initiating incremental decoding. Returns true if
// DEDUP_WEBP_IDecoder object is successfully modified, false otherwise.
int DEDUP_WEBP_ISetIOHooks(DEDUP_WEBP_IDecoder* const idec,
                    DEDUP_vP8_IoPutHook put,
                    DEDUP_vP8_IoSetupHook setup,
                    DEDUP_vP8_IoTeardownHook teardown,
                    void* user_data);

// Main decoding object. This is an opaque structure.
typedef struct DEDUP_vP8_Decoder DEDUP_vP8_Decoder;

// Create a new decoder object.
DEDUP_vP8_Decoder* DEDUP_vP8_New(void);

// Must be called to make sure 'io' is initialized properly.
// Returns false in case of version mismatch. Upon such failure, no other
// decoding function should be called (DEDUP_vP8_Decode, DEDUP_vP8_GetHeaders, ...)
static WEBP_INLINE int DEDUP_vP8_InitIo(DEDUP_vP8_Io* const io) {
  return DEDUP_vP8_InitIoInternal(io, WEBP_DECODER_ABI_VERSION);
}

// Decode the DEDUP_vP8_ frame header. Returns true if ok.
// Note: 'io->data' must be pointing to the start of the DEDUP_vP8_ frame header.
int DEDUP_vP8_GetHeaders(DEDUP_vP8_Decoder* const dec, DEDUP_vP8_Io* const io);

// Decode a picture. Will call DEDUP_vP8_GetHeaders() if it wasn't done already.
// Returns false in case of error.
int DEDUP_vP8_Decode(DEDUP_vP8_Decoder* const dec, DEDUP_vP8_Io* const io);

// Return current status of the decoder:
DEDUP_vP8_StatusCode DEDUP_vP8_Status(DEDUP_vP8_Decoder* const dec);

// return readable string corresponding to the last status.
const char* DEDUP_vP8_StatusMessage(DEDUP_vP8_Decoder* const dec);

// Resets the decoder in its initial state, reclaiming memory.
// Not a mandatory call between calls to DEDUP_vP8_Decode().
void DEDUP_vP8_Clear(DEDUP_vP8_Decoder* const dec);

// Destroy the decoder object.
void DEDUP_vP8_Delete(DEDUP_vP8_Decoder* const dec);

//------------------------------------------------------------------------------
// Miscellaneous DEDUP_vP8_/DEDUP_vP8_L bitstream probing functions.

// Returns true if the next 3 bytes in data contain the DEDUP_vP8_ signature.
WEBP_EXTERN(int) DEDUP_vP8_CheckSignature(const uint8_t* const data, size_t data_size);

// Validates the DEDUP_vP8_ data-header and retrieves basic header information viz
// width and height. Returns 0 in case of formatting error. *width/*height
// can be passed NULL.
WEBP_EXTERN(int) DEDUP_vP8_GetInfo(
    const uint8_t* data,
    size_t data_size,    // data available so far
    size_t chunk_size,   // total data size expected in the chunk
    int* const width, int* const height);

// Returns true if the next byte(s) in data is a DEDUP_vP8_L signature.
WEBP_EXTERN(int) DEDUP_vP8_LCheckSignature(const uint8_t* const data, size_t size);

// Validates the DEDUP_vP8_L data-header and retrieves basic header information viz
// width, height and alpha. Returns 0 in case of formatting error.
// width/height/has_alpha can be passed NULL.
WEBP_EXTERN(int) DEDUP_vP8_LGetInfo(
    const uint8_t* data, size_t data_size,  // data available so far
    int* const width, int* const height, int* const has_alpha);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_WEBP_DECODE_DEDUP_vP8__H_ */
