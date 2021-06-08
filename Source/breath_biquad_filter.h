#ifndef _BREATH_BIQUAD_FILTER_H_
#define _BREATH_BIQUAD_FILTER_H_

#include <stdint.h>
typedef float float32_t;

/**
 * @brief Instance structure for the floating-point Biquad cascade filter.
 */
typedef struct
{
    uint32_t numStages;      /**< number of 2nd order stages in the filter.  Overall order is 2*numStages. */
    float32_t *pState;       /**< Points to the array of state coefficients.  The array is of length 4*numStages. */
    const float32_t *pCoeffs;      /**< Points to the array of coefficients.  The array is of length 5*numStages. */
} breath_arm_biquad_casd_df1_inst_f32;

void breath_arm_biquad_cascade_df1_init_f32(
    breath_arm_biquad_casd_df1_inst_f32 * S,
    uint8_t numStages,
    const float32_t * pCoeffs,
    float32_t * pState);

void breath_arm_biquad_cascade_df1_f32(
    const breath_arm_biquad_casd_df1_inst_f32 * S,
    const float32_t * pSrc,
    float32_t * pDst,
    uint32_t blockSize);

#endif
