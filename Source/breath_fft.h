#pragma once
#include <stdint.h>
typedef float float32_t;

/**
 * @brief Error status returned by some functions in the library.
 */
typedef enum {
  ARM_MATH_SUCCESS = 0, /**< No error */
  ARM_MATH_ARGUMENT_ERROR = -1, /**< One or more arguments are incorrect */
  ARM_MATH_LENGTH_ERROR = -2, /**< Length of data buffer is incorrect */
  ARM_MATH_SIZE_MISMATCH = -3, /**< Size of matrices is not compatible with the operation */
  ARM_MATH_NANINF = -4, /**< Not-a-number (NaN) or infinity is generated */
  ARM_MATH_SINGULAR = -5, /**< Input matrix is singular and cannot be inverted */
  ARM_MATH_TEST_FAILURE = -6 /**< Test Failed */
} arm_status;

/**
 * @brief Instance structure for the Double Precision Floating-point CFFT/CIFFT
 * function.
 */
typedef struct
{
  uint16_t fftLen;               /**< length of the FFT. */
  const float32_t *pTwiddle;     /**< points to the Twiddle factor table. */
  const uint16_t *pBitRevTable;  /**< points to the bit reversal table. */
  uint16_t bitRevLength;         /**< bit reversal table length. */

} arm_cfft_instance_f32;


/**
 * @brief Instance structure for the Double Precision Floating-point RFFT/RIFFT
 * function.
 */
typedef struct
{
  arm_cfft_instance_f32 Sint;      /**< Internal CFFT structure. */
  uint16_t fftLenRFFT;             /**< length of the real sequence */
  const float32_t * pTwiddleRFFT;        /**< Twiddle factors real stage  */
} arm_rfft_fast_instance_f32 ;

                  
arm_status arm_rfft_128_fast_init_f32( arm_rfft_fast_instance_f32 * S );

void arm_cfft_f32(const arm_cfft_instance_f32 * S,
                  float32_t * p1,
                  uint8_t ifftFlag,
                  uint8_t bitReverseFlag);

void arm_rfft_fast_f32(const arm_rfft_fast_instance_f32 * S,
                       float32_t * p,
                       float32_t * pOut,
                       uint8_t ifftFlag);