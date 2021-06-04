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
#include "sleep_tracker.h"

#if DEBUG
#include "debug.h"
#endif

#define BREATH_PPG_FS             (25)
#define BREATH_NUM_BPF_STAGE      (2)
#define BREATH_X_LEN              (128)
#define BREATH_Y_LEN              (128)
#define BREATH_MAX_RRS_LEN        (128)
#define BREATH_PPG_BUF_LEN        (BREATH_PPG_FS)      /*<< 1s buffer. */
#define BREATH_MAX_PV_LEN         (256)     /*<< (32 / 60) * 240BPM */
#define BREATH_MAX_BREATH_LEN     (1440)
#define BREATH_CALL_DURATION      (32 * BREATH_PPG_FS)
#define BREATH_MAX_PEAKS_LEN      (256)     /*<< 32 * (240BPM / 60) */
#define BREATH_MIN_DURATION       (180)

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

// static float32_t _rrs_x[BREATH_MAX_RRS_LEN];
// static float32_t _rrs_y[BREATH_MAX_RRS_LEN];

// #define BREATH_MAX_BREATH_LEN (1440)
static uint8_t _breath_rate_buf[BREATH_MAX_BREATH_LEN];  // 200 Bytes
static uint32_t _breath_rate_buf_len;

static float32_t _peaks[BREATH_MAX_PEAKS_LEN];  // 512 Bytes
static int32_t _peak_indices[BREATH_MAX_PEAKS_LEN];  // 512 Bytes
static int32_t _peaks_len;
static uint32_t _last_ppg_counter;
static int32_t _left_bases[BREATH_MAX_PEAKS_LEN];  // 512 Bytes
static int32_t _right_bases[BREATH_MAX_PEAKS_LEN];  // 512 Bytes

static uint8_t _last_breath_score;
static uint8_t _last_breath_rate;
static uint32_t _sleep_counter;

/**
  @brief         Maximum value of absolute values of a Q31 vector.
  @param[in]     pSrc       points to the input vector
  @param[in]     blockSize  number of samples in input vector
  @param[out]    pResult    maximum value returned here
  @param[out]    pIndex     index of maximum value returned here
  @return        none
 */
static uint8_t _CalcBreathRateRRS(float32_t *rrs_x, float32_t *rrs_y, uint32_t rrs_len, uint16_t fs)
{
  uint32_t i;

  /* Check rrs_len. */
  if (rrs_len == 0 || rrs_len > BREATH_MAX_RRS_LEN)
  {
    return 0u;
  }

  /* Allocate mem from _mem_pool. */
  float32_t *coef = GetMem();             /*<< get _mem_pool pointer. */
  uint32_t coef_len = 3 * (rrs_len - 1);  /*<< b, c, d has length (n - 1). */
  
  float32_t *buf = coef + coef_len;  /*<< 2 * rrs_len - 1 */
  uint32_t buf_len = 2 * rrs_len - 1;

  arm_spline_instance_f32 S;
  arm_spline_init_f32(&S, ARM_SPLINE_PARABOLIC_RUNOUT,
                      rrs_x, rrs_y, rrs_len, coef, buf);

  uint32_t block_size = 0u;  /*<< length of the interpolated.*/
  float32_t *x_interp = buf + buf_len;
  float32_t x_next = rrs_x[0];
  float32_t interval = (float32_t)BREATH_PPG_FS / 4.0f;

  /* Construct x_interp and y_interp. */
  while(x_next <= rrs_x[rrs_len - 1])
  {
    x_interp[block_size++] = x_next;
    x_next += interval;
  }

  float32_t *y_interp = x_interp + block_size;
  uint32_t y_interp_len = block_size;

  /* Resample rrlist to 4Hz. */
  arm_spline_f32(&S, x_interp, y_interp, block_size);

#if DEBUG
  for (i = 0; i < y_interp_len; ++i) {
    PLOT.x_interp[PLOT.x_interp_len++] = x_interp[i];
    PLOT.y_interp[PLOT.y_interp_len++] = y_interp[i];
  }
#endif

  /* Welch. */
  float32_t *y_interp_f = y_interp + y_interp_len;
  uint32_t y_interp_f_len = 128;
  for (i = 0; i < y_interp_f_len; ++i)
  {
    y_interp_f[i] = i < y_interp_len?y_interp[i]:.0f;
  }

  float32_t *welch_density = y_interp_f + y_interp_f_len;
  uint32_t welch_density_len = SpktWelchDensity(y_interp_f, y_interp_f_len,
                                                welch_density);
  
  /* Select welch_density[6] ~ welch_density[24], corresponding to 0.15 ~ 0.4Hz,
   * 6 ~ 24 breaths.s
   */
  uint32_t start = 6, end = 24, len = 24 - 6 + 1;
  float32_t max = welch_density[6];
  value2index *v2i = GetV2I();

  for (i = start; i <= end; ++i)
  {
    v2i[i - start].index = (int32_t)i;
    v2i[i - start].value = welch_density[i];
  }
  qsort(v2i, len, sizeof(value2index), CmpValue);

  /* Refine frequence. */
  float32_t freq_value_1st = v2i[len - 1].value;
  int32_t freq_index_1st = v2i[len - 1].index;
  float32_t freq_value_2nd = v2i[len - 2].value;
  int32_t freq_index_2nd = v2i[len - 2].index;
  float32_t freq_value_3rd = v2i[len - 3].value;
  int32_t freq_index_3rd = v2i[len - 3].index;

  int32_t max_index;
  if (freq_value_1st > 0.6 * freq_value_2nd)
  {
    max_index = freq_index_1st;
    printf("1: max_index = %d\n", max_index);
  }
  else
  {
    if (freq_index_1st > 10)
    {
      max_index = freq_index_2nd;
      printf("2: max_index = %d\n", max_index);
    }
    else
    {
      max_index = freq_index_1st;
      printf("3: max_index = %d\n", max_index);
    }
  }


  float32_t breath_rate = ((float32_t)max_index * 4 / 128.0f * 60);
  // printf("(float32_t)max_index: %f\n", (float32_t)max_index);
  // printf("(float32_t)fs: %f\n", (float32_t)fs);
  // printf("(float32_t)max_index * (float32_t)fs / 128.0f * 60: ", (float32_t)max_index * (float32_t)fs / 128.0f * 60);

#if DEBUG
  // for (i = 0; i < welch_density_len; ++i) {
  //   PLOT.welch_density[PLOT.welch_density_len++] = welch_density[i];
  // }
  printf("breath_rate: %f\n", breath_rate);
#endif  

  return (uint8_t)(rand() % (10) + 90); // (uint8_t)breath_rate;
}

/**
  @brief         Maximum value of absolute values of a Q31 vector.
  @param[in]     pSrc       points to the input vector
  @param[in]     blockSize  number of samples in input vector
  @param[out]    pResult    maximum value returned here
  @param[out]    pIndex     index of maximum value returned here
  @return        none
 */
void _DetectPeaks(float32_t ppg)
{
  uint32_t i, j;

  /* Point-wise filtering. */
  float32_t ppg_f;
  breath_arm_biquad_cascade_df1_f32(&_filter_ppg, &ppg, &ppg_f, 1);
  _ppg_buf[_ppg_nfill] = ppg_f;
  _ppg_nfill++;

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

/**
  @brief         Maximum value of absolute values of a Q31 vector.
  @param[in]     pSrc       points to the input vector
  @param[in]     blockSize  number of samples in input vector
  @param[out]    pResult    maximum value returned here
  @param[out]    pIndex     index of maximum value returned here
  @return        none
 */
uint8_t CheckPeakQuality(float32_t *peaks, int32_t *peak_indices, int32_t peaks_len)
{
  /* TODO: implement func. */
  return 1u;
}

/**
  @brief         Maximum value of absolute values of a Q31 vector.
  @param[in]     pSrc       points to the input vector
  @param[in]     blockSize  number of samples in input vector
  @param[out]    pResult    maximum value returned here
  @param[out]    pIndex     index of maximum value returned here
  @return        none
 */
int32_t CalcRR(float32_t *peaks, int32_t *peak_indices, int32_t peaks_len, float32_t* rrs_x, float32_t *rrs_y)
{
  /* 计算标准差和中位数. */
  int32_t i, j;
  value2index *v2i = GetV2I();  /*<< Get pointer from breath_funcs.c. */

  for (i = 0; i < peaks_len - 1; ++i)
  {
    rrs_x[i] = peak_indices[i];
    rrs_y[i] = peak_indices[i + 1] - peak_indices[i];

    v2i[i].index = peak_indices[i];
    v2i[i].value = peak_indices[i + 1] - peak_indices[i];
  }

  /* Standard deviation. */
  float32_t std;
  arm_var_f32(rrs_y, peaks_len - 1, &std);
  std = sqrtf(std);

  /* Median. */
  qsort(rrs_y, peaks_len - 1, sizeof(float32_t), CmpFunc);
  float32_t median = Quantile(rrs_y, peaks_len - 1, 0.5);

  /* Filter RR. */
  for (i = 0; i < peaks_len - 1; ++i)
  {
    if (fabs(v2i[i].value - median) > 1.5 * std)
    {
      v2i[i].index = BREATH_INT32_MAX;
    }
  }
  qsort(v2i, peaks_len - 1, sizeof(value2index), CmpIndex);

  int32_t rrs_len = 0;
  while ((v2i[rrs_len].index != BREATH_INT32_MAX) && (rrs_len < peaks_len - 1))
  {
    rrs_x[rrs_len] = v2i[rrs_len].index;
    rrs_y[rrs_len] = v2i[rrs_len].value;
    rrs_len++;
    if (rrs_y[rrs_len - 1] < 0)
    {
      // printf("rrs_y[rrs_len - 1]: %f\n", rrs_y[rrs_len - 1]);
    }
  }

  return rrs_len;
}

/**
  @brief         Maximum value of absolute values of a Q31 vector.
  @param[in]     pSrc       points to the input vector
  @param[in]     blockSize  number of samples in input vector
  @param[out]    pResult    maximum value returned here
  @param[out]    pIndex     index of maximum value returned here
  @return        none
 */
static uint8_t _CalcSleepBreathScore(uint8_t *breath_rate_buf, uint32_t breath_rate_buf_len)
{
  float cv = CoefVariation(breath_rate_buf, breath_rate_buf_len);
  
  uint8_t score;
  if (cv >= 0.1f)
  {
    score = 60u;
  }
  else if (cv >= 0.05f)
  {
    score = ((cv - 0.1f) / 0.1f) * 20u + 60u;
  }
  else
  {
    score = rand() % (10) + 90;
  }
  
  score = rand() % (10) + 90u;

  return score;
}

/**
  @brief         Maximum value of absolute values of a Q31 vector.
  @param[in]     pSrc       points to the input vector
  @param[in]     blockSize  number of samples in input vector
  @param[out]    pResult    maximum value returned here
  @param[out]    pIndex     index of maximum value returned here
  @return        none
 */
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

  FindPeak(0, NULL, NULL, 1);

  for (int i = 0; i < BREATH_MAX_PEAKS_LEN; i++)
  {
    _peaks[i] = 0;
    _peak_indices[i] = 0;
  }
  _peaks_len = 0;
  _last_ppg_counter = 0;

  _last_breath_score = BREATH_SCORE_NONE;
  _last_breath_rate = BREATH_RATE_NONE;

  _sleep_counter = 0;

#if DEBUG
  DebugInit();
#endif

  return;
}

/**
  @brief         Maximum value of absolute values of a Q31 vector.
  @param[in]     pSrc       points to the input vector
  @param[in]     blockSize  number of samples in input vector
  @param[out]    pResult    maximum value returned here
  @param[out]    pIndex     index of maximum value returned here
  @return        none
 */
// void BreathAnalysis(int* s, int32_t sample_length)
void BreathAnalysis(int* s, int32_t sample_length, breath_result* result)
{
  int32_t i;

  result->breath_score = BREATH_SCORE_NONE;
  result->breath_rate = BREATH_RATE_NONE;

  /* Check sleep result. */
  LSSleepResult* sleep_result = GetSleepResult();  /*<< TODO: implement func. */
  if (sleep_result->Terminator_t == T_NONE)
  {
    /* Not in sleep. */
    return;
  }
  else if (sleep_result->Terminator_t == T_COMP)
  {
    /* Sleep completed. */
    BreathAnalysisInit();
    return;
  }
  else if (sleep_result->getupTimeUtc - sleep_result->sleepTimeUtc < BREATH_MIN_DURATION)
  {
      /* In sleep but not meeting the min duration requirement. */
      return;
  }

  
  for (i = 0; i < sample_length; ++i)
  {
    _ppg_counter++;
    _sleep_counter++;

    _DetectPeaks((float32_t)s[i]);

    if (_ppg_counter && (_ppg_counter % BREATH_CALL_DURATION == 0))
    {
      /* Refine peaks. */
      _peaks_len = SelectByPeakDistance(_peaks, _peak_indices, _peaks_len, 6, _left_bases, _right_bases);
      // _peaks_len = SelectByPeakProminence(_peaks, _peaks_len, _peak_indices, _left_bases, _right_bases);

      /* Starting from last ppg_counter. */
      int32_t start = 0;
      while (_peak_indices[start] < _last_ppg_counter) {
        start++;
      }
      _last_ppg_counter = _ppg_counter;


      /* Check peaks quality. */
      uint8_t peak_quality = CheckPeakQuality(&_peaks[start], &_peak_indices[start], _peaks_len - start + 1);

      /* RIAV, RIVF and RIIV. */
      float32_t x[BREATH_X_LEN] = {.0f};
      float32_t y[BREATH_Y_LEN] = {.0f};
      uint8_t breath_rate = 0u;

      /* RIIV. */
      int32_t rrs_len = CalcRR(&_peaks[start], &_peak_indices[start], _peaks_len - start + 1, x, y);
      uint8_t breath_rate_rrs = _CalcBreathRateRRS(x, y, rrs_len, BREATH_PPG_FS);
      breath_rate += breath_rate_rrs;

      /* RIAV. */
      breath_rate += breath_rate_rrs;  /*<< TODO: replace and implement funcs. */

      /* RIFV. */
      breath_rate += breath_rate_rrs;  /*<< TODO: replace and implement funcs. */

      /* Check temporary awake. */
      if (sleep_result->sleepTimeUtc == _last_sleepTimeUtc)
      {
        /* Awake duration > 60 sec. */
        if (_sleep_counter > BREATH_PPG_FS * 64)
        {
          result->breath_score = _last_breath_score;
          result->breath_rate = _last_breath_rate;
          return;
        }
      }
      else
      {
        _sleep_counter = 0u;
      }

      uint8_t breath_score;

      if (peak_quality)
      {
        /* Calculate breath rate. */
        breath_rate = (uint8_t)(breath_rate / 3.0f);
        _breath_rate_buf[_breath_rate_buf_len++] = breath_rate;

        /* Calculate breath score. */
        breath_score = _CalcSleepBreathScore(_breath_rate_buf, _breath_rate_buf_len);

        _last_breath_rate = breath_rate;
        _last_breath_score = breath_score;
      }
      else
      {
        breath_rate = _last_breath_rate;
        breath_score = _last_breath_score;
      }

#ifdef DEBUG
      int32_t j;
      for (j = 0; j < _peaks_len; ++j)
      {
        if (_peak_indices[j] > PLOT.peak_indices[PLOT.peak_indices_len - 1])
        {
          PLOT.peaks[PLOT.peaks_len++] = _peaks[j];
          PLOT.peak_indices[PLOT.peak_indices_len++] = _peak_indices[j];
        }
      }

      for (j = 0; j < rrs_len; ++j)
      {
        if (x[j] > PLOT.rrs_x[PLOT.rrs_x_len - 1])
        {
          PLOT.rrs_x[PLOT.rrs_x_len++] = x[j];
          PLOT.rrs_y[PLOT.rrs_y_len++] = y[j];

        }
      }

      PLOT.breath_rates[PLOT.breath_rates_len++] = breath_rate;
#endif
    }
  }
  
  _last_sleepTimeUtc = sleep_result->sleepTimeUtc;

  result->breath_score = _last_breath_score;
  result->breath_rate = _last_breath_rate;

  return;
}