/* module */
%module c2py
%include "carrays.i"
%include "stdint.i"
%include "typemaps.i"
%include "cdata.i"

%array_class(float, float_array);
%array_class(int, int_array);

/* C wrapper */
%{
#include "debug.h"
#include "breath.h"

static int teststructErr = 0;
%}

%include "debug.h"

void DebugInit();
struct DEBUG_TRACKER* GetPLOT();
// uint8_t CalcBreathRate(float *rrs, uint16_t rrs_len, uint16_t fs);
void BreathAnalysisInit();
// void BreathAnalysis(bioSignal_t* s, breath_result* result);
void BreathAnalysis(int* s, uint32_t sample_length);

%extend struct DEBUG_TRACKER{
  // add a __getitem__ method to the structure to get values from the data array
  // float __getitem__(size_t i) {
  //   // x_interp & x_interp_len
  //   if (i >= $self->x_interp_len) {
  //     teststructErr = 1;
  //     return 0;
  //   }
  //   return ($self->x_interp[i]);
  // }

  // x_interp
  float getitem_x_interp(size_t i) {
    if (i >= $self->x_interp_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->x_interp[i]);
  }

  // y_interp
  float getitem_y_interp(size_t i) {
    if (i >= $self->y_interp_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->y_interp[i]);
  }

  // y_interp_f
  float getitem_y_interp_f(size_t i) {
    if (i >= $self->y_interp_f_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->y_interp_f[i]);
  }

  //  peaks
  float getitem_peaks(size_t i) {
    if (i >= $self->peaks_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->peaks[i]);
  }

  // peak_indices
  int32_t getitem_peak_indices(size_t i) {
    if (i >= $self->peak_indices_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->peak_indices[i]);
  }

  // ppg
  float getitem_ppg(size_t i) {
    if (i >= $self->ppg_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->ppg[i]);
  }

  // ppg_f
  float getitem_ppg_f(size_t i) {
    if (i >= $self->ppg_f_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->ppg_f[i]);
  }

  // breath_rates
  int32_t getitem_breath_rates(size_t i) {
    if (i >= $self->breath_rates_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->breath_rates[i]);
  }

  // rrs
  float getitem_rrs(size_t i) {
    if (i >= $self->rrs_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->rrs[i]);
  }

  // welch_density
  float getitem_welch_density(size_t i) {
    if (i >= $self->welch_density_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->welch_density[i]);
  }
}