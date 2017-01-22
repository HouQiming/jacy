// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
//   DEDUP_WEBP_ encoder: internal header.
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef WEBP_ENC_DEDUP_vP8_ENCI_H_
#define WEBP_ENC_DEDUP_vP8_ENCI_H_

#include <string.h>     // for memcpy()
#include "../dec/common.h"
#include "../dsp/dsp.h"
#include "../utils/bit_writer.h"
#include "../utils/thread.h"
#include "../utils/utils.h"
#include "../webp/encode.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Various defines and enums

// version numbers
#define ENC_MAJ_VERSION 0
#define ENC_MIN_VERSION 5
#define ENC_REV_VERSION 1

enum { MAX_LF_LEVELS = 64,       // Maximum loop filter level
       MAX_VARIABLE_LEVEL = 67,  // last (inclusive) level with variable cost
       MAX_LEVEL = 2047          // max level (note: max codable is 2047 + 67)
     };

typedef enum {   // Rate-distortion optimization levels
  RD_OPT_NONE        = 0,  // no rd-opt
  RD_OPT_BASIC       = 1,  // basic scoring (no trellis)
  RD_OPT_TRELLIS     = 2,  // perform trellis-quant on the final decision only
  RD_OPT_TRELLIS_ALL = 3   // trellis-quant for every scoring (much slower)
} DEDUP_vP8_RDLevel;

// YUV-cache parameters. Cache is 32-bytes wide (= one cacheline).
// The original or reconstructed samples can be accessed using DEDUP_vP8_Scan[].
// The predicted blocks can be accessed using offsets to yuv_p_ and
// the arrays DEDUP_vP8_*ModeOffsets[].
// * YUV Samples area (yuv_in_/yuv_out_/yuv_out2_)
//   (see DEDUP_vP8_Scan[] for accessing the blocks, along with
//   Y_OFF_ENC/U_OFF_ENC/V_OFF_ENC):
//             +----+----+
//  Y_OFF_ENC  |YYYY|UUVV|
//  U_OFF_ENC  |YYYY|UUVV|
//  V_OFF_ENC  |YYYY|....| <- 25% wasted U/V area
//             |YYYY|....|
//             +----+----+
// * Prediction area ('yuv_p_', size = PRED_SIZE_ENC)
//   Intra16 predictions (16x16 block each, two per row):
//         |I16DC16|I16TM16|
//         |I16VE16|I16HE16|
//   Chroma U/V predictions (16x8 block each, two per row):
//         |C8DC8|C8TM8|
//         |C8VE8|C8HE8|
//   Intra 4x4 predictions (4x4 block each)
//         |I4DC4 I4TM4 I4VE4 I4HE4|I4RD4 I4VR4 I4LD4 I4VL4|
//         |I4HD4 I4HU4 I4TMP .....|.......................| <- ~31% wasted
#define YUV_SIZE_ENC (BPS * 16)
#define PRED_SIZE_ENC (32 * BPS + 16 * BPS + 8 * BPS)   // I16+Chroma+I4 preds
#define Y_OFF_ENC    (0)
#define U_OFF_ENC    (16)
#define V_OFF_ENC    (16 + 8)

extern const int DEDUP_vP8_Scan[16];           // in quant.c
extern const int DEDUP_vP8_UVModeOffsets[4];   // in analyze.c
extern const int DEDUP_vP8_I16ModeOffsets[4];
extern const int DEDUP_vP8_I4ModeOffsets[NUM_BMODES];

// Layout of prediction blocks
// intra 16x16
#define I16DC16 (0 * 16 * BPS)
#define I16TM16 (I16DC16 + 16)
#define I16VE16 (1 * 16 * BPS)
#define I16HE16 (I16VE16 + 16)
// chroma 8x8, two U/V blocks side by side (hence: 16x8 each)
#define C8DC8 (2 * 16 * BPS)
#define C8TM8 (C8DC8 + 1 * 16)
#define C8VE8 (2 * 16 * BPS + 8 * BPS)
#define C8HE8 (C8VE8 + 1 * 16)
// intra 4x4
#define I4DC4 (3 * 16 * BPS +  0)
#define I4TM4 (I4DC4 +  4)
#define I4VE4 (I4DC4 +  8)
#define I4HE4 (I4DC4 + 12)
#define I4RD4 (I4DC4 + 16)
#define I4VR4 (I4DC4 + 20)
#define I4LD4 (I4DC4 + 24)
#define I4VL4 (I4DC4 + 28)
#define I4HD4 (3 * 16 * BPS + 4 * BPS)
#define I4HU4 (I4HD4 + 4)
#define I4TMP (I4HD4 + 8)

typedef int64_t score_t;     // type used for scores, rate, distortion
// Note that MAX_COST is not the maximum allowed by sizeof(score_t),
// in order to allow overflowing computations.
#define MAX_COST ((score_t)0x7fffffffffffffLL)

#define QFIX 17
#define BIAS(b)  ((b) << (QFIX - 8))
// Fun fact: this is the _only_ line where we're actually being lossy and
// discarding bits.
static WEBP_INLINE int QUANTDIV(uint32_t n, uint32_t iQ, uint32_t B) {
  return (int)((n * iQ + B) >> QFIX);
}

// Uncomment the following to remove token-buffer code:
// #define DISABLE_TOKEN_BUFFER

//------------------------------------------------------------------------------
// Headers

typedef uint32_t proba_t;   // 16b + 16b
typedef uint8_t ProbaArray[NUM_CTX][NUM_PROBAS];
typedef proba_t StatsArray[NUM_CTX][NUM_PROBAS];
typedef uint16_t CostArray[NUM_CTX][MAX_VARIABLE_LEVEL + 1];
typedef const uint16_t* (*CostArrayPtr)[NUM_CTX];   // for easy casting
typedef const uint16_t* CostArrayMap[16][NUM_CTX];
typedef double LFStats[NUM_MB_SEGMENTS][MAX_LF_LEVELS];  // filter stats

typedef struct DEDUP_vP8_Encoder DEDUP_vP8_Encoder;

// segment features
typedef struct {
  int num_segments_;      // Actual number of segments. 1 segment only = unused.
  int update_map_;        // whether to update the segment map or not.
                          // must be 0 if there's only 1 segment.
  int size_;              // bit-cost for transmitting the segment map
} DEDUP_vP8_EncSegmentHeader;

// Struct collecting all frame-persistent probabilities.
typedef struct {
  uint8_t segments_[3];     // probabilities for segment tree
  uint8_t skip_proba_;      // final probability of being skipped.
  ProbaArray coeffs_[NUM_TYPES][NUM_BANDS];      // 1056 bytes
  StatsArray stats_[NUM_TYPES][NUM_BANDS];       // 4224 bytes
  CostArray level_cost_[NUM_TYPES][NUM_BANDS];   // 13056 bytes
  CostArrayMap remapped_costs_[NUM_TYPES];       // 1536 bytes
  int dirty_;               // if true, need to call DEDUP_vP8_CalculateLevelCosts()
  int use_skip_proba_;      // Note: we always use skip_proba for now.
  int nb_skip_;             // number of skipped blocks
} DEDUP_vP8_EncProba;

// Filter parameters. Not actually used in the code (we don't perform
// the in-loop filtering), but filled from user's config
typedef struct {
  int simple_;             // filtering type: 0=complex, 1=simple
  int level_;              // base filter level [0..63]
  int sharpness_;          // [0..7]
  int i4x4_lf_delta_;      // delta filter level for i4x4 relative to i16x16
} DEDUP_vP8_EncFilterHeader;

//------------------------------------------------------------------------------
// Informations about the macroblocks.

typedef struct {
  // block type
  unsigned int type_:2;     // 0=i4x4, 1=i16x16
  unsigned int uv_mode_:2;
  unsigned int skip_:1;
  unsigned int segment_:2;
  uint8_t alpha_;      // quantization-susceptibility
} DEDUP_vP8_MBInfo;

typedef struct DEDUP_vP8_Matrix {
  uint16_t q_[16];        // quantizer steps
  uint16_t iq_[16];       // reciprocals, fixed point.
  uint32_t bias_[16];     // rounding bias
  uint32_t zthresh_[16];  // value below which a coefficient is zeroed
  uint16_t sharpen_[16];  // frequency boosters for slight sharpening
} DEDUP_vP8_Matrix;

typedef struct {
  DEDUP_vP8_Matrix y1_, y2_, uv_;  // quantization matrices
  int alpha_;      // quant-susceptibility, range [-127,127]. Zero is neutral.
                   // Lower values indicate a lower risk of blurriness.
  int beta_;       // filter-susceptibility, range [0,255].
  int quant_;      // final segment quantizer.
  int fstrength_;  // final in-loop filtering strength
  int max_edge_;   // max edge delta (for filtering strength)
  int min_disto_;  // minimum distortion required to trigger filtering record
  // reactivities
  int lambda_i16_, lambda_i4_, lambda_uv_;
  int lambda_mode_, lambda_trellis_, tlambda_;
  int lambda_trellis_i16_, lambda_trellis_i4_, lambda_trellis_uv_;

  // lambda values for distortion-based evaluation
  score_t i4_penalty_;   // penalty for using Intra4
} DEDUP_vP8_SegmentInfo;

// Handy transient struct to accumulate score and info during RD-optimization
// and mode evaluation.
typedef struct {
  score_t D, SD;              // Distortion, spectral distortion
  score_t H, R, score;        // header bits, rate, score.
  int16_t y_dc_levels[16];    // Quantized levels for luma-DC, luma-AC, chroma.
  int16_t y_ac_levels[16][16];
  int16_t uv_levels[4 + 4][16];
  int mode_i16;               // mode number for intra16 prediction
  uint8_t modes_i4[16];       // mode numbers for intra4 predictions
  int mode_uv;                // mode number of chroma prediction
  uint32_t nz;                // non-zero blocks
} DEDUP_vP8_ModeScore;

// Iterator structure to iterate through macroblocks, pointing to the
// right neighbouring data (samples, predictions, contexts, ...)
typedef struct {
  int x_, y_;                      // current macroblock
  int y_stride_, uv_stride_;       // respective strides
  uint8_t*      yuv_in_;           // input samples
  uint8_t*      yuv_out_;          // output samples
  uint8_t*      yuv_out2_;         // secondary buffer swapped with yuv_out_.
  uint8_t*      yuv_p_;            // scratch buffer for prediction
  DEDUP_vP8_Encoder*   enc_;              // back-pointer
  DEDUP_vP8_MBInfo*    mb_;               // current macroblock
  DEDUP_vP8_BitWriter* bw_;               // current bit-writer
  uint8_t*      preds_;            // intra mode predictors (4x4 blocks)
  uint32_t*     nz_;               // non-zero pattern
  uint8_t       i4_boundary_[37];  // 32+5 boundary samples needed by intra4x4
  uint8_t*      i4_top_;           // pointer to the current top boundary sample
  int           i4_;               // current intra4x4 mode being tested
  int           top_nz_[9];        // top-non-zero context.
  int           left_nz_[9];       // left-non-zero. left_nz[8] is independent.
  uint64_t      bit_count_[4][3];  // bit counters for coded levels.
  uint64_t      luma_bits_;        // macroblock bit-cost for luma
  uint64_t      uv_bits_;          // macroblock bit-cost for chroma
  LFStats*      lf_stats_;         // filter stats (borrowed from enc_)
  int           do_trellis_;       // if true, perform extra level optimisation
  int           count_down_;       // number of mb still to be processed
  int           count_down0_;      // starting counter value (for progress)
  int           percent0_;         // saved initial progress percent

  uint8_t* y_left_;    // left luma samples (addressable from index -1 to 15).
  uint8_t* u_left_;    // left u samples (addressable from index -1 to 7)
  uint8_t* v_left_;    // left v samples (addressable from index -1 to 7)

  uint8_t* y_top_;     // top luma samples at position 'x_'
  uint8_t* uv_top_;    // top u/v samples at position 'x_', packed as 16 bytes

  // memory for storing y/u/v_left_
  uint8_t yuv_left_mem_[17 + 16 + 16 + 8 + WEBP_ALIGN_CST];
  // memory for yuv_*
  uint8_t yuv_mem_[3 * YUV_SIZE_ENC + PRED_SIZE_ENC + WEBP_ALIGN_CST];
} DEDUP_vP8_EncIterator;

  // in iterator.c
// must be called first
void DEDUP_vP8_IteratorInit(DEDUP_vP8_Encoder* const enc, DEDUP_vP8_EncIterator* const it);
// restart a scan
void DEDUP_vP8_IteratorReset(DEDUP_vP8_EncIterator* const it);
// reset iterator position to row 'y'
void DEDUP_vP8_IteratorSetRow(DEDUP_vP8_EncIterator* const it, int y);
// set count down (=number of iterations to go)
void DEDUP_vP8_IteratorSetCountDown(DEDUP_vP8_EncIterator* const it, int count_down);
// return true if iteration is finished
int DEDUP_vP8_IteratorIsDone(const DEDUP_vP8_EncIterator* const it);
// Import uncompressed samples from source.
// If tmp_32 is not NULL, import boundary samples too.
// tmp_32 is a 32-bytes scratch buffer that must be aligned in memory.
void DEDUP_vP8_IteratorImport(DEDUP_vP8_EncIterator* const it, uint8_t* tmp_32);
// export decimated samples
void DEDUP_vP8_IteratorExport(const DEDUP_vP8_EncIterator* const it);
// go to next macroblock. Returns false if not finished.
int DEDUP_vP8_IteratorNext(DEDUP_vP8_EncIterator* const it);
// save the yuv_out_ boundary values to top_/left_ arrays for next iterations.
void DEDUP_vP8_IteratorSaveBoundary(DEDUP_vP8_EncIterator* const it);
// Report progression based on macroblock rows. Return 0 for user-abort request.
int DEDUP_vP8_IteratorProgress(const DEDUP_vP8_EncIterator* const it,
                        int final_delta_percent);
// Intra4x4 iterations
void DEDUP_vP8_IteratorStartI4(DEDUP_vP8_EncIterator* const it);
// returns true if not done.
int DEDUP_vP8_IteratorRotateI4(DEDUP_vP8_EncIterator* const it,
                        const uint8_t* const yuv_out);

// Non-zero context setup/teardown
void DEDUP_vP8_IteratorNzToBytes(DEDUP_vP8_EncIterator* const it);
void DEDUP_vP8_IteratorBytesToNz(DEDUP_vP8_EncIterator* const it);

// Helper functions to set mode properties
void DEDUP_vP8_SetIntra16Mode(const DEDUP_vP8_EncIterator* const it, int mode);
void DEDUP_vP8_SetIntra4Mode(const DEDUP_vP8_EncIterator* const it, const uint8_t* modes);
void DEDUP_vP8_SetIntraUVMode(const DEDUP_vP8_EncIterator* const it, int mode);
void DEDUP_vP8_SetSkip(const DEDUP_vP8_EncIterator* const it, int skip);
void DEDUP_vP8_SetSegment(const DEDUP_vP8_EncIterator* const it, int segment);

//------------------------------------------------------------------------------
// Paginated token buffer

typedef struct DEDUP_vP8_Tokens DEDUP_vP8_Tokens;  // struct details in token.c

typedef struct {
#if !defined(DISABLE_TOKEN_BUFFER)
  DEDUP_vP8_Tokens* pages_;        // first page
  DEDUP_vP8_Tokens** last_page_;   // last page
  uint16_t* tokens_;        // set to (*last_page_)->tokens_
  int left_;                // how many free tokens left before the page is full
  int page_size_;           // number of tokens per page
#endif
  int error_;         // true in case of malloc error
} DEDUP_vP8_TBuffer;

// initialize an empty buffer
void DEDUP_vP8_TBufferInit(DEDUP_vP8_TBuffer* const b, int page_size);
void DEDUP_vP8_TBufferClear(DEDUP_vP8_TBuffer* const b);   // de-allocate pages memory

#if !defined(DISABLE_TOKEN_BUFFER)

// Finalizes bitstream when probabilities are known.
// Deletes the allocated token memory if final_pass is true.
int DEDUP_vP8_EmitTokens(DEDUP_vP8_TBuffer* const b, DEDUP_vP8_BitWriter* const bw,
                  const uint8_t* const probas, int final_pass);

// record the coding of coefficients without knowing the probabilities yet
int DEDUP_vP8_RecordCoeffTokens(int ctx, const struct DEDUP_vP8_Residual* const res,
                         DEDUP_vP8_TBuffer* const tokens);

// Estimate the final coded size given a set of 'probas'.
size_t DEDUP_vP8_EstimateTokenSize(DEDUP_vP8_TBuffer* const b, const uint8_t* const probas);

// unused for now
void DEDUP_vP8_TokenToStats(const DEDUP_vP8_TBuffer* const b, proba_t* const stats);

#endif  // !DISABLE_TOKEN_BUFFER

//------------------------------------------------------------------------------
// DEDUP_vP8_Encoder

struct DEDUP_vP8_Encoder {
  const DEDUP_WEBP_Config* config_;    // user configuration and parameters
  DEDUP_WEBP_Picture* pic_;            // input / output picture

  // headers
  DEDUP_vP8_EncFilterHeader   filter_hdr_;     // filtering information
  DEDUP_vP8_EncSegmentHeader  segment_hdr_;    // segment information

  int profile_;                      // DEDUP_vP8_'s profile, deduced from Config.

  // dimension, in macroblock units.
  int mb_w_, mb_h_;
  int preds_w_;   // stride of the *preds_ prediction plane (=4*mb_w + 1)

  // number of partitions (1, 2, 4 or 8 = MAX_NUM_PARTITIONS)
  int num_parts_;

  // per-partition boolean decoders.
  DEDUP_vP8_BitWriter bw_;                         // part0
  DEDUP_vP8_BitWriter parts_[MAX_NUM_PARTITIONS];  // token partitions
  DEDUP_vP8_TBuffer tokens_;                       // token buffer

  int percent_;                             // for progress

  // transparency blob
  int has_alpha_;
  uint8_t* alpha_data_;       // non-NULL if transparency is present
  uint32_t alpha_data_size_;
  DEDUP_WEBP_Worker alpha_worker_;

  // quantization info (one set of DC/AC dequant factor per segment)
  DEDUP_vP8_SegmentInfo dqm_[NUM_MB_SEGMENTS];
  int base_quant_;                 // nominal quantizer value. Only used
                                   // for relative coding of segments' quant.
  int alpha_;                      // global susceptibility (<=> complexity)
  int uv_alpha_;                   // U/V quantization susceptibility
  // global offset of quantizers, shared by all segments
  int dq_y1_dc_;
  int dq_y2_dc_, dq_y2_ac_;
  int dq_uv_dc_, dq_uv_ac_;

  // probabilities and statistics
  DEDUP_vP8_EncProba proba_;
  uint64_t    sse_[4];      // sum of Y/U/V/A squared errors for all macroblocks
  uint64_t    sse_count_;   // pixel count for the sse_[] stats
  int         coded_size_;
  int         residual_bytes_[3][4];
  int         block_count_[3];

  // quality/speed settings
  int method_;               // 0=fastest, 6=best/slowest.
  DEDUP_vP8_RDLevel rd_opt_level_;  // Deduced from method_.
  int max_i4_header_bits_;   // partition #0 safeness factor
  int mb_header_limit_;      // rough limit for header bits per MB
  int thread_level_;         // derived from config->thread_level
  int do_search_;            // derived from config->target_XXX
  int use_tokens_;           // if true, use token buffer

  // Memory
  DEDUP_vP8_MBInfo* mb_info_;   // contextual macroblock infos (mb_w_ + 1)
  uint8_t*   preds_;     // predictions modes: (4*mb_w+1) * (4*mb_h+1)
  uint32_t*  nz_;        // non-zero bit context: mb_w+1
  uint8_t*   y_top_;     // top luma samples.
  uint8_t*   uv_top_;    // top u/v samples.
                         // U and V are packed into 16 bytes (8 U + 8 V)
  LFStats*   lf_stats_;  // autofilter stats (if NULL, autofilter is off)
};

//------------------------------------------------------------------------------
// internal functions. Not public.

  // in tree.c
extern const uint8_t DEDUP_vP8_CoeffsProba0[NUM_TYPES][NUM_BANDS][NUM_CTX][NUM_PROBAS];
extern const uint8_t
    DEDUP_vP8_CoeffsUpdateProba[NUM_TYPES][NUM_BANDS][NUM_CTX][NUM_PROBAS];
// Reset the token probabilities to their initial (default) values
void DEDUP_vP8_DefaultProbas(DEDUP_vP8_Encoder* const enc);
// Write the token probabilities
void DEDUP_vP8_WriteProbas(DEDUP_vP8_BitWriter* const bw, const DEDUP_vP8_EncProba* const probas);
// Writes the partition #0 modes (that is: all intra modes)
void DEDUP_vP8_CodeIntraModes(DEDUP_vP8_Encoder* const enc);

  // in syntax.c
// Generates the final bitstream by coding the partition0 and headers,
// and appending an assembly of all the pre-coded token partitions.
// Return true if everything is ok.
int DEDUP_vP8_EncWrite(DEDUP_vP8_Encoder* const enc);
// Release memory allocated for bit-writing in DEDUP_vP8_EncLoop & seq.
void DEDUP_vP8_EncFreeBitWriters(DEDUP_vP8_Encoder* const enc);

  // in frame.c
extern const uint8_t DEDUP_vP8_Cat3[];
extern const uint8_t DEDUP_vP8_Cat4[];
extern const uint8_t DEDUP_vP8_Cat5[];
extern const uint8_t DEDUP_vP8_Cat6[];

// Form all the four Intra16x16 predictions in the yuv_p_ cache
void DEDUP_vP8_MakeLuma16Preds(const DEDUP_vP8_EncIterator* const it);
// Form all the four Chroma8x8 predictions in the yuv_p_ cache
void DEDUP_vP8_MakeChroma8Preds(const DEDUP_vP8_EncIterator* const it);
// Form all the ten Intra4x4 predictions in the yuv_p_ cache
// for the 4x4 block it->i4_
void DEDUP_vP8_MakeIntra4Preds(const DEDUP_vP8_EncIterator* const it);
// Rate calculation
int DEDUP_vP8_GetCostLuma16(DEDUP_vP8_EncIterator* const it, const DEDUP_vP8_ModeScore* const rd);
int DEDUP_vP8_GetCostLuma4(DEDUP_vP8_EncIterator* const it, const int16_t levels[16]);
int DEDUP_vP8_GetCostUV(DEDUP_vP8_EncIterator* const it, const DEDUP_vP8_ModeScore* const rd);
// Main coding calls
int DEDUP_vP8_EncLoop(DEDUP_vP8_Encoder* const enc);
int DEDUP_vP8_EncTokenLoop(DEDUP_vP8_Encoder* const enc);

  // in webpenc.c
// Assign an error code to a picture. Return false for convenience.
int DEDUP_WEBP_EncodingSetError(const DEDUP_WEBP_Picture* const pic, DEDUP_WEBP_EncodingError error);
int DEDUP_WEBP_ReportProgress(const DEDUP_WEBP_Picture* const pic,
                       int percent, int* const percent_store);

  // in analysis.c
// Main analysis loop. Decides the segmentations and complexity.
// Assigns a first guess for Intra16 and uvmode_ prediction modes.
int DEDUP_vP8_EncAnalyze(DEDUP_vP8_Encoder* const enc);

  // in quant.c
// Sets up segment's quantization values, base_quant_ and filter strengths.
void DEDUP_vP8_SetSegmentParams(DEDUP_vP8_Encoder* const enc, float quality);
// Pick best modes and fills the levels. Returns true if skipped.
int DEDUP_vP8_Decimate(DEDUP_vP8_EncIterator* const it, DEDUP_vP8_ModeScore* const rd,
                DEDUP_vP8_RDLevel rd_opt);

  // in alpha.c
void DEDUP_vP8_EncInitAlpha(DEDUP_vP8_Encoder* const enc);    // initialize alpha compression
int DEDUP_vP8_EncStartAlpha(DEDUP_vP8_Encoder* const enc);    // start alpha coding process
int DEDUP_vP8_EncFinishAlpha(DEDUP_vP8_Encoder* const enc);   // finalize compressed data
int DEDUP_vP8_EncDeleteAlpha(DEDUP_vP8_Encoder* const enc);   // delete compressed data

// autofilter
void DEDUP_vP8_InitFilter(DEDUP_vP8_EncIterator* const it);
void DEDUP_vP8_StoreFilterStats(DEDUP_vP8_EncIterator* const it);
void DEDUP_vP8_AdjustFilterStrength(DEDUP_vP8_EncIterator* const it);

// returns the approximate filtering strength needed to smooth a edge
// step of 'delta', given a sharpness parameter 'sharpness'.
int DEDUP_vP8_FilterStrengthFromDelta(int sharpness, int delta);

  // misc utils for picture_*.c:

// Remove reference to the ARGB/YUVA buffer (doesn't free anything).
void DEDUP_WEBP_PictureResetBuffers(DEDUP_WEBP_Picture* const picture);

// Allocates ARGB buffer of given dimension (previous one is always free'd).
// Preserves the YUV(A) buffer. Returns false in case of error (invalid param,
// out-of-memory).
int DEDUP_WEBP_PictureAllocARGB(DEDUP_WEBP_Picture* const picture, int width, int height);

// Allocates YUVA buffer of given dimension (previous one is always free'd).
// Uses picture->csp to determine whether an alpha buffer is needed.
// Preserves the ARGB buffer.
// Returns false in case of error (invalid param, out-of-memory).
int DEDUP_WEBP_PictureAllocYUVA(DEDUP_WEBP_Picture* const picture, int width, int height);

// Clean-up the RGB samples under fully transparent area, to help lossless
// compressibility (no guarantee, though). Assumes that pic->use_argb is true.
void DEDUP_WEBP_CleanupTransparentAreaLossless(DEDUP_WEBP_Picture* const pic);

  // in near_lossless.c
// Near lossless preprocessing in RGB color-space.
int DEDUP_vP8_ApplyNearLossless(int xsize, int ysize, uint32_t* argb, int quality);
// Near lossless adjustment for predictors.
void DEDUP_vP8_ApplyNearLosslessPredict(int xsize, int ysize, int pred_bits,
                                 const uint32_t* argb_orig,
                                 uint32_t* argb, uint32_t* argb_scratch,
                                 const uint32_t* const transform_data,
                                 int quality, int subtract_green);
//------------------------------------------------------------------------------

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_ENC_DEDUP_vP8_ENCI_H_ */
