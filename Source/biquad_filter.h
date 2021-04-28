#pragma once
#include <stdint.h>
#include <string.h>		// memset()

/**
 *  @brief Instance structure for the floating-point Biquad cascade filter.
 */
typedef struct
{
    uint32_t numStages;      /**< number of 2nd order stages in the filter.  Overall order is 2*numStages. */
    double *pState;       /**< Points to the array of state coefficients.  The array is of length 4*numStages. */
    double *pCoeffs;      /**< Points to the array of coefficients.  The array is of length 5*numStages. */
} biquad_casd_df1_inst_f32;

void biquad_cascade_df1_init_f32(
        biquad_casd_df1_inst_f32 * S,
        uint8_t numStages,
        double * pCoeffs,
        double * pState);

void biquad_cascade_df1_f32(
        const biquad_casd_df1_inst_f32 * S,
        double * pSrc,
        double * pDst,
        uint32_t blockSize);
