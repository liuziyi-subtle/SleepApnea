#include "breath_fft.h"
#include "breath_funcs.h"
#include <math.h>
#include <stdio.h>
#include <float.h>

#if DEBUG
#include "debug.h"
#endif

#define BREATH_HANN_SCALE (0.041666666666666664353702032031)

const static float64_t _hann_window[64] = 
{
    0.000000000000000000000000000000, 0.002407636663901591145275915551,
    0.009607359798384784710378880845, 0.021529832133895587809035987448,
    0.038060233744356630758431947470, 0.059039367825822530733148596482,
    0.084265193848727326653147429170, 0.113494773318631503311593178296,
    0.146446609406726269142495766573, 0.182803357918177367125167620543,
    0.222214883490199022197941758350, 0.264301631587001151224569639453,
    0.308658283817455136865959275383, 0.354857661372768862229065689462,
    0.402454838991935903713681454974, 0.450991429835219670430035421305,
    0.500000000000000000000000000000, 0.549008570164780440592267041211,
    0.597545161008064207308621007542, 0.645142338627231137770934310538,
    0.691341716182544918645191955875, 0.735698368412998959797732823063,
    0.777785116509801088824360704166, 0.817196642081822743897134841973,
    0.853553390593273730857504233427, 0.886505226681368552199558052962,
    0.915734806151272673346852570830, 0.940960632174177469266851403518,
    0.961939766255643369241568052530, 0.978470167866104523213266475068,
    0.990392640201615215289621119155, 0.997592363336098464365875315707,
    1.000000000000000000000000000000, 0.997592363336098464365875315707,
    0.990392640201615215289621119155, 0.978470167866104523213266475068,
    0.961939766255643369241568052530, 0.940960632174177469266851403518,
    0.915734806151272673346852570830, 0.886505226681368552199558052962,
    0.853553390593273730857504233427, 0.817196642081822854919437304488,
    0.777785116509801088824360704166, 0.735698368412998959797732823063,
    0.691341716182545140689796880906, 0.645142338627231137770934310538,
    0.597545161008064207308621007542, 0.549008570164780218547662116180,
    0.500000000000000000000000000000, 0.450991429835219892474640346336,
    0.402454838991935792691378992458, 0.354857661372768862229065689462,
    0.308658283817454970332505581609, 0.264301631587001151224569639453,
    0.222214883490199022197941758350, 0.182803357918177145080562695512,
    0.146446609406726269142495766573, 0.113494773318631669845046872069,
    0.084265193848727326653147429170, 0.059039367825822530733148596482,
    0.038060233744356741780734409986, 0.021529832133895587809035987448,
    0.009607359798384784710378880845, 0.002407636663901535634124684293
};

static float64_t _mem_pool[1024];  // 32768 Bytes

float64_t Mean(const float64_t* data, uint32_t data_len) {
  uint32_t count;      /*<< loop counter */
  float64_t sum = 0.0; /*<< temporary result storage */

  /* Initialize counter with number of samples */
  count = data_len;
  while (count > 0U) {
    sum += *data++;
    count--;
  }

  float64_t mean = (sum / data_len);

  return mean;
}

void _SortFunc(float64_t* data, uint32_t data_length) {
  float64_t temp;
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

void FindMax(float64_t * data, uint32_t data_len, float64_t* maxval,
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

uint32_t SpktWelchDensity(float64_t* data, uint32_t data_len, float64_t* results) {
  float64_t mean = Mean(data, data_len);
  float64_t* temp_data = _mem_pool;
  // float64_t* results = _mem_pool + data_len;

  uint32_t i;
  for (i = 0; i < data_len; ++i) {
    temp_data[i] = (data[i] - mean) * _hann_window[i];
  }

  arm_rfft_fast_instance_f64 S;
  arm_rfft_64_fast_init_f64(&S);
  arm_rfft_fast_f64(&S, temp_data, results, 0);

  for (i = 0; i < data_len; i += 2) {
    results[i] = (pow(results[i], 2) + pow(results[i + 1], 2)) * BREATH_HANN_SCALE;
    // printf("results[%u]: %f\n", i, results[i]);
    // printf("results[%u]: %f\n", i + 1, results[i + 1]);
  }

  float64_t* p = &results[1];
  float64_t* q = p + 1;
  while (p != &results[(int)(data_len / 2)]) {
    *p = *q;
    *p = (*p) * 2;
    q += 2;
    p++;
  }

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
void arm_var_f32(
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