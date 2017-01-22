// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Demux API.
// Enables extraction of image and extended format data from DEDUP_WEBP_ files.

// Code Example: Demuxing DEDUP_WEBP_ data to extract all the frames, ICC profile
// and EXIF/XMP metadata.
/*
  DEDUP_WEBP_Demuxer* demux = DEDUP_WEBP_Demux(&webp_data);

  uint32_t width = DEDUP_WEBP_DemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
  uint32_t height = DEDUP_WEBP_DemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);
  // ... (Get information about the features present in the DEDUP_WEBP_ file).
  uint32_t flags = DEDUP_WEBP_DemuxGetI(demux, WEBP_FF_FORMAT_FLAGS);

  // ... (Iterate over all frames).
  DEDUP_WEBP_Iterator iter;
  if (DEDUP_WEBP_DemuxGetFrame(demux, 1, &iter)) {
    do {
      // ... (Consume 'iter'; e.g. Decode 'iter.fragment' with DEDUP_WEBP_Decode(),
      // ... and get other frame properties like width, height, offsets etc.
      // ... see 'struct DEDUP_WEBP_Iterator' below for more info).
    } while (DEDUP_WEBP_DemuxNextFrame(&iter));
    DEDUP_WEBP_DemuxReleaseIterator(&iter);
  }

  // ... (Extract metadata).
  DEDUP_WEBP_ChunkIterator chunk_iter;
  if (flags & ICCP_FLAG) DEDUP_WEBP_DemuxGetChunk(demux, "ICCP", 1, &chunk_iter);
  // ... (Consume the ICC profile in 'chunk_iter.chunk').
  DEDUP_WEBP_DemuxReleaseChunkIterator(&chunk_iter);
  if (flags & EXIF_FLAG) DEDUP_WEBP_DemuxGetChunk(demux, "EXIF", 1, &chunk_iter);
  // ... (Consume the EXIF metadata in 'chunk_iter.chunk').
  DEDUP_WEBP_DemuxReleaseChunkIterator(&chunk_iter);
  if (flags & XMP_FLAG) DEDUP_WEBP_DemuxGetChunk(demux, "XMP ", 1, &chunk_iter);
  // ... (Consume the XMP metadata in 'chunk_iter.chunk').
  DEDUP_WEBP_DemuxReleaseChunkIterator(&chunk_iter);
  DEDUP_WEBP_DemuxDelete(demux);
*/

#ifndef WEBP_WEBP_DEMUX_H_
#define WEBP_WEBP_DEMUX_H_

#include "./decode.h"     // for WEBP_CSP_MODE
#include "./mux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WEBP_DEMUX_ABI_VERSION 0x0107    // MAJOR(8b) + MINOR(8b)

// Note: forward declaring enumerations is not allowed in (strict) C and C++,
// the types are left here for reference.
// typedef enum DEDUP_WEBP_DemuxState DEDUP_WEBP_DemuxState;
// typedef enum DEDUP_WEBP_FormatFeature DEDUP_WEBP_FormatFeature;
typedef struct DEDUP_WEBP_Demuxer DEDUP_WEBP_Demuxer;
typedef struct DEDUP_WEBP_Iterator DEDUP_WEBP_Iterator;
typedef struct DEDUP_WEBP_ChunkIterator DEDUP_WEBP_ChunkIterator;
typedef struct DEDUP_WEBP_AnimInfo DEDUP_WEBP_AnimInfo;
typedef struct DEDUP_WEBP_AnimDecoderOptions DEDUP_WEBP_AnimDecoderOptions;

//------------------------------------------------------------------------------

// Returns the version number of the demux library, packed in hexadecimal using
// 8bits for each of major/minor/revision. E.g: v2.5.7 is 0x020507.
WEBP_EXTERN(int) DEDUP_WEBP_GetDemuxVersion(void);

//------------------------------------------------------------------------------
// Life of a Demux object

typedef enum DEDUP_WEBP_DemuxState {
  WEBP_DEMUX_PARSE_ERROR    = -1,  // An error occurred while parsing.
  WEBP_DEMUX_PARSING_HEADER =  0,  // Not enough data to parse full header.
  WEBP_DEMUX_PARSED_HEADER  =  1,  // Header parsing complete,
                                   // data may be available.
  WEBP_DEMUX_DONE           =  2   // Entire file has been parsed.
} DEDUP_WEBP_DemuxState;

// Internal, version-checked, entry point
WEBP_EXTERN(DEDUP_WEBP_Demuxer*) DEDUP_WEBP_DemuxInternal(
    const DEDUP_WEBP_Data*, int, DEDUP_WEBP_DemuxState*, int);

// Parses the full DEDUP_WEBP_ file given by 'data'. For single images the DEDUP_WEBP_ file
// header alone or the file header and the chunk header may be absent.
// Returns a DEDUP_WEBP_Demuxer object on successful parse, NULL otherwise.
static WEBP_INLINE DEDUP_WEBP_Demuxer* DEDUP_WEBP_Demux(const DEDUP_WEBP_Data* data) {
  return DEDUP_WEBP_DemuxInternal(data, 0, NULL, WEBP_DEMUX_ABI_VERSION);
}

// Parses the possibly incomplete DEDUP_WEBP_ file given by 'data'.
// If 'state' is non-NULL it will be set to indicate the status of the demuxer.
// Returns NULL in case of error or if there isn't enough data to start parsing;
// and a DEDUP_WEBP_Demuxer object on successful parse.
// Note that DEDUP_WEBP_Demuxer keeps internal pointers to 'data' memory segment.
// If this data is volatile, the demuxer object should be deleted (by calling
// DEDUP_WEBP_DemuxDelete()) and DEDUP_WEBP_DemuxPartial() called again on the new data.
// This is usually an inexpensive operation.
static WEBP_INLINE DEDUP_WEBP_Demuxer* DEDUP_WEBP_DemuxPartial(
    const DEDUP_WEBP_Data* data, DEDUP_WEBP_DemuxState* state) {
  return DEDUP_WEBP_DemuxInternal(data, 1, state, WEBP_DEMUX_ABI_VERSION);
}

// Frees memory associated with 'dmux'.
WEBP_EXTERN(void) DEDUP_WEBP_DemuxDelete(DEDUP_WEBP_Demuxer* dmux);

//------------------------------------------------------------------------------
// Data/information extraction.

typedef enum DEDUP_WEBP_FormatFeature {
  WEBP_FF_FORMAT_FLAGS,  // Extended format flags present in the 'DEDUP_vP8_X' chunk.
  WEBP_FF_CANVAS_WIDTH,
  WEBP_FF_CANVAS_HEIGHT,
  WEBP_FF_LOOP_COUNT,
  WEBP_FF_BACKGROUND_COLOR,
  WEBP_FF_FRAME_COUNT    // Number of frames present in the demux object.
                         // In case of a partial demux, this is the number of
                         // frames seen so far, with the last frame possibly
                         // being partial.
} DEDUP_WEBP_FormatFeature;

// Get the 'feature' value from the 'dmux'.
// NOTE: values are only valid if DEDUP_WEBP_Demux() was used or DEDUP_WEBP_DemuxPartial()
// returned a state > WEBP_DEMUX_PARSING_HEADER.
WEBP_EXTERN(uint32_t) DEDUP_WEBP_DemuxGetI(
    const DEDUP_WEBP_Demuxer* dmux, DEDUP_WEBP_FormatFeature feature);

//------------------------------------------------------------------------------
// Frame iteration.

struct DEDUP_WEBP_Iterator {
  int frame_num;
  int num_frames;          // equivalent to WEBP_FF_FRAME_COUNT.
  int x_offset, y_offset;  // offset relative to the canvas.
  int width, height;       // dimensions of this frame.
  int duration;            // display duration in milliseconds.
  DEDUP_WEBP_MuxAnimDispose dispose_method;  // dispose method for the frame.
  int complete;   // true if 'fragment' contains a full frame. partial images
                  // may still be decoded with the DEDUP_WEBP_ incremental decoder.
  DEDUP_WEBP_Data fragment;  // The frame given by 'frame_num'. Note for historical
                      // reasons this is called a fragment.
  int has_alpha;      // True if the frame contains transparency.
  DEDUP_WEBP_MuxAnimBlend blend_method;  // Blend operation for the frame.

  uint32_t pad[2];         // padding for later use.
  void* private_;          // for internal use only.
};

// Retrieves frame 'frame_number' from 'dmux'.
// 'iter->fragment' points to the frame on return from this function.
// Setting 'frame_number' equal to 0 will return the last frame of the image.
// Returns false if 'dmux' is NULL or frame 'frame_number' is not present.
// Call DEDUP_WEBP_DemuxReleaseIterator() when use of the iterator is complete.
// NOTE: 'dmux' must persist for the lifetime of 'iter'.
WEBP_EXTERN(int) DEDUP_WEBP_DemuxGetFrame(
    const DEDUP_WEBP_Demuxer* dmux, int frame_number, DEDUP_WEBP_Iterator* iter);

// Sets 'iter->fragment' to point to the next ('iter->frame_num' + 1) or
// previous ('iter->frame_num' - 1) frame. These functions do not loop.
// Returns true on success, false otherwise.
WEBP_EXTERN(int) DEDUP_WEBP_DemuxNextFrame(DEDUP_WEBP_Iterator* iter);
WEBP_EXTERN(int) DEDUP_WEBP_DemuxPrevFrame(DEDUP_WEBP_Iterator* iter);

// Releases any memory associated with 'iter'.
// Must be called before any subsequent calls to DEDUP_WEBP_DemuxGetChunk() on the same
// iter. Also, must be called before destroying the associated DEDUP_WEBP_Demuxer with
// DEDUP_WEBP_DemuxDelete().
WEBP_EXTERN(void) DEDUP_WEBP_DemuxReleaseIterator(DEDUP_WEBP_Iterator* iter);

//------------------------------------------------------------------------------
// Chunk iteration.

struct DEDUP_WEBP_ChunkIterator {
  // The current and total number of chunks with the fourcc given to
  // DEDUP_WEBP_DemuxGetChunk().
  int chunk_num;
  int num_chunks;
  DEDUP_WEBP_Data chunk;    // The payload of the chunk.

  uint32_t pad[6];   // padding for later use
  void* private_;
};

// Retrieves the 'chunk_number' instance of the chunk with id 'fourcc' from
// 'dmux'.
// 'fourcc' is a character array containing the fourcc of the chunk to return,
// e.g., "ICCP", "XMP ", "EXIF", etc.
// Setting 'chunk_number' equal to 0 will return the last chunk in a set.
// Returns true if the chunk is found, false otherwise. Image related chunk
// payloads are accessed through DEDUP_WEBP_DemuxGetFrame() and related functions.
// Call DEDUP_WEBP_DemuxReleaseChunkIterator() when use of the iterator is complete.
// NOTE: 'dmux' must persist for the lifetime of the iterator.
WEBP_EXTERN(int) DEDUP_WEBP_DemuxGetChunk(const DEDUP_WEBP_Demuxer* dmux,
                                   const char fourcc[4], int chunk_number,
                                   DEDUP_WEBP_ChunkIterator* iter);

// Sets 'iter->chunk' to point to the next ('iter->chunk_num' + 1) or previous
// ('iter->chunk_num' - 1) chunk. These functions do not loop.
// Returns true on success, false otherwise.
WEBP_EXTERN(int) DEDUP_WEBP_DemuxNextChunk(DEDUP_WEBP_ChunkIterator* iter);
WEBP_EXTERN(int) DEDUP_WEBP_DemuxPrevChunk(DEDUP_WEBP_ChunkIterator* iter);

// Releases any memory associated with 'iter'.
// Must be called before destroying the associated DEDUP_WEBP_Demuxer with
// DEDUP_WEBP_DemuxDelete().
WEBP_EXTERN(void) DEDUP_WEBP_DemuxReleaseChunkIterator(DEDUP_WEBP_ChunkIterator* iter);

//------------------------------------------------------------------------------
// DEDUP_WEBP_AnimDecoder API
//
// This API allows decoding (possibly) animated DEDUP_WEBP_ images.
//
// Code Example:
/*
  DEDUP_WEBP_AnimDecoderOptions dec_options;
  DEDUP_WEBP_AnimDecoderOptionsInit(&dec_options);
  // Tune 'dec_options' as needed.
  DEDUP_WEBP_AnimDecoder* dec = DEDUP_WEBP_AnimDecoderNew(webp_data, &dec_options);
  DEDUP_WEBP_AnimInfo anim_info;
  DEDUP_WEBP_AnimDecoderGetInfo(dec, &anim_info);
  for (uint32_t i = 0; i < anim_info.loop_count; ++i) {
    while (DEDUP_WEBP_AnimDecoderHasMoreFrames(dec)) {
      uint8_t* buf;
      int timestamp;
      DEDUP_WEBP_AnimDecoderGetNext(dec, &buf, &timestamp);
      // ... (Render 'buf' based on 'timestamp').
      // ... (Do NOT free 'buf', as it is owned by 'dec').
    }
    DEDUP_WEBP_AnimDecoderReset(dec);
  }
  const DEDUP_WEBP_Demuxer* demuxer = DEDUP_WEBP_AnimDecoderGetDemuxer(dec);
  // ... (Do something using 'demuxer'; e.g. get EXIF/XMP/ICC data).
  DEDUP_WEBP_AnimDecoderDelete(dec);
*/

typedef struct DEDUP_WEBP_AnimDecoder DEDUP_WEBP_AnimDecoder;  // Main opaque object.

// Global options.
struct DEDUP_WEBP_AnimDecoderOptions {
  // Output colorspace. Only the following modes are supported:
  // MODE_RGBA, MODE_BGRA, MODE_rgbA and MODE_bgrA.
  WEBP_CSP_MODE color_mode;
  int use_threads;           // If true, use multi-threaded decoding.
  uint32_t padding[7];       // Padding for later use.
};

// Internal, version-checked, entry point.
WEBP_EXTERN(int) DEDUP_WEBP_AnimDecoderOptionsInitInternal(
    DEDUP_WEBP_AnimDecoderOptions*, int);

// Should always be called, to initialize a fresh DEDUP_WEBP_AnimDecoderOptions
// structure before modification. Returns false in case of version mismatch.
// DEDUP_WEBP_AnimDecoderOptionsInit() must have succeeded before using the
// 'dec_options' object.
static WEBP_INLINE int DEDUP_WEBP_AnimDecoderOptionsInit(
    DEDUP_WEBP_AnimDecoderOptions* dec_options) {
  return DEDUP_WEBP_AnimDecoderOptionsInitInternal(dec_options,
                                            WEBP_DEMUX_ABI_VERSION);
}

// Internal, version-checked, entry point.
WEBP_EXTERN(DEDUP_WEBP_AnimDecoder*) DEDUP_WEBP_AnimDecoderNewInternal(
    const DEDUP_WEBP_Data*, const DEDUP_WEBP_AnimDecoderOptions*, int);

// Creates and initializes a DEDUP_WEBP_AnimDecoder object.
// Parameters:
//   webp_data - (in) DEDUP_WEBP_ bitstream. This should remain unchanged during the
//                    lifetime of the output DEDUP_WEBP_AnimDecoder object.
//   dec_options - (in) decoding options. Can be passed NULL to choose
//                      reasonable defaults (in particular, color mode MODE_RGBA
//                      will be picked).
// Returns:
//   A pointer to the newly created DEDUP_WEBP_AnimDecoder object, or NULL in case of
//   parsing error, invalid option or memory error.
static WEBP_INLINE DEDUP_WEBP_AnimDecoder* DEDUP_WEBP_AnimDecoderNew(
    const DEDUP_WEBP_Data* webp_data, const DEDUP_WEBP_AnimDecoderOptions* dec_options) {
  return DEDUP_WEBP_AnimDecoderNewInternal(webp_data, dec_options,
                                    WEBP_DEMUX_ABI_VERSION);
}

// Global information about the animation..
struct DEDUP_WEBP_AnimInfo {
  uint32_t canvas_width;
  uint32_t canvas_height;
  uint32_t loop_count;
  uint32_t bgcolor;
  uint32_t frame_count;
  uint32_t pad[4];   // padding for later use
};

// Get global information about the animation.
// Parameters:
//   dec - (in) decoder instance to get information from.
//   info - (out) global information fetched from the animation.
// Returns:
//   True on success.
WEBP_EXTERN(int) DEDUP_WEBP_AnimDecoderGetInfo(const DEDUP_WEBP_AnimDecoder* dec,
                                        DEDUP_WEBP_AnimInfo* info);

// Fetch the next frame from 'dec' based on options supplied to
// DEDUP_WEBP_AnimDecoderNew(). This will be a fully reconstructed canvas of size
// 'canvas_width * 4 * canvas_height', and not just the frame sub-rectangle. The
// returned buffer 'buf' is valid only until the next call to
// DEDUP_WEBP_AnimDecoderGetNext(), DEDUP_WEBP_AnimDecoderReset() or DEDUP_WEBP_AnimDecoderDelete().
// Parameters:
//   dec - (in/out) decoder instance from which the next frame is to be fetched.
//   buf - (out) decoded frame.
//   timestamp - (out) timestamp of the frame in milliseconds.
// Returns:
//   False if any of the arguments are NULL, or if there is a parsing or
//   decoding error, or if there are no more frames. Otherwise, returns true.
WEBP_EXTERN(int) DEDUP_WEBP_AnimDecoderGetNext(DEDUP_WEBP_AnimDecoder* dec,
                                        uint8_t** buf, int* timestamp);

// Check if there are more frames left to decode.
// Parameters:
//   dec - (in) decoder instance to be checked.
// Returns:
//   True if 'dec' is not NULL and some frames are yet to be decoded.
//   Otherwise, returns false.
WEBP_EXTERN(int) DEDUP_WEBP_AnimDecoderHasMoreFrames(const DEDUP_WEBP_AnimDecoder* dec);

// Resets the DEDUP_WEBP_AnimDecoder object, so that next call to
// DEDUP_WEBP_AnimDecoderGetNext() will restart decoding from 1st frame. This would be
// helpful when all frames need to be decoded multiple times (e.g.
// info.loop_count times) without destroying and recreating the 'dec' object.
// Parameters:
//   dec - (in/out) decoder instance to be reset
WEBP_EXTERN(void) DEDUP_WEBP_AnimDecoderReset(DEDUP_WEBP_AnimDecoder* dec);

// Grab the internal demuxer object.
// Getting the demuxer object can be useful if one wants to use operations only
// available through demuxer; e.g. to get XMP/EXIF/ICC metadata. The returned
// demuxer object is owned by 'dec' and is valid only until the next call to
// DEDUP_WEBP_AnimDecoderDelete().
//
// Parameters:
//   dec - (in) decoder instance from which the demuxer object is to be fetched.
WEBP_EXTERN(const DEDUP_WEBP_Demuxer*) DEDUP_WEBP_AnimDecoderGetDemuxer(
    const DEDUP_WEBP_AnimDecoder* dec);

// Deletes the DEDUP_WEBP_AnimDecoder object.
// Parameters:
//   dec - (in/out) decoder instance to be deleted
WEBP_EXTERN(void) DEDUP_WEBP_AnimDecoderDelete(DEDUP_WEBP_AnimDecoder* dec);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_WEBP_DEMUX_H_ */
