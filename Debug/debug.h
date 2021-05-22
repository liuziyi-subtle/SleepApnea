#include <stdint.h>

#define MAX_LEN (1000000)

#define FLT_MAX __FLT_MAX__
#define FLT_MIN __FLT_MIN__
#define INT32_MAX __INT32_MAX__
#define INT32_MIN __INT32_MIN__

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

  float rrs[MAX_LEN];
  int rrs_len;
};

extern struct DEBUG_TRACKER PLOT;

void DebugInit();
struct DEBUG_TRACKER* GetPLOT();
