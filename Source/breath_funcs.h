#include <stdint.h>
typedef double float64_t;

struct bpv
{
    uint8_t type;   /*<< true -> peak, false -> valley. */
    int pos;        /*<< sampling index. */
    float val;      /*<< amplitude. */
};

float64_t Mean(const float64_t* data, uint32_t data_length);
void FindMax(float64_t * data, uint32_t data_len, float64_t* maxval, uint32_t * maxval_index);
uint32_t SpktWelchDensity(float64_t* data, uint32_t data_length, float64_t* results);
// uint8_t FindPeaks(const float *array, int32_t L, int32_t i, int16_t radius, int32_t minPH, int32_t maxPH);
float FindPeak(float datum, int32_t *left_base, int32_t *right_base, uint8_t init);
struct bpv MakeBPV(uint8_t type, int pos, float val);