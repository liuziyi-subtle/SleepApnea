#pragma once
#include <stdint.h>

typedef float float32_t;

typedef enum {
  SOURCE_ECG = 0,
  SOURCE_ACC = 0,
  SOURCE_PPG_G,
  SOURCE_PPG_IR
} bioSigSource_t;

typedef struct {
  bioSigSource_t sensor_type;
  int32_t sample_length;
  uint32_t* time_stamp;
  union {
    int32_t* signal;
    struct {
      int32_t* x;
      int32_t* y;
      int32_t* z;
    } acc_t;
  } sig_t;
  int32_t* serial_number;
  uint8_t* led;
  uint8_t is_from_ui;
} bioSignal_t;

typedef struct
{
  uint8_t breath_score;
  uint8_t breath_rate_latest;
} breath_result;

void BreathAnalysisInit();
// void BreathAnalysis(bioSignal_t* s, breath_result* result);
void BreathAnalysis(int* s, uint32_t sample_length);
// void BreathAnalysis(int* s, uint32_t sample_length, breath_result* result);