#include "ls_sleep_breath.h"

#define FS (25)

static uint16_t k_fill = 0;
static k_ppg_buf[FS];
uint16_t SleepBreath(const float ppg) {
  
  k_ppg_buf[k_fill++] = ppg;
  
  if (k_fill < FS) {
    return 0;
  } else {
    k_fill = 0u;
  }





}