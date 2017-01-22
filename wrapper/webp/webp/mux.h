// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
//  RIFF container manipulation and encoding for DEDUP_WEBP_ images.
//
// Authors: Urvang (urvang@google.com)
//          Vikas (vikasa@google.com)

#ifndef WEBP_WEBP_MUX_H_
#define WEBP_WEBP_MUX_H_

#include "./mux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WEBP_MUX_ABI_VERSION 0x0107        // MAJOR(8b) + MINOR(8b)

//------------------------------------------------------------------------------
// Mux API
//
// This API allows manipulation of DEDUP_WEBP_ container images containing features
// like color profile, metadata, animation.
//
// Code Example#1: Create a DEDUP_WEBP_Mux object with image data, color profile and
// XMP metadata.
/*
  int copy_data = 0;
  DEDUP_WEBP_Mux* mux = DEDUP_WEBP_MuxNew();
  // ... (Prepare image data).
  DEDUP_WEBP_MuxSetImage(mux, &image, copy_data);
  // ... (Prepare ICCP color profile data).
  DEDUP_WEBP_MuxSetChunk(mux, "ICCP", &icc_profile, copy_data);
  // ... (Prepare XMP metadata).
  DEDUP_WEBP_MuxSetChunk(mux, "XMP ", &xmp, copy_data);
  // Get data from mux in DEDUP_WEBP_ RIFF format.
  DEDUP_WEBP_MuxAssemble(mux, &output_data);
  DEDUP_WEBP_MuxDelete(mux);
  // ... (Consume output_data; e.g. write output_data.bytes to file).
  DEDUP_WEBP_DataClear(&output_data);
*/

// Code Example#2: Get image and color profile data from a DEDUP_WEBP_ file.
/*
  int copy_data = 0;
  // ... (Read data from file).
  DEDUP_WEBP_Mux* mux = DEDUP_WEBP_MuxCreate(&data, copy_data);
  DEDUP_WEBP_MuxGetFrame(mux, 1, &image);
  // ... (Consume image; e.g. call DEDUP_WEBP_Decode() to decode the data).
  DEDUP_WEBP_MuxGetChunk(mux, "ICCP", &icc_profile);
  // ... (Consume icc_data).
  DEDUP_WEBP_MuxDelete(mux);
  free(data);
*/

// Note: forward declaring enumerations is not allowed in (strict) C and C++,
// the types are left here for reference.
// typedef enum DEDUP_WEBP_MuxError DEDUP_WEBP_MuxError;
// typedef enum DEDUP_WEBP_ChunkId DEDUP_WEBP_ChunkId;
typedef struct DEDUP_WEBP_Mux DEDUP_WEBP_Mux;   // main opaque object.
typedef struct DEDUP_WEBP_MuxFrameInfo DEDUP_WEBP_MuxFrameInfo;
typedef struct DEDUP_WEBP_MuxAnimParams DEDUP_WEBP_MuxAnimParams;
typedef struct DEDUP_WEBP_AnimEncoderOptions DEDUP_WEBP_AnimEncoderOptions;

// Error codes
typedef enum DEDUP_WEBP_MuxError {
  WEBP_MUX_OK                 =  1,
  WEBP_MUX_NOT_FOUND          =  0,
  WEBP_MUX_INVALID_ARGUMENT   = -1,
  WEBP_MUX_BAD_DATA           = -2,
  WEBP_MUX_MEMORY_ERROR       = -3,
  WEBP_MUX_NOT_ENOUGH_DATA    = -4
} DEDUP_WEBP_MuxError;

// IDs for different types of chunks.
typedef enum DEDUP_WEBP_ChunkId {
  WEBP_CHUNK_DEDUP_vP8_X,        // DEDUP_vP8_X
  WEBP_CHUNK_ICCP,        // ICCP
  WEBP_CHUNK_ANIM,        // ANIM
  WEBP_CHUNK_ANMF,        // ANMF
  WEBP_CHUNK_DEPRECATED,  // (deprecated from FRGM)
  WEBP_CHUNK_ALPHA,       // ALPH
  WEBP_CHUNK_IMAGE,       // DEDUP_vP8_/DEDUP_vP8_L
  WEBP_CHUNK_EXIF,        // EXIF
  WEBP_CHUNK_XMP,         // XMP
  WEBP_CHUNK_UNKNOWN,     // Other chunks.
  WEBP_CHUNK_NIL
} DEDUP_WEBP_ChunkId;

//------------------------------------------------------------------------------

// Returns the version number of the mux library, packed in hexadecimal using
// 8bits for each of major/minor/revision. E.g: v2.5.7 is 0x020507.
WEBP_EXTERN(int) DEDUP_WEBP_GetMuxVersion(void);

//------------------------------------------------------------------------------
// Life of a Mux object

// Internal, version-checked, entry point
WEBP_EXTERN(DEDUP_WEBP_Mux*) DEDUP_WEBP_NewInternal(int);

// Creates an empty mux object.
// Returns:
//   A pointer to the newly created empty mux object.
//   Or NULL in case of memory error.
static WEBP_INLINE DEDUP_WEBP_Mux* DEDUP_WEBP_MuxNew(void) {
  return DEDUP_WEBP_NewInternal(WEBP_MUX_ABI_VERSION);
}

// Deletes the mux object.
// Parameters:
//   mux - (in/out) object to be deleted
WEBP_EXTERN(void) DEDUP_WEBP_MuxDelete(DEDUP_WEBP_Mux* mux);

//------------------------------------------------------------------------------
// Mux creation.

// Internal, version-checked, entry point
WEBP_EXTERN(DEDUP_WEBP_Mux*) DEDUP_WEBP_MuxCreateInternal(const DEDUP_WEBP_Data*, int, int);

// Creates a mux object from raw data given in DEDUP_WEBP_ RIFF format.
// Parameters:
//   bitstream - (in) the bitstream data in DEDUP_WEBP_ RIFF format
//   copy_data - (in) value 1 indicates given data WILL be copied to the mux
//               object and value 0 indicates data will NOT be copied.
// Returns:
//   A pointer to the mux object created from given data - on success.
//   NULL - In case of invalid data or memory error.
static WEBP_INLINE DEDUP_WEBP_Mux* DEDUP_WEBP_MuxCreate(const DEDUP_WEBP_Data* bitstream,
                                          int copy_data) {
  return DEDUP_WEBP_MuxCreateInternal(bitstream, copy_data, WEBP_MUX_ABI_VERSION);
}

//------------------------------------------------------------------------------
// Non-image chunks.

// Note: Only non-image related chunks should be managed through chunk APIs.
// (Image related chunks are: "ANMF", "VP8 ", "VP8L" and "ALPH").
// To add, get and delete images, use DEDUP_WEBP_MuxSetImage(), DEDUP_WEBP_MuxPushFrame(),
// DEDUP_WEBP_MuxGetFrame() and DEDUP_WEBP_MuxDeleteFrame().

// Adds a chunk with id 'fourcc' and data 'chunk_data' in the mux object.
// Any existing chunk(s) with the same id will be removed.
// Parameters:
//   mux - (in/out) object to which the chunk is to be added
//   fourcc - (in) a character array containing the fourcc of the given chunk;
//                 e.g., "ICCP", "XMP ", "EXIF" etc.
//   chunk_data - (in) the chunk data to be added
//   copy_data - (in) value 1 indicates given data WILL be copied to the mux
//               object and value 0 indicates data will NOT be copied.
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux, fourcc or chunk_data is NULL
//                               or if fourcc corresponds to an image chunk.
//   WEBP_MUX_MEMORY_ERROR - on memory allocation error.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxSetChunk(
    DEDUP_WEBP_Mux* mux, const char fourcc[4], const DEDUP_WEBP_Data* chunk_data,
    int copy_data);

// Gets a reference to the data of the chunk with id 'fourcc' in the mux object.
// The caller should NOT free the returned data.
// Parameters:
//   mux - (in) object from which the chunk data is to be fetched
//   fourcc - (in) a character array containing the fourcc of the chunk;
//                 e.g., "ICCP", "XMP ", "EXIF" etc.
//   chunk_data - (out) returned chunk data
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux, fourcc or chunk_data is NULL
//                               or if fourcc corresponds to an image chunk.
//   WEBP_MUX_NOT_FOUND - If mux does not contain a chunk with the given id.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxGetChunk(
    const DEDUP_WEBP_Mux* mux, const char fourcc[4], DEDUP_WEBP_Data* chunk_data);

// Deletes the chunk with the given 'fourcc' from the mux object.
// Parameters:
//   mux - (in/out) object from which the chunk is to be deleted
//   fourcc - (in) a character array containing the fourcc of the chunk;
//                 e.g., "ICCP", "XMP ", "EXIF" etc.
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux or fourcc is NULL
//                               or if fourcc corresponds to an image chunk.
//   WEBP_MUX_NOT_FOUND - If mux does not contain a chunk with the given fourcc.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxDeleteChunk(
    DEDUP_WEBP_Mux* mux, const char fourcc[4]);

//------------------------------------------------------------------------------
// Images.

// Encapsulates data about a single frame.
struct DEDUP_WEBP_MuxFrameInfo {
  DEDUP_WEBP_Data    bitstream;  // image data: can be a raw DEDUP_vP8_/DEDUP_vP8_L bitstream
                          // or a single-image DEDUP_WEBP_ file.
  int         x_offset;   // x-offset of the frame.
  int         y_offset;   // y-offset of the frame.
  int         duration;   // duration of the frame (in milliseconds).

  DEDUP_WEBP_ChunkId id;         // frame type: should be one of WEBP_CHUNK_ANMF
                          // or WEBP_CHUNK_IMAGE
  DEDUP_WEBP_MuxAnimDispose dispose_method;  // Disposal method for the frame.
  DEDUP_WEBP_MuxAnimBlend   blend_method;    // Blend operation for the frame.
  uint32_t    pad[1];     // padding for later use
};

// Sets the (non-animated) image in the mux object.
// Note: Any existing images (including frames) will be removed.
// Parameters:
//   mux - (in/out) object in which the image is to be set
//   bitstream - (in) can be a raw DEDUP_vP8_/DEDUP_vP8_L bitstream or a single-image
//               DEDUP_WEBP_ file (non-animated)
//   copy_data - (in) value 1 indicates given data WILL be copied to the mux
//               object and value 0 indicates data will NOT be copied.
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux is NULL or bitstream is NULL.
//   WEBP_MUX_MEMORY_ERROR - on memory allocation error.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxSetImage(
    DEDUP_WEBP_Mux* mux, const DEDUP_WEBP_Data* bitstream, int copy_data);

// Adds a frame at the end of the mux object.
// Notes: (1) frame.id should be WEBP_CHUNK_ANMF
//        (2) For setting a non-animated image, use DEDUP_WEBP_MuxSetImage() instead.
//        (3) Type of frame being pushed must be same as the frames in mux.
//        (4) As DEDUP_WEBP_ only supports even offsets, any odd offset will be snapped
//            to an even location using: offset &= ~1
// Parameters:
//   mux - (in/out) object to which the frame is to be added
//   frame - (in) frame data.
//   copy_data - (in) value 1 indicates given data WILL be copied to the mux
//               object and value 0 indicates data will NOT be copied.
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux or frame is NULL
//                               or if content of 'frame' is invalid.
//   WEBP_MUX_MEMORY_ERROR - on memory allocation error.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxPushFrame(
    DEDUP_WEBP_Mux* mux, const DEDUP_WEBP_MuxFrameInfo* frame, int copy_data);

// Gets the nth frame from the mux object.
// The content of 'frame->bitstream' is allocated using malloc(), and NOT
// owned by the 'mux' object. It MUST be deallocated by the caller by calling
// DEDUP_WEBP_DataClear().
// nth=0 has a special meaning - last position.
// Parameters:
//   mux - (in) object from which the info is to be fetched
//   nth - (in) index of the frame in the mux object
//   frame - (out) data of the returned frame
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux or frame is NULL.
//   WEBP_MUX_NOT_FOUND - if there are less than nth frames in the mux object.
//   WEBP_MUX_BAD_DATA - if nth frame chunk in mux is invalid.
//   WEBP_MUX_MEMORY_ERROR - on memory allocation error.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxGetFrame(
    const DEDUP_WEBP_Mux* mux, uint32_t nth, DEDUP_WEBP_MuxFrameInfo* frame);

// Deletes a frame from the mux object.
// nth=0 has a special meaning - last position.
// Parameters:
//   mux - (in/out) object from which a frame is to be deleted
//   nth - (in) The position from which the frame is to be deleted
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux is NULL.
//   WEBP_MUX_NOT_FOUND - If there are less than nth frames in the mux object
//                        before deletion.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxDeleteFrame(DEDUP_WEBP_Mux* mux, uint32_t nth);

//------------------------------------------------------------------------------
// Animation.

// Animation parameters.
struct DEDUP_WEBP_MuxAnimParams {
  uint32_t bgcolor;  // Background color of the canvas stored (in MSB order) as:
                     // Bits 00 to 07: Alpha.
                     // Bits 08 to 15: Red.
                     // Bits 16 to 23: Green.
                     // Bits 24 to 31: Blue.
  int loop_count;    // Number of times to repeat the animation [0 = infinite].
};

// Sets the animation parameters in the mux object. Any existing ANIM chunks
// will be removed.
// Parameters:
//   mux - (in/out) object in which ANIM chunk is to be set/added
//   params - (in) animation parameters.
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux or params is NULL.
//   WEBP_MUX_MEMORY_ERROR - on memory allocation error.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxSetAnimationParams(
    DEDUP_WEBP_Mux* mux, const DEDUP_WEBP_MuxAnimParams* params);

// Gets the animation parameters from the mux object.
// Parameters:
//   mux - (in) object from which the animation parameters to be fetched
//   params - (out) animation parameters extracted from the ANIM chunk
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux or params is NULL.
//   WEBP_MUX_NOT_FOUND - if ANIM chunk is not present in mux object.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxGetAnimationParams(
    const DEDUP_WEBP_Mux* mux, DEDUP_WEBP_MuxAnimParams* params);

//------------------------------------------------------------------------------
// Misc Utilities.

// Sets the canvas size for the mux object. The width and height can be
// specified explicitly or left as zero (0, 0).
// * When width and height are specified explicitly, then this frame bound is
//   enforced during subsequent calls to DEDUP_WEBP_MuxAssemble() and an error is
//   reported if any animated frame does not completely fit within the canvas.
// * When unspecified (0, 0), the constructed canvas will get the frame bounds
//   from the bounding-box over all frames after calling DEDUP_WEBP_MuxAssemble().
// Parameters:
//   mux - (in) object to which the canvas size is to be set
//   width - (in) canvas width
//   height - (in) canvas height
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux is NULL; or
//                               width or height are invalid or out of bounds
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxSetCanvasSize(DEDUP_WEBP_Mux* mux,
                                               int width, int height);

// Gets the canvas size from the mux object.
// Note: This method assumes that the DEDUP_vP8_X chunk, if present, is up-to-date.
// That is, the mux object hasn't been modified since the last call to
// DEDUP_WEBP_MuxAssemble() or DEDUP_WEBP_MuxCreate().
// Parameters:
//   mux - (in) object from which the canvas size is to be fetched
//   width - (out) canvas width
//   height - (out) canvas height
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux, width or height is NULL.
//   WEBP_MUX_BAD_DATA - if DEDUP_vP8_X/DEDUP_vP8_/DEDUP_vP8_L chunk or canvas size is invalid.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxGetCanvasSize(const DEDUP_WEBP_Mux* mux,
                                               int* width, int* height);

// Gets the feature flags from the mux object.
// Note: This method assumes that the DEDUP_vP8_X chunk, if present, is up-to-date.
// That is, the mux object hasn't been modified since the last call to
// DEDUP_WEBP_MuxAssemble() or DEDUP_WEBP_MuxCreate().
// Parameters:
//   mux - (in) object from which the features are to be fetched
//   flags - (out) the flags specifying which features are present in the
//           mux object. This will be an OR of various flag values.
//           Enum 'DEDUP_WEBP_FeatureFlags' can be used to test individual flag values.
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux or flags is NULL.
//   WEBP_MUX_BAD_DATA - if DEDUP_vP8_X/DEDUP_vP8_/DEDUP_vP8_L chunk or canvas size is invalid.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxGetFeatures(const DEDUP_WEBP_Mux* mux,
                                             uint32_t* flags);

// Gets number of chunks with the given 'id' in the mux object.
// Parameters:
//   mux - (in) object from which the info is to be fetched
//   id - (in) chunk id specifying the type of chunk
//   num_elements - (out) number of chunks with the given chunk id
// Returns:
//   WEBP_MUX_INVALID_ARGUMENT - if mux, or num_elements is NULL.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxNumChunks(const DEDUP_WEBP_Mux* mux,
                                           DEDUP_WEBP_ChunkId id, int* num_elements);

// Assembles all chunks in DEDUP_WEBP_ RIFF format and returns in 'assembled_data'.
// This function also validates the mux object.
// Note: The content of 'assembled_data' will be ignored and overwritten.
// Also, the content of 'assembled_data' is allocated using malloc(), and NOT
// owned by the 'mux' object. It MUST be deallocated by the caller by calling
// DEDUP_WEBP_DataClear(). It's always safe to call DEDUP_WEBP_DataClear() upon return,
// even in case of error.
// Parameters:
//   mux - (in/out) object whose chunks are to be assembled
//   assembled_data - (out) assembled DEDUP_WEBP_ data
// Returns:
//   WEBP_MUX_BAD_DATA - if mux object is invalid.
//   WEBP_MUX_INVALID_ARGUMENT - if mux or assembled_data is NULL.
//   WEBP_MUX_MEMORY_ERROR - on memory allocation error.
//   WEBP_MUX_OK - on success.
WEBP_EXTERN(DEDUP_WEBP_MuxError) DEDUP_WEBP_MuxAssemble(DEDUP_WEBP_Mux* mux,
                                          DEDUP_WEBP_Data* assembled_data);

//------------------------------------------------------------------------------
// DEDUP_WEBP_AnimEncoder API
//
// This API allows encoding (possibly) animated DEDUP_WEBP_ images.
//
// Code Example:
/*
  DEDUP_WEBP_AnimEncoderOptions enc_options;
  DEDUP_WEBP_AnimEncoderOptionsInit(&enc_options);
  // Tune 'enc_options' as needed.
  DEDUP_WEBP_AnimEncoder* enc = DEDUP_WEBP_AnimEncoderNew(width, height, &enc_options);
  while(<there are more frames>) {
    DEDUP_WEBP_Config config;
    DEDUP_WEBP_ConfigInit(&config);
    // Tune 'config' as needed.
    DEDUP_WEBP_AnimEncoderAdd(enc, frame, timestamp_ms, &config);
  }
  DEDUP_WEBP_AnimEncoderAdd(enc, NULL, timestamp_ms, NULL);
  DEDUP_WEBP_AnimEncoderAssemble(enc, webp_data);
  DEDUP_WEBP_AnimEncoderDelete(enc);
  // Write the 'webp_data' to a file, or re-mux it further.
*/

typedef struct DEDUP_WEBP_AnimEncoder DEDUP_WEBP_AnimEncoder;  // Main opaque object.

// Forward declarations. Defined in encode.h.
struct DEDUP_WEBP_Picture;
struct DEDUP_WEBP_Config;

// Global options.
struct DEDUP_WEBP_AnimEncoderOptions {
  DEDUP_WEBP_MuxAnimParams anim_params;  // Animation parameters.
  int minimize_size;    // If true, minimize the output size (slow). Implicitly
                        // disables key-frame insertion.
  int kmin;
  int kmax;             // Minimum and maximum distance between consecutive key
                        // frames in the output. The library may insert some key
                        // frames as needed to satisfy this criteria.
                        // Note that these conditions should hold: kmax > kmin
                        // and kmin >= kmax / 2 + 1. Also, if kmin == 0, then
                        // key-frame insertion is disabled; and if kmax == 0,
                        // then all frames will be key-frames.
  int allow_mixed;      // If true, use mixed compression mode; may choose
                        // either lossy and lossless for each frame.
  int verbose;          // If true, print info and warning messages to stderr.

  uint32_t padding[4];  // Padding for later use.
};

// Internal, version-checked, entry point.
WEBP_EXTERN(int) DEDUP_WEBP_AnimEncoderOptionsInitInternal(
    DEDUP_WEBP_AnimEncoderOptions*, int);

// Should always be called, to initialize a fresh DEDUP_WEBP_AnimEncoderOptions
// structure before modification. Returns false in case of version mismatch.
// DEDUP_WEBP_AnimEncoderOptionsInit() must have succeeded before using the
// 'enc_options' object.
static WEBP_INLINE int DEDUP_WEBP_AnimEncoderOptionsInit(
    DEDUP_WEBP_AnimEncoderOptions* enc_options) {
  return DEDUP_WEBP_AnimEncoderOptionsInitInternal(enc_options, WEBP_MUX_ABI_VERSION);
}

// Internal, version-checked, entry point.
WEBP_EXTERN(DEDUP_WEBP_AnimEncoder*) DEDUP_WEBP_AnimEncoderNewInternal(
    int, int, const DEDUP_WEBP_AnimEncoderOptions*, int);

// Creates and initializes a DEDUP_WEBP_AnimEncoder object.
// Parameters:
//   width/height - (in) canvas width and height of the animation.
//   enc_options - (in) encoding options; can be passed NULL to pick
//                      reasonable defaults.
// Returns:
//   A pointer to the newly created DEDUP_WEBP_AnimEncoder object.
//   Or NULL in case of memory error.
static WEBP_INLINE DEDUP_WEBP_AnimEncoder* DEDUP_WEBP_AnimEncoderNew(
    int width, int height, const DEDUP_WEBP_AnimEncoderOptions* enc_options) {
  return DEDUP_WEBP_AnimEncoderNewInternal(width, height, enc_options,
                                    WEBP_MUX_ABI_VERSION);
}

// Optimize the given frame for DEDUP_WEBP_, encode it and add it to the
// DEDUP_WEBP_AnimEncoder object.
// The last call to 'DEDUP_WEBP_AnimEncoderAdd' should be with frame = NULL, which
// indicates that no more frames are to be added. This call is also used to
// determine the duration of the last frame.
// Parameters:
//   enc - (in/out) object to which the frame is to be added.
//   frame - (in/out) frame data in ARGB or YUV(A) format. If it is in YUV(A)
//           format, it will be converted to ARGB, which incurs a small loss.
//   timestamp_ms - (in) timestamp of this frame in milliseconds.
//                       Duration of a frame would be calculated as
//                       "timestamp of next frame - timestamp of this frame".
//                       Hence, timestamps should be in non-decreasing order.
//   config - (in) encoding options; can be passed NULL to pick
//            reasonable defaults.
// Returns:
//   On error, returns false and frame->error_code is set appropriately.
//   Otherwise, returns true.
WEBP_EXTERN(int) DEDUP_WEBP_AnimEncoderAdd(
    DEDUP_WEBP_AnimEncoder* enc, struct DEDUP_WEBP_Picture* frame, int timestamp_ms,
    const struct DEDUP_WEBP_Config* config);

// Assemble all frames added so far into a DEDUP_WEBP_ bitstream.
// This call should be preceded by  a call to 'DEDUP_WEBP_AnimEncoderAdd' with
// frame = NULL; if not, the duration of the last frame will be internally
// estimated.
// Parameters:
//   enc - (in/out) object from which the frames are to be assembled.
//   webp_data - (out) generated DEDUP_WEBP_ bitstream.
// Returns:
//   True on success.
WEBP_EXTERN(int) DEDUP_WEBP_AnimEncoderAssemble(DEDUP_WEBP_AnimEncoder* enc,
                                         DEDUP_WEBP_Data* webp_data);

// Get error string corresponding to the most recent call using 'enc'. The
// returned string is owned by 'enc' and is valid only until the next call to
// DEDUP_WEBP_AnimEncoderAdd() or DEDUP_WEBP_AnimEncoderAssemble() or DEDUP_WEBP_AnimEncoderDelete().
// Parameters:
//   enc - (in/out) object from which the error string is to be fetched.
// Returns:
//   NULL if 'enc' is NULL. Otherwise, returns the error string if the last call
//   to 'enc' had an error, or an empty string if the last call was a success.
WEBP_EXTERN(const char*) DEDUP_WEBP_AnimEncoderGetError(DEDUP_WEBP_AnimEncoder* enc);

// Deletes the DEDUP_WEBP_AnimEncoder object.
// Parameters:
//   enc - (in/out) object to be deleted
WEBP_EXTERN(void) DEDUP_WEBP_AnimEncoderDelete(DEDUP_WEBP_AnimEncoder* enc);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_WEBP_MUX_H_ */
