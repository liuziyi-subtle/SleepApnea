#include <stdint.h>

#define MAX_LENGTH (1000000)

struct DEBUG_TRACKER {
  float x_interp[MAX_LENGTH];
  uint32_t x_interp_len;

  float y_interp[MAX_LENGTH];
  uint32_t y_interp_len;

  float y_interp_f[MAX_LENGTH];
  uint32_t y_interp_f_len;
};

void DebugInit();
struct DEBUG_TRACKER* GetPLOT();