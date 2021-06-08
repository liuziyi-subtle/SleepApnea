#include "debug.h"
#include "breath.h"

#include <stdio.h>
#include <stdlib.h>

struct DEBUG_TRACKER PLOT = {{0}, 0, {0}, 0, {0}, 0, {0}, 0, {0}, 0, {0}, 0};

/* Init */
void DebugInit() {
  PLOT.x_interp_len = 0;
  PLOT.y_interp_len = 0;
  PLOT.y_interp_f_len = 0;
  PLOT.welch_density_len = 0;
  PLOT.peaks_len = 0;
  PLOT.peak_indices_len = 0;
  PLOT.ppg_len = 0;
  PLOT.ppg_f_len = 0;
  PLOT.breath_rates_len = 0;
  PLOT.rrs_x_len = 0;
  PLOT.rrs_y_len = 0;
}

/* Get PLOT */
struct DEBUG_TRACKER* GetPLOT() {
  // struct DEBUG_TRACKER* out = (struct DEBUG_TRACKER*)malloc(sizeof(struct DEBUG_TRACKER));
  struct DEBUG_TRACKER* out = &PLOT;
  return out;
}

/* Call BreathAnalysis. */
void CallBreathAnalysis(int* s, int32_t sample_length, uint8_t init)
{
  /* Initialize BreathAnalysis. */
  if (init)
  {
    BreathAnalysisInit();
    return;
  }

  uint32_t sleepTimeUtc = 1623025118;
  uint32_t getupTimeUtc = 1623025118;
  SlTerminator_t completeCycleIndicator = SL_T_TEMP;

  sleep_result sl_result = {completeCycleIndicator, sleepTimeUtc, getupTimeUtc};
  breath_result result = {BREATH_SCORE_NONE, BREATH_RATE_NONE};

  int32_t i;
  for (i = 0; i < sample_length; ++i)
  {
    if (i && (i % 60 == 0))
    {
      sl_result.getupTimeUtc += 60;
    }

    if (i % 25 == 0)
    {
      BreathAnalysis(&s[i], 25, &result, &sl_result);
    }
  }

  return;
}