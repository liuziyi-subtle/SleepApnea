#include <stdint.h>

#define MAX_LEN (1000000)

struct DEBUG_TRACKER {
  float x_interp[MAX_LEN];
  uint32_t x_interp_len;

  float y_interp[MAX_LEN];
  uint32_t y_interp_len;

  float y_interp_f[MAX_LEN];
  uint32_t y_interp_f_len;

  float welch_density[MAX_LEN];
  uint32_t welch_density_len;

  float peaks[MAX_LEN];
  int peaks_len;

  int32_t peak_indices[MAX_LEN];
  int peak_indices_len;

  float ppg[MAX_LEN];
  int ppg_len;

  float ppg_f[MAX_LEN];
  int ppg_f_len;

  int breath_rates[MAX_LEN];
  int breath_rates_len;

  float rrs_x[MAX_LEN];
  int rrs_x_len;

  float rrs_y[MAX_LEN];
  int rrs_y_len;
};

extern struct DEBUG_TRACKER PLOT;

void DebugInit();
struct DEBUG_TRACKER* GetPLOT();
void CallBreathAnalysis(int* s, int32_t sample_length, uint8_t init);
