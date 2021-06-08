#ifndef _BREATH_H_
#define _BREATH_H_

#include <stdint.h>

typedef float float32_t;

#define BREATH_SCORE_NONE (0)  /*<< No breath score. */
#define BREATH_RATE_NONE  (0)  /*<< No breath rate. */

typedef enum slTerminator {
  SL_T_NONE = 0,
  SL_T_TEMP = 1,
  SL_T_COMP = 2,
} SlTerminator_t;

typedef struct
{
    SlTerminator_t completeCycleIndicator;
    uint32_t sleepTimeUtc;
    uint32_t getupTimeUtc;
} sleep_result;

typedef struct
{
  uint8_t breath_score;
  uint8_t breath_rate;
} breath_result;

void BreathAnalysisInit(void);
void BreathAnalysis(int* s, int32_t sample_length, breath_result* result, sleep_result *sl_result);

#endif
