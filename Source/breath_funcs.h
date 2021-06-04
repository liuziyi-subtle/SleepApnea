#include <stdint.h>
typedef float float32_t;

#define BREATH_FLT_MAX __FLT_MAX__
#define BREATH_FLT_MIN __FLT_MIN__
#define BREATH_INT32_MAX __INT32_MAX__
#define BREATH_INT32_MIN __INT32_MIN__

struct bpv
{
    uint8_t type;   /*<< true -> peak, false -> valley. */
    int pos;        /*<< sampling index. */
    float val;      /*<< amplitude. */
};

typedef struct _value2index 
{
  float value;
  int32_t index;
} value2index;


float32_t Mean(const float32_t* data, uint32_t data_length);
void FindMax(float32_t * data, uint32_t data_len, float32_t* maxval, uint32_t * maxval_index);
uint32_t SpktWelchDensity(float32_t* data, uint32_t data_len, float32_t* results);
// uint8_t FindPeaks(const float *array, int32_t L, int32_t i, int16_t radius, int32_t minPH, int32_t maxPH);
float FindPeak(float datum, int32_t *left_base, int32_t *right_base, uint8_t init);
struct bpv MakeBPV(uint8_t type, int pos, float val);
float Quantile(const float* data, uint32_t data_length, float q);
int CmpValue(const void *a, const void *b);
int CmpIndex(const void *a, const void *b);
int CmpFunc(const void* a, const void* b);
value2index* GetV2I(void);
void arm_var_f32(float * pSrc, uint32_t blockSize, float * pResult);
float* GetMem(void);
float CoefVariation(uint8_t *data, uint32_t data_len);