// /**
//   @brief         Function for Analysis of sleep breathing quality.
//   @param[in,out] 
//   @param[in]     Sleep state
//   @param[in]
//   @param[in]

//  */


// /**
//   @brief         Function for calculating breath rate.
//   @param[in,out] 
//   @param[in]     rri      RR-interval list.
//   @param[in]     rri_len  Length of RR-interval list.
//   @param[in]     fs       Sample rate
//   @return        Confidence indicating whether the breath rate can be used.
//  */
#include <stdint.h>
#include <stdio.h>
#include "arm_spline_interp_f32.h"
#define MAX_INTERP_LEN (50)

#if DEBUG
#include "debug.h"
extern struct DEBUG_TRACKER PLOT;
#endif

extern void arm_spline_init_f32(
  arm_spline_instance_f32 * S,
  arm_spline_type type,
  const float32_t * x,
  const float32_t * y,
  uint32_t n,
  float32_t * coeffs,
  float32_t * tempBuffer);
extern void arm_spline_f32(
  arm_spline_instance_f32 * S, 
  const float32_t * xq,
  float32_t * pDst,
  uint32_t blockSize);


/* 2-scale butter bandpass. */
#include "ls_sleep_apnea_arm_rfft_fast_f64.h"

#include "biquad_filter.h"

#define BREATH_NUM_BPF_STAGE (2)
#define BREATH_NUM_BPF_COEFF (BREATH_NUM_BPF_STAGE * 5)
#define BREATH_NUM_BPF_STATE (BREATH_NUM_BPF_STAGE * 4)

biquad_casd_df1_inst_f32 k_bpf_filt;
static double k_bpf_state_filt[BREATH_NUM_BPF_STATE];
static double k_bpf_coeff_filt[BREATH_NUM_BPF_COEFF] = {
  1.0, 2.0, 1.0,
  1.997982103112522889176716489600948989391326904296875,
  -0.99798698842669619768486199973267503082752227783203125,
  0.000000887081773650622205434283350927726630175129685085266829,
  -0.000001774163547301244410868566701855453260350259370170533657,
  0.000000887081773650622205434283350927726630175129685085266829,
  1.999349012530860392189424601383507251739501953125,
  -0.99934952229378393173675476646167226135730743408203125
};  /*<< fs = 1kHz, bandpass = [0.1, 0.4]. */


// static inline int _CmpFunc(const void* a, const void* b) {
//   return (*(float64_t*)a > *(float64_t*)b) ? 1 : -1;
// }

static float k_mem_pool[100000000];

uint8_t CalcBreathRate(float *rrs, uint16_t rrs_len, uint16_t fs) {
  uint32_t i;

  /**
    Allocate mem from k_mem_pool. 
    TODO: Replace this section with an alloc func. 
   */
  float *x = k_mem_pool;
  uint32_t x_len = rrs_len;

  float *coef = x + x_len;  
  uint32_t coef_len = 3 * (rrs_len - 1);  /*<< b, c, d has length (n - 1). */
  
  float *buf = coef + coef_len;  /*<< 2 * rrs_len - 1 */
  uint32_t buf_len = 2 * rrs_len - 1;

  float sum_rrs = .0f;
  for (i = 0; i < rrs_len; ++i) {
    sum_rrs += rrs[i];
  }
  uint32_t block_size = (uint32_t)(sum_rrs - 1);

  float *x_interp = buf + buf_len;
  uint32_t x_interp_len = block_size;

  float *y_interp = x_interp + x_interp_len;
  uint32_t y_interp_len = block_size;
  
  float *y_interp_f = y_interp + y_interp_len;
  uint32_t y_interp_f_len = block_size;

  /* Resample rrlist to 1000Hz. */
  x[0] = .0f;
  x[rrs_len - 1] = (float)rrs_len;
  float interval = (float)rrs_len / (rrs_len - 1);
  for (i = 1; i < rrs_len; ++i) {
    x[i] = i * interval;
  }

  float step = rrs_len / (sum_rrs - 1);
  for (i = 0; i < sum_rrs - 1; ++i) {
    x_interp[i] = step * i;
  }

  arm_spline_instance_f32 S;
  arm_spline_init_f32(&S, ARM_SPLINE_PARABOLIC_RUNOUT, x, rrs, rrs_len, coef, buf);
  arm_spline_f32(&S, x_interp, y_interp, block_size);

  #if DEBUG
  for (i = 0; i < y_interp_len; ++i) {
    PLOT.x_interp[PLOT.x_interp_len++] = x_interp[i];
    PLOT.y_interp[PLOT.y_interp_len++] = y_interp[i];
  }
  #endif

  /* Filter breath. */
  biquad_cascade_df1_init_f32(&k_bpf_filt, BREATH_NUM_BPF_STAGE, 
                              k_bpf_coeff_filt, k_bpf_state_filt);
  biquad_cascade_df1_f32(&k_bpf_filt, y_interp, y_interp_f, y_interp_f_len);

  #if DEBUG
  for (i = 0; i < y_interp_f_len; ++i) {
    PLOT.y_interp_f[PLOT.y_interp_f_len++] = y_interp_f[i];
    // printf("%f, ", y_interp_f[i]);
  }
  // struct INTERMEDIATES X = GetPLOT();
  // printf("X.x_interp_len: %u", X.x_interp_len);
  // printf("X.y_interp_len: %u", X.y_interp_len);
  // printf("X.y_interp_f_len: %u", X.y_interp_f_len);
  #endif
  printf("=================================>\n");

  // /* FFT. */
  // float64_t* temp_data = k_mem_pool;
  // float64_t* results = k_mem_pool + data_length;

  // uint32_t i;
  // for (i = 0; i < data_length; ++i) {
  //   temp_data[i] = (data[i] - k_mean) * k_hann_window[i];
  // }

  // nonwear_arm_rfft_fast_instance_f64 S;
  // nonwear_arm_rfft_64_fast_init_f64(&S);
  // nonwear_arm_rfft_fast_f64(&S, temp_data, results, 0);

  // for (i = 0; i < data_length; i += 2) {
  //   results[i] = (pow(results[i], 2) + pow(results[i + 1], 2)) * NWC_HANN_SCALE;
  // }

  // /* Find max. */
  // qsort(data_sorted, data_length, sizeof(float64_t), _CmpFunc);

  return 0;
}