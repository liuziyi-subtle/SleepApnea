#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "breath_interp.h"
#include "breath_biquad_filter.h"
#include "breath_fft.h"
#include "breath_funcs.h"
#include "breath.h"
#include <math.h>
#include "breath_find_peaks.h"

#if DEBUG
#include "debug.h"
#endif

#define FS                        (25)
#define BREATH_NUM_BPF_STAGE      (2)
#define BREATH_MAX_RRS_LEN        (128)
#define BREATH_PPG_BUF_LEN        (FS)      /*<< 1s buffer. */
#define BREATH_MAX_PV_LEN         (256)     /*<< (32 / 60) * 240BPM */
#define BREATH_MAX_BREATH_LEN     (200)
#define BREATH_CALL_DURATION      (32 * FS)
#define BREATH_RATE_NONE          (-1)
#define BREATH_SCORE_NONE         (-1)
#define BREATH_MAX_PEAKS_LEN      (256)     /*<< 32 * (240BPM / 60) */

static float32_t _mem_pool[1024];  // 4096 Bytes
static float64_t _mem_pool_f64[1024];  // 8192 Bytes

/* fs = 5Hz, 2-scale butter bandpass = [0.1, 0.4]. */
static breath_arm_biquad_casd_df1_inst_f32 _filter;
static float32_t _state[BREATH_NUM_BPF_STAGE * 4];
static float32_t _coeff[BREATH_NUM_BPF_STAGE * 5] =
{
  1.0f,
  2.0f,
  1.0f,
  1.91505038738250732421875f,
  -0.92256987094879150390625f,
  0.001348711899481713771820068359375f,
  -0.00269742379896342754364013671875f,
  0.001348711899481713771820068359375f,
  1.9734938144683837890625f,
  -0.974299132823944091796875f
};

/* fs = 25Hz, 2-scale butter highpass = 0.15. */
static breath_arm_biquad_casd_df1_inst_f32 _filter_ppg;
static float32_t _state_ppg[BREATH_NUM_BPF_STAGE * 4];
static float32_t _coeff_ppg[BREATH_NUM_BPF_STAGE * 5] =
{
  1.0f, -2.0f, 1.0f, 1.93132781982421875f, -0.932701051235198974609375f,
  0.951933920383453369140625f, -1.90386784076690673828125f,
  0.951933920383453369140625f, 1.97016251087188720703125f,
  -0.9715633392333984375f
};

static float32_t _ppg_buf[BREATH_PPG_BUF_LEN];  // 100 Bytes.
static uint32_t _ppg_nfill;
static uint32_t _ppg_counter;

// static uint32_t _num_peaks;
static float32_t _rr_buf[BREATH_MAX_RRS_LEN];  // 512 Bytes
static uint32_t _rr_buf_len;
static uint32_t _total_rr_len;
static uint32_t _last_total_rr_len;

// #define BREATH_MAX_BREATH_LEN (1440)
static uint8_t _breath_rate_buf[BREATH_MAX_BREATH_LEN];  // 200 Bytes
static uint32_t _breath_rate_buf_len;

static float32_t _peaks[BREATH_MAX_PEAKS_LEN];  // 512 Bytes
static int32_t _peak_indices[BREATH_MAX_PEAKS_LEN];  // 512 Bytes
static int32_t _peaks_len;
static uint32_t _last_ppg_counter;
static int32_t _left_bases[BREATH_MAX_PEAKS_LEN];  // 512 Bytes
static int32_t _right_bases[BREATH_MAX_PEAKS_LEN];  // 512 Bytes

typedef struct _value2index_temp
{
  float32_t value;
  int32_t index;
} value2index_temp;

static value2index_temp v2i_t[256];


static uint8_t _CalcBreathRate(float32_t *rrs, uint32_t rrs_len, uint16_t fs)
{
  uint32_t i;

  /* Check rrs_len. */
  if (rrs_len == 0 || rrs_len > BREATH_MAX_RRS_LEN)
  {
    return 0u;
  }

  /**
   * Allocate mem from _mem_pool. TODO: Replace this section with an alloc func. 
   */
  float32_t *x = _mem_pool;
  uint32_t x_len = rrs_len;  /*<< max = 128. */

  float32_t *coef = x + x_len;
  uint32_t coef_len = 3 * (rrs_len - 1);  /*<< b, c, d has length (n - 1). */
  
  float32_t *buf = coef + coef_len;  /*<< 2 * rrs_len - 1 */
  uint32_t buf_len = 2 * rrs_len - 1;

  uint32_t block_size = 0u;  /*<< length of the interpolated.*/
  for (i = 0; i < rrs_len; ++i) {
    block_size += (uint32_t)rrs[i];
  }

  float32_t *x_interp = buf + buf_len;
  uint32_t x_interp_len = block_size;

  float32_t *y_interp = x_interp + x_interp_len;
  uint32_t y_interp_len = block_size;
  
  float32_t *y_interp_f = y_interp + y_interp_len;
  uint32_t y_interp_f_len = block_size;

  /* Resample rrlist to 25Hz. */
  float32_t interval = (float32_t)rrs_len / (rrs_len - 1);
  for (i = 0; i < rrs_len; ++i) {
    x[i] = i * interval;
  }
  x[rrs_len - 1] = (float32_t)rrs_len;

  interval = (float32_t)rrs_len / (block_size - 1);
  for (i = 0; i < block_size; ++i) {
    x_interp[i] = interval * i;
  }
  x_interp[block_size - 1] = (float32_t)rrs_len;

  arm_spline_instance_f32 S;
  arm_spline_init_f32(&S, ARM_SPLINE_PARABOLIC_RUNOUT, x, rrs, rrs_len, coef, buf);
  arm_spline_f32(&S, x_interp, y_interp, block_size);

#if DEBUG
  for (i = 0; i < y_interp_len; ++i) {
    PLOT.x_interp[PLOT.x_interp_len++] = x_interp[i];
    PLOT.y_interp[PLOT.y_interp_len++] = y_interp[i];
  }
#endif

  /* Filter breath. TODO: Place-in. */
  breath_arm_biquad_cascade_df1_init_f32(&_filter, BREATH_NUM_BPF_STAGE,
                                  _coeff, _state);
  breath_arm_biquad_cascade_df1_f32(&_filter, y_interp, y_interp_f, y_interp_f_len);

#if DEBUG
  for (i = 0; i < y_interp_f_len; ++i) {
    PLOT.y_interp_f[PLOT.y_interp_f_len++] = y_interp_f[i];
    // printf("y_interp_f[%u]: %f\n", i, y_interp_f[i]);
  }
#endif

  /* Welch. */
  float64_t *y_interp_f_f64 = _mem_pool_f64;  /*<< Transfer to float64_t. */
  float64_t *welch_density = y_interp_f_f64 + y_interp_f_len;
  for (i = 0; i < y_interp_f_len; ++i) {
    y_interp_f_f64[i] = (float64_t)y_interp_f[i];
  }
  uint32_t welch_density_len = SpktWelchDensity(y_interp_f_f64, y_interp_f_len,
                                                welch_density);

  /* Find max. */
  float64_t maxval;
  uint32_t maxval_index;
  FindMax(welch_density, welch_density_len, &maxval, &maxval_index);
  uint8_t breath_rate = (uint8_t)(maxval_index * (fs / 64.0f));
  printf("maxval_index: %u, maxval: %f, breath_rate: %f\n", maxval_index, maxval, breath_rate);

#if DEBUG
  for (i = 0; i < welch_density_len; ++i) {
    PLOT.welch_density[PLOT.welch_density_len++] = welch_density[i];
  }
#endif  

  return 0;
}

void _DetectPeaks(float32_t ppg)
{
  uint32_t i, j;

  /* Point-wise filtering. */
  float32_t ppg_f;
  breath_arm_biquad_cascade_df1_f32(&_filter_ppg, &ppg, &ppg_f, 1);
  _ppg_buf[_ppg_nfill] = ppg_f;
  _ppg_nfill++;
  _ppg_counter++;

#if DEBUG
  PLOT.ppg[PLOT.ppg_len++] = ppg;
  PLOT.ppg_f[PLOT.ppg_f_len++] = ppg_f;
#endif

  if (_ppg_nfill < BREATH_PPG_BUF_LEN)
  {
    return;
  }

  for (i = 0; i < BREATH_PPG_BUF_LEN; i++)
  {
    // printf("_ppg_buf[i]: %f, _ppg_counter: %u\n", _ppg_buf[i], _ppg_counter);
    int32_t left_base = 0, right_base = 0;
    float32_t peak = FindPeak(_ppg_buf[i], &left_base, &right_base, 0u);
    if (fabs(peak) > .0f)
    {
      int32_t peak_index = _ppg_counter - BREATH_PPG_BUF_LEN + i - right_base - 1;

      if (_peaks_len == BREATH_MAX_PEAKS_LEN)
      {
        for (j = 0; j < _peaks_len - 1; ++j)
        {
          _peaks[j] = _peaks[j + 1];
          _peak_indices[j] = _peak_indices[j + 1];
          _left_bases[j] = _left_bases[j + 1];
          _right_bases[j] = _right_bases[j + 1];
        }
        _peaks_len--;
      }

      _peaks[_peaks_len] = peak;
      _peak_indices[_peaks_len] = peak_index;
      _left_bases[_peaks_len] = left_base;
      _right_bases[_peaks_len] = right_base;
      _peaks_len++;
    }
  }
  
  _ppg_nfill = 0u;

  return;
}

uint8_t _CalcSleepBreathScore(uint8_t *breath_rate_buf, uint32_t breath_rate_buf_len)
{
  // return 5;
  int score = rand() % (10) + 90;
  return (uint8_t)score;
}

void BreathAnalysisInit()
{
  uint32_t i;

  breath_arm_biquad_cascade_df1_init_f32 (&_filter, BREATH_NUM_BPF_STAGE, _coeff, _state);
  breath_arm_biquad_cascade_df1_init_f32 (&_filter_ppg, BREATH_NUM_BPF_STAGE, _coeff_ppg, _state_ppg);

  for (i = 0; i < BREATH_PPG_BUF_LEN; ++i) {
    _ppg_buf[i] = .0f;
  }

  _ppg_nfill = 0;
  _ppg_counter = 0;
  // _valley_value = __FLT_MAX__;
  // _valley_index = -1;

  // _candpv.npv = 0;
  // for (int i = 0; i < BREATH_MAX_PV_LEN; i++)
  // {
  //   _candpv.pv[i] = MakeBPV(0, 0, 0);
  // }

  for (int i = 0; i < BREATH_MAX_RRS_LEN; i++)
  {
    // _pv_buf[i] = .0f;
    // _peak_buf[i] = .0f;
    // _peak_index_buf[i] = 0;
    _rr_buf[i] = .0f;
  }
  // _num_peaks = 0;
  _rr_buf_len = 0;
  _total_rr_len = 0;
  _last_total_rr_len = 0;

  FindPeak(0, NULL, NULL, 1);

  for (int i = 0; i < BREATH_MAX_PEAKS_LEN; i++)
  {
    _peaks[i] = 0;
    _peak_indices[i] = 0;
  }
  _peaks_len = 0;
  _last_ppg_counter = 0;

#if DEBUG
  DebugInit();
#endif

  return;
}


int32_t CalcRR(float32_t *peaks, int32_t *peak_indices, int32_t peaks_len, float32_t* rrs)
{
  /* 计算标准差和中位数. */
  int32_t i;
  int32_t rrs_len = 0;
  for (i = 0; i < peaks_len - 1; ++i)
  {
    rrs[rrs_len++] = peaks[i + 1] - peaks[i];
  }

  float32_t std;
  arm_var_f32(rrs, rrs_len, &std);
  std = sqrtf(std);


}


// void BreathAnalysis(bioSignal_t* s, breath_result* result)
// void BreathAnalysis(int* s, uint32_t sample_length, breath_result* result)
void BreathAnalysis(int* s, uint32_t sample_length)
// void BreathAnalysis(int* s, uint32_t sample_length, breath_result* result)
{
  uint32_t i, j;
  
  for (i = 0; i < sample_length; ++i)
  {
    _DetectPeaks((float32_t)s[i]);

    if (_ppg_counter && (_ppg_counter % BREATH_CALL_DURATION == 0))
    {
      _peaks_len = SelectByPeakDistance(_peaks, _peak_indices, _peaks_len, 6, _left_bases, _right_bases);
      // _peaks_len = SelectByPeakProminence(_peaks, _peaks_len, _peak_indices, _left_bases, _right_bases);
      
      float *rrs = _mem_pool;
      float *rrs_indices = rrs + 256;
      int32_t rrs_lens = CalcRR(_peaks, _peak_indices, _peaks_len, rrs, rrs_indices);

#if DEBUG
      for (j = 0; j < _peaks_len; ++j)
      {
        if (_peak_indices[j] > PLOT.peak_indices[PLOT.peak_indices_len - 1])
        {
          PLOT.peaks[PLOT.peaks_len++] = _peaks[j];
          PLOT.peak_indices[PLOT.peak_indices_len++] = _peak_indices[j];
        }
      }
#endif 
      for (j = 0; j < _peaks_len - 1; ++j)
      {
        if (_peak_indices[j] > _last_ppg_counter)
        {
          _rr_buf[_rr_buf_len] = _peak_indices[j + 1] - _peak_indices[j];
          _rr_buf_len++;
        }
      }

      uint8_t breath_rate = _CalcBreathRate(_rr_buf, _rr_buf_len, 25);
      // _breath_rate_buf[_breath_rate_buf_len++] = breath_rate;

#if DEBUG
          // PLOT.breath_rates[PLOT.breath_rates_len++] = breath_rate;
#endif
      
      _rr_buf_len = 0;
      _last_ppg_counter = _ppg_counter;
    }
  }

  // /* 如果不满足最小时长则不计算分数. */
  // if (_ppg_counter < FS * 3600 * 3)
  // {
  //   result->breath_score = BREATH_SCORE_NONE;
  // }
  // else
  // {
  //   uint8_t breath_score = _CalcSleepBreathScore(_breath_rate_buf, _breath_rate_buf_len);
  //   result->breath_score = breath_score;
  // }

  return;
}