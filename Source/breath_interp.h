#include <stdint.h>

typedef float float32_t;

/**
 * @brief Struct for specifying cubic spline type
 */
typedef enum
{
  ARM_SPLINE_NATURAL = 0,           /**< Natural spline */
  ARM_SPLINE_PARABOLIC_RUNOUT = 1   /**< Parabolic runout spline */
} arm_spline_type;

/**
 * @brief Instance structure for the floating-point cubic spline interpolation.
 */
typedef struct
{
  arm_spline_type type;      /**< Type (boundary conditions) */
  const float32_t * x;       /**< x values */
  const float32_t * y;       /**< y values */
  uint32_t n_x;              /**< Number of known data points */
  float32_t * coeffs;        /**< Coefficients buffer (b,c, and d) */
} arm_spline_instance_f32;


void arm_spline_init_f32(
    arm_spline_instance_f32 * S,
    arm_spline_type type,
    const float32_t * x,
    const float32_t * y,
    uint32_t n,
    float32_t * coeffs,
    float32_t * tempBuffer);


void arm_spline_f32(
    arm_spline_instance_f32 * S,
    const float32_t * xq,
    float32_t * pDst,
    uint32_t blockSize);

