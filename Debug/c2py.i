/* module */
%module c2py
%include "carrays.i"
%include "stdint.i"
%include "typemaps.i"
%include "cdata.i"

%array_class(float, float_array);

/* C wrapper */
%{
#include "debug.h"
#include "breath_analysis.h"

static int teststructErr = 0;
%}

%include "debug.h"

void DebugInit();
struct DEBUG_TRACKER* GetPLOT();
uint8_t CalcBreathRate(float *rrs, uint16_t rrs_len, uint16_t fs);

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

  float getitem_x_interp(size_t i) {
    if (i >= $self->x_interp_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->x_interp[i]);
  }

  float getitem_y_interp(size_t i) {
    if (i >= $self->y_interp_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->y_interp[i]);
  }

  float getitem_y_interp_f(size_t i) {
    if (i >= $self->y_interp_f_len) {
      teststructErr = 1;
      return 0;
    }
    return ($self->y_interp_f[i]);
  }
}