#ifndef _BREATH_FIND_PEAKS_H_
#define _BREATH_FIND_PEAKS_H_

#include <stdint.h>

int32_t SelectByPeakDistance(float *peaks, int32_t *peak_indices, int32_t peaks_len, int32_t distance, int32_t *left_bases, int32_t *right_bases);
int32_t SelectByPeakProminence(float *peaks, int32_t peaks_len, int32_t *peak_indices, int32_t *left_base, int32_t *right_base);
float FindPeak(float datum, int32_t *left_base, int32_t *right_base, uint8_t init);

#endif
