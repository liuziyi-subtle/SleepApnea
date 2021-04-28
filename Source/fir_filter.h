/**
* Copyright 2019 Liu Ziyi.
*/
#pragma once
#include <stdint.h>
#include <string.h>
#ifndef float32_t
#define float32_t   double
#endif  // !float32_t

typedef struct {
    uint16_t    numTaps;
    float32_t   *pState;
    float32_t   *pCoeffs;
} fir_instance_f32;
void fir_init_f32(
        fir_instance_f32 * S,
        uint16_t numTaps,
        float32_t * pCoeffs,
        float32_t * pState,
        uint32_t blockSize);
void fir_f32(
        const fir_instance_f32 * S,
        float32_t * pSrc,
        float32_t * pDst,
        uint32_t blockSize);
