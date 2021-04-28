#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

struct DEBUG_TRACKER PLOT = {{0}, 0, {0}, 0, {0}, 0};

/* Init */
void DebugInit() {
  PLOT.x_interp_len = 0;
  PLOT.y_interp_len = 0;
  PLOT.y_interp_f_len = 0;
}

/* Get PLOT */
struct DEBUG_TRACKER* GetPLOT() {
  // struct DEBUG_TRACKER* out = (struct DEBUG_TRACKER*)malloc(sizeof(struct DEBUG_TRACKER));
  struct DEBUG_TRACKER* out = &PLOT;
  return out;
}
