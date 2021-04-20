/**
  @brief         Function for Analysis of sleep breathing quality.
  @param[in,out] 
  @param[in]     Sleep state
  @param[in]
  @param[in]

 */
void 

/**
  @brief         Function for calculating breath rate.
  @param[in,out] 
  @param[in]     rri      RR-interval list.
  @param[in]     rri_len  Length of RR-interval list.
  @param[in]     fs       Sample rate
  @return        Confidence indicating whether the breath rate can be used.
 */
#include <stdint.h>


#define MAX_INTERP_LEN (50) 
static float32_t k_interp_x[MAX_INTERP_LEN];
static float32_t k_interp_y[MAX_INTERP_LEN];
static float32_t k_interp_coef[MAX_INTERP_LEN];
static float32_t k_interp_buf[];
uint8_t CalcBreathRate(float32_t *rrilist, uint16_t rrilist_len, uint16_t fs) {
  uint16_t i;

  /* Resample rrlist to 1000Hz. */
  float32_t rrilist_x[50] = {0};  /*<< x values. */
  rrilist_x[rrilist_len - 1] = rrilist_len;
  rrilist_x_interval = rrilist_len / (rrilist_len - 1);
  for (i = 1; i < rrilist_len; ++i) {
    rrilist_x[i] = i * rrilist_x_interval;
  }

  arm_spline_instance_f32 S;
  arm_spline_init_f32(&S, ARM_SPLINE_PARABOLIC_RUNOUT, rrilist_x, rrilist, rrilist_len, coef, buf);
  arm_spline_f32(&S, outX, outY, scale);
  

  /* Filter breath. */

  /* FFT. */

  /* Find max. */
}