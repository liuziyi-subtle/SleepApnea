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


// static inline int _CmpFunc(const void* a, const void* b) {
//   return (*(float64_t*)a > *(float64_t*)b) ? 1 : -1;
// }

static float k_mem_pool[5120u];

uint8_t CalcBreathRate(float *rrs, uint16_t rrs_len, uint16_t fs) {
  uint16_t i;

  /**
    Allocate mem from k_mem_pool. 
    TODO: Replace this section with an alloc func. 
   */
  float *x = k_mem_pool;
  float *coef = x + rrs_len;  /*<< 3 * (rrs_len - 1) */
  float *buf = coef + 3 * (rrs_len - 1);  /*<< 2 * rrs_len - 1 */
  float *x_out = buf + 2 * rrs_len - 1;  /*<< max length is 32 * fs*/
  float *y_out = x_out + 32 * fs;

  /* Resample rrlist to 1000Hz. */
  x[0] = .0f;
  x[rrs_len - 1] = (float)rrs_len;
  float interval = (float)rrs_len / (rrs_len - 1);
  for (i = 1; i < rrs_len; ++i) {
    x[i] = i * interval;
  }

  arm_spline_instance_f32 S;
  arm_spline_init_f32(&S, ARM_SPLINE_PARABOLIC_RUNOUT, x, rrs, rrs_len, coef, buf);
  arm_spline_f32(&S, x_out, y_out, 2);

  // /* Filter breath. */
  // float *y_out_f = k_mem_pool + 5120;
  // biquad_cascade_df1_init_f32(&bpfQrsFilt, NUM_BPF_STAGE,
  //           bpfCoeffQrsFilt, bpfStateQrsFilt);
  // biquad_cascade_df1_f32(&bpfQrsFilt, y_out, y_out_f, 1);

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