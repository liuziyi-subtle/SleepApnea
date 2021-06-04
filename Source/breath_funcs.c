#include "breath_fft.h"
#include "breath_funcs.h"
#include <math.h>
#include <stdio.h>
#include <float.h>

#if DEBUG
#include "debug.h"
#endif

#define BREATH_HANN_SCALE (0.041666666666666664353702032031)

/* signal.get_window('hann', 128). */
const static float32_t _hann_window[128] = 
{
  0.        , 0.00060227, 0.00240764, 0.00541175, 0.00960736,
  0.01498437, 0.02152983, 0.02922797, 0.03806023, 0.04800535,
  0.05903937, 0.07113569, 0.08426519, 0.09839623, 0.11349477,
  0.12952444, 0.14644661, 0.16422052, 0.18280336, 0.20215035,
  0.22221488, 0.24294863, 0.26430163, 0.28622245, 0.30865828,
  0.33155507, 0.35485766, 0.37850991, 0.40245484, 0.42663476,
  0.45099143, 0.47546616, 0.5       , 0.52453384, 0.54900857,
  0.57336524, 0.59754516, 0.62149009, 0.64514234, 0.66844493,
  0.69134172, 0.71377755, 0.73569837, 0.75705137, 0.77778512,
  0.79784965, 0.81719664, 0.83577948, 0.85355339, 0.87047556,
  0.88650523, 0.90160377, 0.91573481, 0.92886431, 0.94096063,
  0.95199465, 0.96193977, 0.97077203, 0.97847017, 0.98501563,
  0.99039264, 0.99458825, 0.99759236, 0.99939773, 1.        ,
  0.99939773, 0.99759236, 0.99458825, 0.99039264, 0.98501563,
  0.97847017, 0.97077203, 0.96193977, 0.95199465, 0.94096063,
  0.92886431, 0.91573481, 0.90160377, 0.88650523, 0.87047556,
  0.85355339, 0.83577948, 0.81719664, 0.79784965, 0.77778512,
  0.75705137, 0.73569837, 0.71377755, 0.69134172, 0.66844493,
  0.64514234, 0.62149009, 0.59754516, 0.57336524, 0.54900857,
  0.52453384, 0.5       , 0.47546616, 0.45099143, 0.42663476,
  0.40245484, 0.37850991, 0.35485766, 0.33155507, 0.30865828,
  0.28622245, 0.26430163, 0.24294863, 0.22221488, 0.20215035,
  0.18280336, 0.16422052, 0.14644661, 0.12952444, 0.11349477,
  0.09839623, 0.08426519, 0.07113569, 0.05903937, 0.04800535,
  0.03806023, 0.02922797, 0.02152983, 0.01498437, 0.00960736,
  0.00541175, 0.00240764, 0.00060227
};

// static float64_t _mem_pool[1024];  // 32768 Bytes
static float _mem_pool[2048];
static value2index v2i[256];

float32_t Mean(const float32_t* data, uint32_t data_len) {
  uint32_t count;      /*<< loop counter */
  float32_t sum = 0.0; /*<< temporary result storage */

  /* Initialize counter with number of samples */
  count = data_len;
  while (count > 0U) {
    sum += *data++;
    count--;
  }

  float32_t mean = (sum / data_len);

  return mean;
}

void _SortFunc(float32_t* data, uint32_t data_length) {
  float32_t temp;
  uint32_t i, j;
  // Sort the array nums in ascending order
  for (i = 0; i < data_length - 1; i++) {
    for (j = i + 1; j < data_length; j++) {
      if (data[j] < data[i]) {
        // swap elements
        temp = data[i];
        data[i] = data[j];
        data[j] = temp;
      }
    }
  }

  return;
}

void FindMax(float32_t * data, uint32_t data_len, float32_t* maxval,
             uint32_t * maxval_index)
{
  uint32_t i;
  
  *maxval_index = 0u;
  *maxval = data[*maxval_index];
  
  for (i = 0; i < data_len; ++i)
  {
    if (data[i] > *maxval)
    {
      *maxval = data[i];
      *maxval_index = i;
    }
  }

  printf("In func: maxval = %f, maxval_index = %u\n", *maxval, *maxval_index);
  return;
}

uint32_t SpktWelchDensity(float32_t* data, uint32_t data_len, float32_t* results) {
  float32_t mean = Mean(data, data_len);
  // float32_t* temp_data = _mem_pool;
  // float64_t* results = _mem_pool + data_len;

  uint32_t i;
  for (i = 0; i < data_len; ++i)  /*  */
  {
    data[i] = (data[i] - mean) * _hann_window[i];
  }

#ifdef DEBUG
  // printf("==================== Enter breath_funcs.c =====================\n");
  // for (i = 0; i < data_len; ++i)
  // {
  //   printf("%f,\n", data[i]);
  // }

  // printf("---------------------------------------------------------------\n");
#endif

  arm_rfft_fast_instance_f32 S;
  arm_rfft_128_fast_init_f32(&S);
  arm_rfft_fast_f32(&S, data, results, 0);

  for (i = 0; i < data_len; i += 2)
  {
    results[i] = (pow(results[i], 2) + pow(results[i + 1], 2)) * BREATH_HANN_SCALE;
    // results[i] = sqrtf(pow(results[i], 2) + pow(results[i + 1], 2));
  }

  /* Move results[0], results[2], results[4], ... to results[0], results[2],
   * results[4], ... .*/
  float32_t* p = &results[1];
  float32_t* q = p + 1;
  while (p != &results[(int)(data_len / 2)]) {
    *p = *q;
    *p = (*p) * 2;  /*<< why *2? */
    q += 2;
    p++;
  }

#ifdef DEBUG
  // for (i = 0; i < (uint32_t)(data_len / 2); ++i)
  // {
  //   printf("results[%u]: %f\n", i, results[i] / 2.0f);
  // }
  // printf("==================== Quite breath_funcs.c ====================\n");
#endif

  return (uint32_t)(data_len / 2);
}


struct bpv MakeBPV(uint8_t type, int pos, float val)
{
    struct bpv temp;
    temp.type = type;
    temp.pos = pos;
    temp.val = val;
    return temp;
}

#ifndef __CC_ARM
void breath_arm_var_f32(
  float * pSrc,
  uint32_t blockSize,
  float * pResult)
{

  float sum = 0.0f;                          /* Temporary result storage */
  float sumOfSquares = 0.0f;                 /* Sum of squares */
  float in;                                  /* input value */
  uint32_t blkCnt;                               /* loop counter */

#ifndef ARM_MATH_CM0_FAMILY
   
  /* Run the below code for Cortex-M4 and Cortex-M3 */

  float meanOfSquares, mean, squareOfMean;   /* Temporary variables */

    if(blockSize == 1)
    {
        *pResult = 0;
        return;
    }

  if (blockSize == 0)
  {
    *pResult = 0;
    return;
  }

  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.    
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1])  */
    /* Compute Sum of squares of the input samples    
     * and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.    
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute Sum of squares of the input samples    
     * and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute Mean of squares of the input samples    
   * and then store the result in a temporary variable, meanOfSquares. */
  meanOfSquares = sumOfSquares / ((float) blockSize - 1.0f);

  /* Compute mean of all input values */
  mean = sum / (float) blockSize;

  /* Compute square of mean */
  squareOfMean = (mean * mean) * (((float) blockSize) /
                                  ((float) blockSize - 1.0f));

  /* Compute variance and then store the result to the destination */
  *pResult = meanOfSquares - squareOfMean;

#else

  /* Run the below code for Cortex-M0 */
  float squareOfSum;                         /* Square of Sum */

    if(blockSize == 1)
    {
        *pResult = 0;
        return;
    }

  if(blockSize == 0)
  {
    *pResult = 0;
    return;
  }

  
  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute Sum of squares of the input samples     
     * and then store the result in a temporary variable, sumOfSquares. */
    in = *pSrc++;
    sumOfSquares += in * in;

    /* C = (A[0] + A[1] + ... + A[blockSize-1]) */
    /* Compute Sum of the input samples     
     * and then store the result in a temporary variable, sum. */
    sum += in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute the square of sum */
  squareOfSum = ((sum * sum) / (float) blockSize);

  /* Compute the variance */
  *pResult = ((sumOfSquares - squareOfSum) / (float) (blockSize - 1.0f));

#endif /* #ifndef ARM_MATH_CM0_FAMILY */

}
#endif // __CC_ARM

/* 默认排序过 */
float Quantile(const float* data, uint32_t data_length, float q) {
  float qIndexRight = 1.0 + (data_length - 1.0) * q;
  float qIndexLeft = floor(qIndexRight);
  float fraction = qIndexRight - qIndexLeft;
  uint32_t qIndex = (uint32_t)qIndexLeft;
  float quantile = data[qIndex - 1u] + (data[qIndex] - data[qIndex - 1u]) * fraction;
  return quantile;
}

float CoefVariation(uint8_t *data, uint32_t data_len)
{
  /* STD and Mean. */
  float32_t std;
  breath_arm_var_f32(data, data_len, &std);
  std = sqrtf(std);

  /* What if mean close to 0. */
  float mean = Mean(data, data_len);

  float cv = std / mean;

  return cv;
}


int CmpValue(const void *a, const void *b)
{
  if (((value2index*)a)->value - ((value2index*)b)->value > 0)
  {
    return 1;
  }
  else
  {
    return -1;
  }
}


int CmpIndex(const void *a, const void *b)
{
    return ((value2index*)a)->index - ((value2index*)b)->index;
}


int CmpFunc(const void* a, const void* b) {
  return (*(float*)a > *(float*)b) ? 1 : -1;
}


value2index* GetV2I(void)
{
  return v2i;
}

float* GetMem(void)
{
  return _mem_pool;
}