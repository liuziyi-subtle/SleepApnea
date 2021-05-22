#include <math.h>
#include <stdio.h>
#include <float.h>
#include "breath_find_peaks.h"
#include <stdlib.h>

#if DEBUG
#include "debug.h"
#endif

typedef struct _value2index 
{
  float value;
  int32_t index;
} value2index;

static value2index v2i[150];

int CmpValue(const void *a, const void *b){
  if (((value2index*)a)->value - ((value2index*)b)->value > 0)
  {
    return 1;
  }
  else
  {
    return -1;
  }
}

int CmpIndex(const void *a, const void *b){
    return ((value2index*)a)->index - ((value2index*)b)->index;
}

int32_t SelectByPeakDistance(float *peaks, int32_t *peak_indices, int32_t peaks_len, int32_t distance, int32_t *left_bases, int32_t *right_bases)
{
  int32_t i;
  for (i = 0; i < peaks_len; ++i)
  {
    v2i[i].value = peaks[i];
    v2i[i].index = i;
  }
  qsort(v2i, peaks_len, sizeof(value2index), CmpValue);

  for (i = peaks_len - 1; i >= 0; --i)
  {
    int32_t j = v2i[i].index;

    int32_t k = j - 1;
    while ((k >= 0) && (abs(peak_indices[j] - peak_indices[k]) < distance))
    {
      peak_indices[k] = INT32_MAX;
      k -= 1;
    }

    k = j + 1;
    while ((k < peaks_len) && (abs(peak_indices[k] - peak_indices[j]) < distance))
    {
      peak_indices[k] = INT32_MAX;
      k += 1;
    }
  }

  /* Sort by index so as to remove INT32_MAX. */
  for (i = 0; i < peaks_len; ++i)
  {
    v2i[i].value = peaks[i];
    v2i[i].index = peak_indices[i];
  }
  qsort(v2i, peaks_len, sizeof(value2index), CmpIndex);

  for (i = 0; i < peaks_len; ++i)
  {
    peaks[i] = v2i[i].value;
    // peak_indices[i] = v2i[i].index;

    if (v2i[i].index == INT32_MAX)
    {
      break;
    }
  }

  for (i = 0; i < peaks_len; ++i)
  {
    v2i[i].value = (float)right_bases[i];
    v2i[i].index = peak_indices[i];
  }
  qsort(v2i, peaks_len, sizeof(value2index), CmpIndex);  

  for (i = 0; i < peaks_len; ++i)
  {
    right_bases[i] = (int32_t)v2i[i].value;
    // peak_indices[i] = v2i[i].index;

    if (v2i[i].index == INT32_MAX)
    {
      break;
    }
  }

  for (i = 0; i < peaks_len; ++i)
  {
    v2i[i].value = (float)left_bases[i];
    v2i[i].index = peak_indices[i];
  }
  qsort(v2i, peaks_len, sizeof(value2index), CmpIndex);  

  for (i = 0; i < peaks_len; ++i)
  {
    left_bases[i] = (int32_t)v2i[i].value;
    peak_indices[i] = v2i[i].index;

    if (v2i[i].index == INT32_MAX)
    {
      break;
    }
  } 

  return i;
}

int32_t SelectByPeakProminence(float *peaks, int32_t peaks_len, int32_t *peak_indices, int32_t *left_bases, int32_t *right_bases)
{
  int32_t i;

  for (i = 0; i < peaks_len; ++i)
  {
    v2i[i].value = peaks[i];
    if (right_bases[i] > 3)
    {
      v2i[i].index = peak_indices[i];
    }
    else{
      v2i[i].index = INT32_MAX;
    }
  }
  qsort(v2i, peaks_len, sizeof(value2index), CmpIndex);

  for (i = 0; i < peaks_len; ++i)
  {
    peaks[i] = v2i[i].value;
    peak_indices[i] = v2i[i].index;

    if (peak_indices[i] == INT32_MAX)
    {
      // printf("peaks_len: %d, new_peaks_len: %d\n", peaks_len, i);
      break;
    }
  }

  return i;
}

float FindPeak(float datum, int32_t *left_base, int32_t *right_base, uint8_t init)
{
	static float max, last_datum;
  static int32_t since_max, to_max;
	
  float peak = .0f;

	if (init)
  {
    max = FLT_MIN;
    // min = __FLT_MAX__;
    last_datum = FLT_MIN;
    since_max = 0;
    // right_points = 0;
    to_max = 0;
    // left_points = 0;
    return .0f;
	}

  if (datum > last_datum)
  {
    // printf("datum > last_datum: datum = %f, last_datum = %f\n", datum, last_datum);
    /* 此时新的上升开始，返回最近的一次上升+下降. */
    if ((since_max > 0) && (to_max > 0))
    {
      peak = max;
      *left_base = to_max;
      *right_base = since_max;
      max = last_datum;
      // min = __FLT_MAX__;
      to_max = 0;
      since_max = 0;
      // printf("peak: %f, max: %f\n", peak, max);
    }

    if (datum > max)
    {
      max = datum;
      to_max++;
    }
  }
  else if (datum < last_datum)
  {
    // printf("datum < last_datum: datum = %f, last_datum = %f\n", datum, last_datum);
    // right_points = since_max;
    // since_max = 0;
    // if (datum < min)
    // {
    // min = datum;
    since_max++;
      // printf("datum < last_datum: datum = %f, last_datum = %f, min = %f\n", datum, last_datum, min);
    // }
  }
  // else
  // {

  // }

  last_datum = datum;
  return peak;
}