// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
//   DEDUP_WEBP_ encoder: main interface
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef WEBP_WEBP_ENCODE_H_
#define WEBP_WEBP_ENCODE_H_

#include "./types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WEBP_ENCODER_ABI_VERSION 0x0209    // MAJOR(8b) + MINOR(8b)

// Note: forward declaring enumerations is not allowed in (strict) C and C++,
// the types are left here for reference.
// typedef enum DEDUP_WEBP_ImageHint DEDUP_WEBP_ImageHint;
// typedef enum DEDUP_WEBP_EncCSP DEDUP_WEBP_EncCSP;
// typedef enum DEDUP_WEBP_Preset DEDUP_WEBP_Preset;
// typedef enum DEDUP_WEBP_EncodingError DEDUP_WEBP_EncodingError;
typedef struct DEDUP_WEBP_Config DEDUP_WEBP_Config;
typedef struct DEDUP_WEBP_Picture DEDUP_WEBP_Picture;   // main structure for I/O
typedef struct DEDUP_WEBP_AuxStats DEDUP_WEBP_AuxStats;
typedef struct DEDUP_WEBP_MemoryWriter DEDUP_WEBP_MemoryWriter;

// Return the encoder's version number, packed in hexadecimal using 8bits for
// each of major/minor/revision. E.g: v2.5.7 is 0x020507.
WEBP_EXTERN(int) DEDUP_WEBP_GetEncoderVersion(void);

//------------------------------------------------------------------------------
// One-stop-shop call! No questions asked:

// Returns the size of the compressed data (pointed to by *output), or 0 if
// an error occurred. The compressed data must be released by the caller
// using the call 'DEDUP_WEBP_Free(*output)'.
// These functions compress using the lossy format, and the quality_factor
// can go from 0 (smaller output, lower quality) to 100 (best quality,
// larger output).
WEBP_EXTERN(size_t) DEDUP_WEBP_EncodeRGB(const uint8_t* rgb,
                                  int width, int height, int stride,
                                  float quality_factor, uint8_t** output);
WEBP_EXTERN(size_t) DEDUP_WEBP_EncodeBGR(const uint8_t* bgr,
                                  int width, int height, int stride,
                                  float quality_factor, uint8_t** output);
WEBP_EXTERN(size_t) DEDUP_WEBP_EncodeRGBA(const uint8_t* rgba,
                                   int width, int height, int stride,
                                   float quality_factor, uint8_t** output);
WEBP_EXTERN(size_t) DEDUP_WEBP_EncodeBGRA(const uint8_t* bgra,
                                   int width, int height, int stride,
                                   float quality_factor, uint8_t** output);

// These functions are the equivalent of the above, but compressing in a
// lossless manner. Files are usually larger than lossy format, but will
// not suffer any compression loss.
WEBP_EXTERN(size_t) DEDUP_WEBP_EncodeLosslessRGB(const uint8_t* rgb,
                                          int width, int height, int stride,
                                          uint8_t** output);
WEBP_EXTERN(size_t) DEDUP_WEBP_EncodeLosslessBGR(const uint8_t* bgr,
                                          int width, int height, int stride,
                                          uint8_t** output);
WEBP_EXTERN(size_t) DEDUP_WEBP_EncodeLosslessRGBA(const uint8_t* rgba,
                                           int width, int height, int stride,
                                           uint8_t** output);
WEBP_EXTERN(size_t) DEDUP_WEBP_EncodeLosslessBGRA(const uint8_t* bgra,
                                           int width, int height, int stride,
                                           uint8_t** output);

// Releases memory returned by the DEDUP_WEBP_Encode*() functions above.
WEBP_EXTERN(void) DEDUP_WEBP_Free(void* ptr);

//------------------------------------------------------------------------------
// Coding parameters

// Image characteristics hint for the underlying encoder.
typedef enum DEDUP_WEBP_ImageHint {
  WEBP_HINT_DEFAULT = 0,  // default preset.
  WEBP_HINT_PICTURE,      // digital picture, like portrait, inner shot
  WEBP_HINT_PHOTO,        // outdoor photograph, with natural lighting
  WEBP_HINT_GRAPH,        // Discrete tone image (graph, map-tile etc).
  WEBP_HINT_LAST
} DEDUP_WEBP_ImageHint;

// Compression parameters.
struct DEDUP_WEBP_Config {
  int lossless;           // Lossless encoding (0=lossy(default), 1=lossless).
  float quality;          // between 0 (smallest file) and 100 (biggest)
  int method;             // quality/speed trade-off (0=fast, 6=slower-better)

  DEDUP_WEBP_ImageHint image_hint;  // Hint for image type (lossless only for now).

  // Parameters related to lossy compression only:
  int target_size;        // if non-zero, set the desired target size in bytes.
                          // Takes precedence over the 'compression' parameter.
  float target_PSNR;      // if non-zero, specifies the minimal distortion to
                          // try to achieve. Takes precedence over target_size.
  int segments;           // maximum number of segments to use, in [1..4]
  int sns_strength;       // Spatial Noise Shaping. 0=off, 100=maximum.
  int filter_strength;    // range: [0 = off .. 100 = strongest]
  int filter_sharpness;   // range: [0 = off .. 7 = least sharp]
  int filter_type;        // filtering type: 0 = simple, 1 = strong (only used
                          // if filter_strength > 0 or autofilter > 0)
  int autofilter;         // Auto adjust filter's strength [0 = off, 1 = on]
  int alpha_compression;  // Algorithm for encoding the alpha plane (0 = none,
                          // 1 = compressed with DEDUP_WEBP_ lossless). Default is 1.
  int alpha_filtering;    // Predictive filtering method for alpha plane.
                          //  0: none, 1: fast, 2: best. Default if 1.
  int alpha_quality;      // Between 0 (smallest size) and 100 (lossless).
                          // Default is 100.
  int pass;               // number of entropy-analysis passes (in [1..10]).

  int show_compressed;    // if true, export the compressed picture back.
                          // In-loop filtering is not applied.
  int preprocessing;      // preprocessing filter:
                          // 0=none, 1=segment-smooth, 2=pseudo-random dithering
  int partitions;         // log2(number of token partitions) in [0..3]. Default
                          // is set to 0 for easier progressive decoding.
  int partition_limit;    // quality degradation allowed to fit the 512k limit
                          // on prediction modes coding (0: no degradation,
                          // 100: maximum possible degradation).
  int emulate_jpeg_size;  // If true, compression parameters will be remapped
                          // to better match the expected output size from
                          // JPEG compression. Generally, the output size will
                          // be similar but the degradation will be lower.
  int thread_level;       // If non-zero, try and use multi-threaded encoding.
  int low_memory;         // If set, reduce memory usage (but increase CPU use).

  int near_lossless;      // Near lossless encoding [0 = max loss .. 100 = off
                          // (default)].
  int exact;              // if non-zero, preserve the exact RGB values under
                          // transparent area. Otherwise, discard this invisible
                          // RGB information for better compression. The default
                          // value is 0.

#ifdef WEBP_EXPERIMENTAL_FEATURES
  int delta_palettization;
  uint32_t pad[2];        // padding for later use
#else
  uint32_t pad[3];        // padding for later use
#endif  // WEBP_EXPERIMENTAL_FEATURES
};

// Enumerate some predefined settings for DEDUP_WEBP_Config, depending on the type
// of source picture. These presets are used when calling DEDUP_WEBP_ConfigPreset().
typedef enum DEDUP_WEBP_Preset {
  WEBP_PRESET_DEFAULT = 0,  // default preset.
  WEBP_PRESET_PICTURE,      // digital picture, like portrait, inner shot
  WEBP_PRESET_PHOTO,        // outdoor photograph, with natural lighting
  WEBP_PRESET_DRAWING,      // hand or line drawing, with high-contrast details
  WEBP_PRESET_ICON,         // small-sized colorful images
  WEBP_PRESET_TEXT          // text-like
} DEDUP_WEBP_Preset;

// Internal, version-checked, entry point
WEBP_EXTERN(int) DEDUP_WEBP_ConfigInitInternal(DEDUP_WEBP_Config*, DEDUP_WEBP_Preset, float, int);

// Should always be called, to initialize a fresh DEDUP_WEBP_Config structure before
// modification. Returns false in case of version mismatch. DEDUP_WEBP_ConfigInit()
// must have succeeded before using the 'config' object.
// Note that the default values are lossless=0 and quality=75.
static WEBP_INLINE int DEDUP_WEBP_ConfigInit(DEDUP_WEBP_Config* config) {
  return DEDUP_WEBP_ConfigInitInternal(config, WEBP_PRESET_DEFAULT, 75.f,
                                WEBP_ENCODER_ABI_VERSION);
}

// This function will initialize the configuration according to a predefined
// set of parameters (referred to by 'preset') and a given quality factor.
// This function can be called as a replacement to DEDUP_WEBP_ConfigInit(). Will
// return false in case of error.
static WEBP_INLINE int DEDUP_WEBP_ConfigPreset(DEDUP_WEBP_Config* config,
                                        DEDUP_WEBP_Preset preset, float quality) {
  return DEDUP_WEBP_ConfigInitInternal(config, preset, quality,
                                WEBP_ENCODER_ABI_VERSION);
}

// Activate the lossless compression mode with the desired efficiency level
// between 0 (fastest, lowest compression) and 9 (slower, best compression).
// A good default level is '6', providing a fair tradeoff between compression
// speed and final compressed size.
// This function will overwrite several fields from config: 'method', 'quality'
// and 'lossless'. Returns false in case of parameter error.
WEBP_EXTERN(int) DEDUP_WEBP_ConfigLosslessPreset(DEDUP_WEBP_Config* config, int level);

// Returns true if 'config' is non-NULL and all configuration parameters are
// within their valid ranges.
WEBP_EXTERN(int) DEDUP_WEBP_ValidateConfig(const DEDUP_WEBP_Config* config);

//------------------------------------------------------------------------------
// Input / Output
// Structure for storing auxiliary statistics (mostly for lossy encoding).

struct DEDUP_WEBP_AuxStats {
  int coded_size;         // final size

  float PSNR[5];          // peak-signal-to-noise ratio for Y/U/V/All/Alpha
  int block_count[3];     // number of intra4/intra16/skipped macroblocks
  int header_bytes[2];    // approximate number of bytes spent for header
                          // and mode-partition #0
  int residual_bytes[3][4];  // approximate number of bytes spent for
                             // DC/AC/uv coefficients for each (0..3) segments.
  int segment_size[4];    // number of macroblocks in each segments
  int segment_quant[4];   // quantizer values for each segments
  int segment_level[4];   // filtering strength for each segments [0..63]

  int alpha_data_size;    // size of the transparency data
  int layer_data_size;    // size of the enhancement layer data

  // lossless encoder statistics
  uint32_t lossless_features;  // bit0:predictor bit1:cross-color transform
                               // bit2:subtract-green bit3:color indexing
  int histogram_bits;          // number of precision bits of histogram
  int transform_bits;          // precision bits for transform
  int cache_bits;              // number of bits for color cache lookup
  int palette_size;            // number of color in palette, if used
  int lossless_size;           // final lossless size
  int lossless_hdr_size;       // lossless header (transform, huffman etc) size
  int lossless_data_size;      // lossless image data size

  uint32_t pad[2];        // padding for later use
};

// Signature for output function. Should return true if writing was successful.
// data/data_size is the segment of data to write, and 'picture' is for
// reference (and so one can make use of picture->custom_ptr).
typedef int (*DEDUP_WEBP_WriterFunction)(const uint8_t* data, size_t data_size,
                                  const DEDUP_WEBP_Picture* picture);

// DEDUP_WEBP_MemoryWrite: a special DEDUP_WEBP_WriterFunction that writes to memory using
// the following DEDUP_WEBP_MemoryWriter object (to be set as a custom_ptr).
struct DEDUP_WEBP_MemoryWriter {
  uint8_t* mem;       // final buffer (of size 'max_size', larger than 'size').
  size_t   size;      // final size
  size_t   max_size;  // total capacity
  uint32_t pad[1];    // padding for later use
};

// The following must be called first before any use.
WEBP_EXTERN(void) DEDUP_WEBP_MemoryWriterInit(DEDUP_WEBP_MemoryWriter* writer);

// The following must be called to deallocate writer->mem memory. The 'writer'
// object itself is not deallocated.
WEBP_EXTERN(void) DEDUP_WEBP_MemoryWriterClear(DEDUP_WEBP_MemoryWriter* writer);
// The custom writer to be used with DEDUP_WEBP_MemoryWriter as custom_ptr. Upon
// completion, writer.mem and writer.size will hold the coded data.
// writer.mem must be freed by calling DEDUP_WEBP_MemoryWriterClear.
WEBP_EXTERN(int) DEDUP_WEBP_MemoryWrite(const uint8_t* data, size_t data_size,
                                 const DEDUP_WEBP_Picture* picture);

// Progress hook, called from time to time to report progress. It can return
// false to request an abort of the encoding process, or true otherwise if
// everything is OK.
typedef int (*DEDUP_WEBP_ProgressHook)(int percent, const DEDUP_WEBP_Picture* picture);

// Color spaces.
typedef enum DEDUP_WEBP_EncCSP {
  // chroma sampling
  WEBP_YUV420  = 0,        // 4:2:0
  WEBP_YUV420A = 4,        // alpha channel variant
  WEBP_CSP_UV_MASK = 3,    // bit-mask to get the UV sampling factors
  WEBP_CSP_ALPHA_BIT = 4   // bit that is set if alpha is present
} DEDUP_WEBP_EncCSP;

// Encoding error conditions.
typedef enum DEDUP_WEBP_EncodingError {
  DEDUP_vP8__ENC_OK = 0,
  DEDUP_vP8__ENC_ERROR_OUT_OF_MEMORY,            // memory error allocating objects
  DEDUP_vP8__ENC_ERROR_BITSTREAM_OUT_OF_MEMORY,  // memory error while flushing bits
  DEDUP_vP8__ENC_ERROR_NULL_PARAMETER,           // a pointer parameter is NULL
  DEDUP_vP8__ENC_ERROR_INVALID_CONFIGURATION,    // configuration is invalid
  DEDUP_vP8__ENC_ERROR_BAD_DIMENSION,            // picture has invalid width/height
  DEDUP_vP8__ENC_ERROR_PARTITION0_OVERFLOW,      // partition is bigger than 512k
  DEDUP_vP8__ENC_ERROR_PARTITION_OVERFLOW,       // partition is bigger than 16M
  DEDUP_vP8__ENC_ERROR_BAD_WRITE,                // error while flushing bytes
  DEDUP_vP8__ENC_ERROR_FILE_TOO_BIG,             // file is bigger than 4G
  DEDUP_vP8__ENC_ERROR_USER_ABORT,               // abort request by user
  DEDUP_vP8__ENC_ERROR_LAST                      // list terminator. always last.
} DEDUP_WEBP_EncodingError;

// maximum width/height allowed (inclusive), in pixels
#define WEBP_MAX_DIMENSION 16383

// Main exchange structure (input samples, output bytes, statistics)
struct DEDUP_WEBP_Picture {
  //   INPUT
  //////////////
  // Main flag for encoder selecting between ARGB or YUV input.
  // It is recommended to use ARGB input (*argb, argb_stride) for lossless
  // compression, and YUV input (*y, *u, *v, etc.) for lossy compression
  // since these are the respective native colorspace for these formats.
  int use_argb;

  // YUV input (mostly used for input to lossy compression)
  DEDUP_WEBP_EncCSP colorspace;     // colorspace: should be YUV420 for now (=Y'CbCr).
  int width, height;         // dimensions (less or equal to WEBP_MAX_DIMENSION)
  uint8_t *y, *u, *v;        // pointers to luma/chroma planes.
  int y_stride, uv_stride;   // luma/chroma strides.
  uint8_t* a;                // pointer to the alpha plane
  int a_stride;              // stride of the alpha plane
  uint32_t pad1[2];          // padding for later use

  // ARGB input (mostly used for input to lossless compression)
  uint32_t* argb;            // Pointer to argb (32 bit) plane.
  int argb_stride;           // This is stride in pixels units, not bytes.
  uint32_t pad2[3];          // padding for later use

  //   OUTPUT
  ///////////////
  // Byte-emission hook, to store compressed bytes as they are ready.
  DEDUP_WEBP_WriterFunction writer;  // can be NULL
  void* custom_ptr;           // can be used by the writer.

  // map for extra information (only for lossy compression mode)
  int extra_info_type;    // 1: intra type, 2: segment, 3: quant
                          // 4: intra-16 prediction mode,
                          // 5: chroma prediction mode,
                          // 6: bit cost, 7: distortion
  uint8_t* extra_info;    // if not NULL, points to an array of size
                          // ((width + 15) / 16) * ((height + 15) / 16) that
                          // will be filled with a macroblock map, depending
                          // on extra_info_type.

  //   STATS AND REPORTS
  ///////////////////////////
  // Pointer to side statistics (updated only if not NULL)
  DEDUP_WEBP_AuxStats* stats;

  // Error code for the latest error encountered during encoding
  DEDUP_WEBP_EncodingError error_code;

  // If not NULL, report progress during encoding.
  DEDUP_WEBP_ProgressHook progress_hook;

  void* user_data;        // this field is free to be set to any value and
                          // used during callbacks (like progress-report e.g.).

  uint32_t pad3[3];       // padding for later use

  // Unused for now
  uint8_t *pad4, *pad5;
  uint32_t pad6[8];       // padding for later use

  // PRIVATE FIELDS
  ////////////////////
  void* memory_;          // row chunk of memory for yuva planes
  void* memory_argb_;     // and for argb too.
  void* pad7[2];          // padding for later use
};

// Internal, version-checked, entry point
WEBP_EXTERN(int) DEDUP_WEBP_PictureInitInternal(DEDUP_WEBP_Picture*, int);

// Should always be called, to initialize the structure. Returns false in case
// of version mismatch. DEDUP_WEBP_PictureInit() must have succeeded before using the
// 'picture' object.
// Note that, by default, use_argb is false and colorspace is WEBP_YUV420.
static WEBP_INLINE int DEDUP_WEBP_PictureInit(DEDUP_WEBP_Picture* picture) {
  return DEDUP_WEBP_PictureInitInternal(picture, WEBP_ENCODER_ABI_VERSION);
}

//------------------------------------------------------------------------------
// DEDUP_WEBP_Picture utils

// Convenience allocation / deallocation based on picture->width/height:
// Allocate y/u/v buffers as per colorspace/width/height specification.
// Note! This function will free the previous buffer if needed.
// Returns false in case of memory error.
WEBP_EXTERN(int) DEDUP_WEBP_PictureAlloc(DEDUP_WEBP_Picture* picture);

// Release the memory allocated by DEDUP_WEBP_PictureAlloc() or DEDUP_WEBP_PictureImport*().
// Note that this function does _not_ free the memory used by the 'picture'
// object itself.
// Besides memory (which is reclaimed) all other fields of 'picture' are
// preserved.
WEBP_EXTERN(void) DEDUP_WEBP_PictureFree(DEDUP_WEBP_Picture* picture);

// Copy the pixels of *src into *dst, using DEDUP_WEBP_PictureAlloc. Upon return, *dst
// will fully own the copied pixels (this is not a view). The 'dst' picture need
// not be initialized as its content is overwritten.
// Returns false in case of memory allocation error.
WEBP_EXTERN(int) DEDUP_WEBP_PictureCopy(const DEDUP_WEBP_Picture* src, DEDUP_WEBP_Picture* dst);

// Compute PSNR, SSIM or LSIM distortion metric between two pictures. Results
// are in dB, stored in result[] in the Y/U/V/Alpha/All or B/G/R/A/All order.
// Returns false in case of error (src and ref don't have same dimension, ...)
// Warning: this function is rather CPU-intensive.
WEBP_EXTERN(int) DEDUP_WEBP_PictureDistortion(
    const DEDUP_WEBP_Picture* src, const DEDUP_WEBP_Picture* ref,
    int metric_type,           // 0 = PSNR, 1 = SSIM, 2 = LSIM
    float result[5]);

// self-crops a picture to the rectangle defined by top/left/width/height.
// Returns false in case of memory allocation error, or if the rectangle is
// outside of the source picture.
// The rectangle for the view is defined by the top-left corner pixel
// coordinates (left, top) as well as its width and height. This rectangle
// must be fully be comprised inside the 'src' source picture. If the source
// picture uses the YUV420 colorspace, the top and left coordinates will be
// snapped to even values.
WEBP_EXTERN(int) DEDUP_WEBP_PictureCrop(DEDUP_WEBP_Picture* picture,
                                 int left, int top, int width, int height);

// Extracts a view from 'src' picture into 'dst'. The rectangle for the view
// is defined by the top-left corner pixel coordinates (left, top) as well
// as its width and height. This rectangle must be fully be comprised inside
// the 'src' source picture. If the source picture uses the YUV420 colorspace,
// the top and left coordinates will be snapped to even values.
// Picture 'src' must out-live 'dst' picture. Self-extraction of view is allowed
// ('src' equal to 'dst') as a mean of fast-cropping (but note that doing so,
// the original dimension will be lost). Picture 'dst' need not be initialized
// with DEDUP_WEBP_PictureInit() if it is different from 'src', since its content will
// be overwritten.
// Returns false in case of memory allocation error or invalid parameters.
WEBP_EXTERN(int) DEDUP_WEBP_PictureView(const DEDUP_WEBP_Picture* src,
                                 int left, int top, int width, int height,
                                 DEDUP_WEBP_Picture* dst);

// Returns true if the 'picture' is actually a view and therefore does
// not own the memory for pixels.
WEBP_EXTERN(int) DEDUP_WEBP_PictureIsView(const DEDUP_WEBP_Picture* picture);

// Rescale a picture to new dimension width x height.
// If either 'width' or 'height' (but not both) is 0 the corresponding
// dimension will be calculated preserving the aspect ratio.
// No gamma correction is applied.
// Returns false in case of error (invalid parameter or insufficient memory).
WEBP_EXTERN(int) DEDUP_WEBP_PictureRescale(DEDUP_WEBP_Picture* pic, int width, int height);

// Colorspace conversion function to import RGB samples.
// Previous buffer will be free'd, if any.
// *rgb buffer should have a size of at least height * rgb_stride.
// Returns false in case of memory error.
WEBP_EXTERN(int) DEDUP_WEBP_PictureImportRGB(
    DEDUP_WEBP_Picture* picture, const uint8_t* rgb, int rgb_stride);
// Same, but for RGBA buffer.
WEBP_EXTERN(int) DEDUP_WEBP_PictureImportRGBA(
    DEDUP_WEBP_Picture* picture, const uint8_t* rgba, int rgba_stride);
// Same, but for RGBA buffer. Imports the RGB direct from the 32-bit format
// input buffer ignoring the alpha channel. Avoids needing to copy the data
// to a temporary 24-bit RGB buffer to import the RGB only.
WEBP_EXTERN(int) DEDUP_WEBP_PictureImportRGBX(
    DEDUP_WEBP_Picture* picture, const uint8_t* rgbx, int rgbx_stride);

// Variants of the above, but taking BGR(A|X) input.
WEBP_EXTERN(int) DEDUP_WEBP_PictureImportBGR(
    DEDUP_WEBP_Picture* picture, const uint8_t* bgr, int bgr_stride);
WEBP_EXTERN(int) DEDUP_WEBP_PictureImportBGRA(
    DEDUP_WEBP_Picture* picture, const uint8_t* bgra, int bgra_stride);
WEBP_EXTERN(int) DEDUP_WEBP_PictureImportBGRX(
    DEDUP_WEBP_Picture* picture, const uint8_t* bgrx, int bgrx_stride);

// Converts picture->argb data to the YUV420A format. The 'colorspace'
// parameter is deprecated and should be equal to WEBP_YUV420.
// Upon return, picture->use_argb is set to false. The presence of real
// non-opaque transparent values is detected, and 'colorspace' will be
// adjusted accordingly. Note that this method is lossy.
// Returns false in case of error.
WEBP_EXTERN(int) DEDUP_WEBP_PictureARGBToYUVA(DEDUP_WEBP_Picture* picture,
                                       DEDUP_WEBP_EncCSP /*colorspace = WEBP_YUV420*/);

// Same as DEDUP_WEBP_PictureARGBToYUVA(), but the conversion is done using
// pseudo-random dithering with a strength 'dithering' between
// 0.0 (no dithering) and 1.0 (maximum dithering). This is useful
// for photographic picture.
WEBP_EXTERN(int) DEDUP_WEBP_PictureARGBToYUVADithered(
    DEDUP_WEBP_Picture* picture, DEDUP_WEBP_EncCSP colorspace, float dithering);

// Performs 'smart' RGBA->YUVA420 downsampling and colorspace conversion.
// Downsampling is handled with extra care in case of color clipping. This
// method is roughly 2x slower than DEDUP_WEBP_PictureARGBToYUVA() but produces better
// YUV representation.
// Returns false in case of error.
WEBP_EXTERN(int) DEDUP_WEBP_PictureSmartARGBToYUVA(DEDUP_WEBP_Picture* picture);

// Converts picture->yuv to picture->argb and sets picture->use_argb to true.
// The input format must be YUV_420 or YUV_420A.
// Note that the use of this method is discouraged if one has access to the
// raw ARGB samples, since using YUV420 is comparatively lossy. Also, the
// conversion from YUV420 to ARGB incurs a small loss too.
// Returns false in case of error.
WEBP_EXTERN(int) DEDUP_WEBP_PictureYUVAToARGB(DEDUP_WEBP_Picture* picture);

// Helper function: given a width x height plane of RGBA or YUV(A) samples
// clean-up the YUV or RGB samples under fully transparent area, to help
// compressibility (no guarantee, though).
WEBP_EXTERN(void) DEDUP_WEBP_CleanupTransparentArea(DEDUP_WEBP_Picture* picture);

// Scan the picture 'picture' for the presence of non fully opaque alpha values.
// Returns true in such case. Otherwise returns false (indicating that the
// alpha plane can be ignored altogether e.g.).
WEBP_EXTERN(int) DEDUP_WEBP_PictureHasTransparency(const DEDUP_WEBP_Picture* picture);

// Remove the transparency information (if present) by blending the color with
// the background color 'background_rgb' (specified as 24bit RGB triplet).
// After this call, all alpha values are reset to 0xff.
WEBP_EXTERN(void) DEDUP_WEBP_BlendAlpha(DEDUP_WEBP_Picture* pic, uint32_t background_rgb);

//------------------------------------------------------------------------------
// Main call

// Main encoding call, after config and picture have been initialized.
// 'picture' must be less than 16384x16384 in dimension (cf WEBP_MAX_DIMENSION),
// and the 'config' object must be a valid one.
// Returns false in case of error, true otherwise.
// In case of error, picture->error_code is updated accordingly.
// 'picture' can hold the source samples in both YUV(A) or ARGB input, depending
// on the value of 'picture->use_argb'. It is highly recommended to use
// the former for lossy encoding, and the latter for lossless encoding
// (when config.lossless is true). Automatic conversion from one format to
// another is provided but they both incur some loss.
WEBP_EXTERN(int) DEDUP_WEBP_Encode(const DEDUP_WEBP_Config* config, DEDUP_WEBP_Picture* picture);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_WEBP_ENCODE_H_ */
