#pragma once
#include <stdint.h>

typedef float float32_t;

#define BREATH_SCORE_NONE (0)  /*<< No breath score. */
#define BREATH_RATE_NONE  (0)  /*<< No breath rate. */

typedef struct
{
  uint8_t breath_score;
  uint8_t breath_rate;
} breath_result;

void BreathAnalysisInit();
void BreathAnalysis(int* s, int32_t sample_length, breath_result* result);
// void BreathAnalysis(int* s, uint32_t sample_length, breath_result* result);