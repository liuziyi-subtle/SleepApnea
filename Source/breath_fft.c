#include "breath_fft.h"

#define ARMBITREVINDEXTABLEF64_32_TABLE_LENGTH ((uint16_t)24)

const uint16_t armBitRevIndexTableF64_32[ARMBITREVINDEXTABLEF64_32_TABLE_LENGTH] = 
{
  /* 4x2, size 24 */
  8,  128, 16, 64,  24,  192, 40,  160, 48,  96,  56,  224,
  72, 144, 88, 208, 104, 176, 120, 240, 152, 200, 184, 232
};

const uint64_t twiddleCoefF64_32[64] =
{
  0x3ff0000000000000, 0x0000000000000000,  //       1,       0'
  0x3fef6297cff75cb0, 0x3fc8f8b83c69a60a,  // 0.98079, 0.19509'
  0x3fed906bcf328d46, 0x3fd87de2a6aea963,  // 0.92388, 0.38268'
  0x3fea9b66290ea1a3, 0x3fe1c73b39ae68c8,  // 0.83147, 0.55557'
  0x3fe6a09e667f3bcc, 0x3fe6a09e667f3bcc,  // 0.70711, 0.70711'
  0x3fe1c73b39ae68c8, 0x3fea9b66290ea1a3,  // 0.55557, 0.83147'
  0x3fd87de2a6aea963, 0x3fed906bcf328d46,  // 0.38268, 0.92388'
  0x3fc8f8b83c69a60a, 0x3fef6297cff75cb0,  // 0.19509, 0.98079'
  0x0000000000000000, 0x3ff0000000000000,  //       0,       1'
  0xbfc8f8b83c69a60a, 0x3fef6297cff75cb0,  //-0.19509, 0.98079'
  0xbfd87de2a6aea963, 0x3fed906bcf328d46,  //-0.38268, 0.92388'
  0xbfe1c73b39ae68c8, 0x3fea9b66290ea1a3,  //-0.55557, 0.83147'
  0xbfe6a09e667f3bcc, 0x3fe6a09e667f3bcc,  //-0.70711, 0.70711'
  0xbfea9b66290ea1a3, 0x3fe1c73b39ae68c8,  //-0.83147, 0.55557'
  0xbfed906bcf328d46, 0x3fd87de2a6aea963,  //-0.92388, 0.38268'
  0xbfef6297cff75cb0, 0x3fc8f8b83c69a60a,  //-0.98079, 0.19509'
  0xbff0000000000000, 0x0000000000000000,  //      -1,       0'
  0xbfef6297cff75cb0, 0xbfc8f8b83c69a60a,  //-0.98079,-0.19509'
  0xbfed906bcf328d46, 0xbfd87de2a6aea963,  //-0.92388,-0.38268'
  0xbfea9b66290ea1a3, 0xbfe1c73b39ae68c8,  //-0.83147,-0.55557'
  0xbfe6a09e667f3bcc, 0xbfe6a09e667f3bcc,  //-0.70711,-0.70711'
  0xbfe1c73b39ae68c8, 0xbfea9b66290ea1a3,  //-0.55557,-0.83147'
  0xbfd87de2a6aea963, 0xbfed906bcf328d46,  //-0.38268,-0.92388'
  0xbfc8f8b83c69a60a, 0xbfef6297cff75cb0,  //-0.19509,-0.98079'
  0x0000000000000000, 0xbff0000000000000,  //       0,      -1'
  0x3fc8f8b83c69a60a, 0xbfef6297cff75cb0,  // 0.19509,-0.98079'
  0x3fd87de2a6aea963, 0xbfed906bcf328d46,  // 0.38268,-0.92388'
  0x3fe1c73b39ae68c8, 0xbfea9b66290ea1a3,  // 0.55557,-0.83147'
  0x3fe6a09e667f3bcc, 0xbfe6a09e667f3bcc,  // 0.70711,-0.70711'
  0x3fea9b66290ea1a3, 0xbfe1c73b39ae68c8,  // 0.83147,-0.55557'
  0x3fed906bcf328d46, 0xbfd87de2a6aea963,  // 0.92388,-0.38268'
  0x3fef6297cff75cb0, 0xbfc8f8b83c69a60a,  // 0.98079,-0.19509'
};

const uint64_t twiddleCoefF64_rfft_64[64] = {
  0x0000000000000000, 0x3ff0000000000000,  //       0,        1'
  0x3fb917a6bc29b42c, 0x3fefd88da3d12526,  // 0.098017,  0.99518'
  0x3fc8f8b83c69a60a, 0x3fef6297cff75cb0,  // 0.19509,  0.98079'
  0x3fd294062ed59f05, 0x3fee9f4156c62dda,  // 0.29028,  0.95694'
  0x3fd87de2a6aea963, 0x3fed906bcf328d46,  // 0.38268,  0.92388'
  0x3fde2b5d3806f63b, 0x3fec38b2f180bdb1,  //  0.4714,  0.88192'
  0x3fe1c73b39ae68c8, 0x3fea9b66290ea1a3,  // 0.55557,  0.83147'
  0x3fe44cf325091dd6, 0x3fe8bc806b151741,  // 0.63439,  0.77301'
  0x3fe6a09e667f3bcc, 0x3fe6a09e667f3bcc,  // 0.70711,  0.70711'
  0x3fe8bc806b151741, 0x3fe44cf325091dd6,  // 0.77301,  0.63439'
  0x3fea9b66290ea1a3, 0x3fe1c73b39ae68c8,  // 0.83147,  0.55557'
  0x3fec38b2f180bdb1, 0x3fde2b5d3806f63b,  // 0.88192,   0.4714'
  0x3fed906bcf328d46, 0x3fd87de2a6aea963,  // 0.92388,  0.38268'
  0x3fee9f4156c62dda, 0x3fd294062ed59f05,  // 0.95694,  0.29028'
  0x3fef6297cff75cb0, 0x3fc8f8b83c69a60a,  // 0.98079,  0.19509'
  0x3fefd88da3d12526, 0x3fb917a6bc29b42c,  // 0.99518, 0.098017'
  0x3ff0000000000000, 0x0000000000000000,  //       1,        0'
  0x3fefd88da3d12526, 0xbfb917a6bc29b42c,  // 0.99518,-0.098017'
  0x3fef6297cff75cb0, 0xbfc8f8b83c69a60a,  // 0.98079, -0.19509'
  0x3fee9f4156c62dda, 0xbfd294062ed59f05,  // 0.95694, -0.29028'
  0x3fed906bcf328d46, 0xbfd87de2a6aea963,  // 0.92388, -0.38268'
  0x3fec38b2f180bdb1, 0xbfde2b5d3806f63b,  // 0.88192,  -0.4714'
  0x3fea9b66290ea1a3, 0xbfe1c73b39ae68c8,  // 0.83147, -0.55557'
  0x3fe8bc806b151741, 0xbfe44cf325091dd6,  // 0.77301, -0.63439'
  0x3fe6a09e667f3bcc, 0xbfe6a09e667f3bcc,  // 0.70711, -0.70711'
  0x3fe44cf325091dd6, 0xbfe8bc806b151741,  // 0.63439, -0.77301'
  0x3fe1c73b39ae68c8, 0xbfea9b66290ea1a3,  // 0.55557, -0.83147'
  0x3fde2b5d3806f63b, 0xbfec38b2f180bdb1,  //  0.4714, -0.88192'
  0x3fd87de2a6aea963, 0xbfed906bcf328d46,  // 0.38268, -0.92388'
  0x3fd294062ed59f05, 0xbfee9f4156c62dda,  // 0.29028, -0.95694'
  0x3fc8f8b83c69a60a, 0xbfef6297cff75cb0,  // 0.19509, -0.98079'
  0x3fb917a6bc29b42c, 0xbfefd88da3d12526,  // 0.098017, -0.99518'
};

arm_status arm_rfft_64_fast_init_f64(arm_rfft_fast_instance_f64 *S)
{
  arm_cfft_instance_f64 *Sint;

  if (!S) return ARM_MATH_ARGUMENT_ERROR;

  Sint = &(S->Sint);
  Sint->fftLen = 32U;
  S->fftLenRFFT = 64U;

  Sint->bitRevLength = ARMBITREVINDEXTABLEF64_32_TABLE_LENGTH;
  Sint->pBitRevTable = (uint16_t *)armBitRevIndexTableF64_32;
  Sint->pTwiddle = (float64_t *)twiddleCoefF64_32;
  S->pTwiddleRFFT = (float64_t *)twiddleCoefF64_rfft_64;

  return ARM_MATH_SUCCESS;
}



void stage_rfft_f64(const arm_rfft_fast_instance_f64 *S,
                    float64_t *p,
                    float64_t *pOut)
{
  uint32_t k;         /* Loop Counter */
  float64_t twR, twI; /* RFFT Twiddle coefficients */
  const float64_t *pCoeff =
      S->pTwiddleRFFT;          /* Points to RFFT Twiddle factors */
  float64_t *pA = p;            /* increasing pointer */
  float64_t *pB = p;            /* decreasing pointer */
  float64_t xAR, xAI, xBR, xBI; /* temporary variables */
  float64_t t1a, t1b;           /* temporary variables */
  float64_t p0, p1, p2, p3;     /* temporary variables */

  k = (S->Sint).fftLen - 1;

  /* Pack first and last sample of the frequency domain together */

  xBR = pB[0];
  xBI = pB[1];
  xAR = pA[0];
  xAI = pA[1];

  twR = *pCoeff++;
  twI = *pCoeff++;

  // U1 = XA(1) + XB(1); % It is real
  t1a = xBR + xAR;

  // U2 = XB(1) - XA(1); % It is imaginary
  t1b = xBI + xAI;

  // real(tw * (xB - xA)) = twR * (xBR - xAR) - twI * (xBI - xAI);
  // imag(tw * (xB - xA)) = twI * (xBR - xAR) + twR * (xBI - xAI);
  *pOut++ = 0.5 * (t1a + t1b);
  *pOut++ = 0.5 * (t1a - t1b);

  // XA(1) = 1/2*( U1 - imag(U2) +  i*( U1 +imag(U2) ));
  pB = p + 2 * k;
  pA += 2;

  do {
    /*
       function X = my_split_rfft(X, ifftFlag)
       % X is a series of real numbers
       L  = length(X);
       XC = X(1:2:end) +i*X(2:2:end);
       XA = fft(XC);
       XB = conj(XA([1 end:-1:2]));
       TW = i*exp(-2*pi*i*[0:L/2-1]/L).';
       for l = 2:L/2
          XA(l) = 1/2 * (XA(l) + XB(l) + TW(l) * (XB(l) - XA(l)));
       end
       XA(1) = 1/2* (XA(1) + XB(1) + TW(1) * (XB(1) - XA(1))) + i*( 1/2*( XA(1)
       + XB(1) + i*( XA(1) - XB(1)))); X = XA;
    */

    xBI = pB[1];
    xBR = pB[0];
    xAR = pA[0];
    xAI = pA[1];

    twR = *pCoeff++;
    twI = *pCoeff++;

    t1a = xBR - xAR;
    t1b = xBI + xAI;

    // real(tw * (xB - xA)) = twR * (xBR - xAR) - twI * (xBI - xAI);
    // imag(tw * (xB - xA)) = twI * (xBR - xAR) + twR * (xBI - xAI);
    p0 = twR * t1a;
    p1 = twI * t1a;
    p2 = twR * t1b;
    p3 = twI * t1b;

    *pOut++ = 0.5 * (xAR + xBR + p0 + p3);  // xAR
    *pOut++ = 0.5 * (xAI - xBI + p1 - p2);  // xAI

    pA += 2;
    pB -= 2;
    k--;
  } while (k > 0U);
}


/* Prepares data for inverse cfft */
void merge_rfft_f64(const arm_rfft_fast_instance_f64 *S,
                    float64_t *p,
                    float64_t *pOut)
{
  uint32_t k;         /* Loop Counter */
  float64_t twR, twI; /* RFFT Twiddle coefficients */
  const float64_t *pCoeff =
      S->pTwiddleRFFT;            /* Points to RFFT Twiddle factors */
  float64_t *pA = p;              /* increasing pointer */
  float64_t *pB = p;              /* decreasing pointer */
  float64_t xAR, xAI, xBR, xBI;   /* temporary variables */
  float64_t t1a, t1b, r, s, t, u; /* temporary variables */

  k = (S->Sint).fftLen - 1;

  xAR = pA[0];
  xAI = pA[1];

  pCoeff += 2;

  *pOut++ = 0.5 * (xAR + xAI);
  *pOut++ = 0.5 * (xAR - xAI);

  pB = p + 2 * k;
  pA += 2;

  while (k > 0U) {
    /* G is half of the frequency complex spectrum */
    // for k = 2:N
    //    Xk(k) = 1/2 * (G(k) + conj(G(N-k+2)) + Tw(k)*( G(k) -
    //    conj(G(N-k+2))));
    xBI = pB[1];
    xBR = pB[0];
    xAR = pA[0];
    xAI = pA[1];

    twR = *pCoeff++;
    twI = *pCoeff++;

    t1a = xAR - xBR;
    t1b = xAI + xBI;

    r = twR * t1a;
    s = twI * t1b;
    t = twI * t1a;
    u = twR * t1b;

    // real(tw * (xA - xB)) = twR * (xAR - xBR) - twI * (xAI - xBI);
    // imag(tw * (xA - xB)) = twI * (xAR - xBR) + twR * (xAI - xBI);
    *pOut++ = 0.5 * (xAR + xBR - r - s);  // xAR
    *pOut++ = 0.5 * (xAI - xBI + t - u);  // xAI

    pA += 2;
    pB -= 2;
    k--;
  }
}


void arm_rfft_fast_f64(arm_rfft_fast_instance_f64 *S,
                       float64_t *p, float64_t *pOut,
                       uint8_t ifftFlag)
{
  arm_cfft_instance_f64 *Sint = &(S->Sint);
  Sint->fftLen = S->fftLenRFFT / 2;

  /* Calculation of Real FFT */
  if (ifftFlag) {
    /*  Real FFT compression */
    merge_rfft_f64(S, p, pOut);

    /* Complex radix-4 IFFT process */
    arm_cfft_f64(Sint, pOut, ifftFlag, 1);
  } else {
    /* Calculation of RFFT of input */
    arm_cfft_f64(Sint, p, ifftFlag, 1);

    /*  Real FFT extraction */
    stage_rfft_f64(S, p, pOut);
  }
}


void arm_bitreversal_64(uint64_t *pSrc,
                        const uint16_t bitRevLen,
                        const uint16_t *pBitRevTab) {
  uint64_t a, b, i, tmp;

  for (i = 0; i < bitRevLen;) {
    a = pBitRevTab[i] >> 2;
    b = pBitRevTab[i + 1] >> 2;

    // real
    tmp = pSrc[a];
    pSrc[a] = pSrc[b];
    pSrc[b] = tmp;

    // complex
    tmp = pSrc[a + 1];
    pSrc[a + 1] = pSrc[b + 1];
    pSrc[b + 1] = tmp;

    i += 2;
  }
}


void arm_radix4_butterfly_f64(float64_t* pSrc, uint16_t fftLen,
                              const float64_t* pCoef,
                              uint16_t twidCoefModifier) {
  float64_t co1, co2, co3, si1, si2, si3;
  uint32_t ia1, ia2, ia3;
  uint32_t i0, i1, i2, i3;
  uint32_t n1, n2, j, k;

  float64_t t1, t2, r1, r2, s1, s2;

  /*  Initializations for the fft calculation */
  n2 = fftLen;
  n1 = n2;
  for (k = fftLen; k > 1U; k >>= 2U) {
    /*  Initializations for the fft calculation */
    n1 = n2;
    n2 >>= 2U;
    ia1 = 0U;

    /*  FFT Calculation */
    j = 0;
    do {
      /*  index calculation for the coefficients */
      ia2 = ia1 + ia1;
      ia3 = ia2 + ia1;
      co1 = pCoef[ia1 * 2U];
      si1 = pCoef[(ia1 * 2U) + 1U];
      co2 = pCoef[ia2 * 2U];
      si2 = pCoef[(ia2 * 2U) + 1U];
      co3 = pCoef[ia3 * 2U];
      si3 = pCoef[(ia3 * 2U) + 1U];

      /*  Twiddle coefficients index modifier */
      ia1 = ia1 + twidCoefModifier;

      i0 = j;
      do {
        /*  index calculation for the input as, */
        /*  pSrc[i0 + 0], pSrc[i0 + fftLen/4], pSrc[i0 + fftLen/2], pSrc[i0 +
         * 3fftLen/4] */
        i1 = i0 + n2;
        i2 = i1 + n2;
        i3 = i2 + n2;

        /* xa + xc */
        r1 = pSrc[(2U * i0)] + pSrc[(2U * i2)];

        /* xa - xc */
        r2 = pSrc[(2U * i0)] - pSrc[(2U * i2)];

        /* ya + yc */
        s1 = pSrc[(2U * i0) + 1U] + pSrc[(2U * i2) + 1U];

        /* ya - yc */
        s2 = pSrc[(2U * i0) + 1U] - pSrc[(2U * i2) + 1U];

        /* xb + xd */
        t1 = pSrc[2U * i1] + pSrc[2U * i3];

        /* xa' = xa + xb + xc + xd */
        pSrc[2U * i0] = r1 + t1;

        /* xa + xc -(xb + xd) */
        r1 = r1 - t1;

        /* yb + yd */
        t2 = pSrc[(2U * i1) + 1U] + pSrc[(2U * i3) + 1U];

        /* ya' = ya + yb + yc + yd */
        pSrc[(2U * i0) + 1U] = s1 + t2;

        /* (ya + yc) - (yb + yd) */
        s1 = s1 - t2;

        /* (yb - yd) */
        t1 = pSrc[(2U * i1) + 1U] - pSrc[(2U * i3) + 1U];

        /* (xb - xd) */
        t2 = pSrc[2U * i1] - pSrc[2U * i3];

        /* xc' = (xa-xb+xc-xd)co2 + (ya-yb+yc-yd)(si2) */
        pSrc[2U * i1] = (r1 * co2) + (s1 * si2);

        /* yc' = (ya-yb+yc-yd)co2 - (xa-xb+xc-xd)(si2) */
        pSrc[(2U * i1) + 1U] = (s1 * co2) - (r1 * si2);

        /* (xa - xc) + (yb - yd) */
        r1 = r2 + t1;

        /* (xa - xc) - (yb - yd) */
        r2 = r2 - t1;

        /* (ya - yc) -  (xb - xd) */
        s1 = s2 - t2;

        /* (ya - yc) +  (xb - xd) */
        s2 = s2 + t2;

        /* xb' = (xa+yb-xc-yd)co1 + (ya-xb-yc+xd)(si1) */
        pSrc[2U * i2] = (r1 * co1) + (s1 * si1);

        /* yb' = (ya-xb-yc+xd)co1 - (xa+yb-xc-yd)(si1) */
        pSrc[(2U * i2) + 1U] = (s1 * co1) - (r1 * si1);

        /* xd' = (xa-yb-xc+yd)co3 + (ya+xb-yc-xd)(si3) */
        pSrc[2U * i3] = (r2 * co3) + (s2 * si3);

        /* yd' = (ya+xb-yc-xd)co3 - (xa-yb-xc+yd)(si3) */
        pSrc[(2U * i3) + 1U] = (s2 * co3) - (r2 * si3);

        i0 += n1;
      } while (i0 < fftLen);
      j++;
    } while (j <= (n2 - 1U));
    twidCoefModifier <<= 2U;
  }
}


void arm_cfft_radix4by2_f64(float64_t* pSrc,
                            uint32_t fftLen,
                            const float64_t* pCoef)
{
  uint32_t i, l;
  uint32_t n2, ia;
  float64_t xt, yt, cosVal, sinVal;
  float64_t p0, p1, p2, p3, a0, a1;

  n2 = fftLen >> 1;
  ia = 0;
  for (i = 0; i < n2; i++) {
    cosVal = pCoef[2 * ia];
    sinVal = pCoef[2 * ia + 1];
    ia++;

    l = i + n2;

    /*  Butterfly implementation */
    a0 = pSrc[2 * i] + pSrc[2 * l];
    xt = pSrc[2 * i] - pSrc[2 * l];

    yt = pSrc[2 * i + 1] - pSrc[2 * l + 1];
    a1 = pSrc[2 * l + 1] + pSrc[2 * i + 1];

    p0 = xt * cosVal;
    p1 = yt * sinVal;
    p2 = yt * cosVal;
    p3 = xt * sinVal;

    pSrc[2 * i] = a0;
    pSrc[2 * i + 1] = a1;

    pSrc[2 * l] = p0 + p1;
    pSrc[2 * l + 1] = p2 - p3;
  }

  // first col
  arm_radix4_butterfly_f64(pSrc, n2, (float64_t*)pCoef, 2U);
  // second col
  arm_radix4_butterfly_f64(pSrc + fftLen, n2, (float64_t*)pCoef, 2U);
}


void arm_cfft_f64(const arm_cfft_instance_f64* S, float64_t* p1,
                  uint8_t ifftFlag, uint8_t bitReverseFlag)
{
  uint32_t L = S->fftLen, l;
  float64_t invL, *pSrc;

  if (ifftFlag == 1U) {
    /*  Conjugate input data  */
    pSrc = p1 + 1;
    for (l = 0; l < L; l++) {
      *pSrc = -*pSrc;
      pSrc += 2;
    }
  }

  switch (L) {
    case 16:
    case 64:
    case 256:
    case 1024:
    case 4096:
      arm_radix4_butterfly_f64(p1, L, (float64_t*)S->pTwiddle, 1U);
      break;

    case 32:
    case 128:
    case 512:
    case 2048:
      arm_cfft_radix4by2_f64(p1, L, (float64_t*)S->pTwiddle);  // 可以去掉
      break;
  }

  if (bitReverseFlag)
    arm_bitreversal_64((uint64_t*)p1, S->bitRevLength, S->pBitRevTable);

  if (ifftFlag == 1U) {
    invL = 1.0 / (float64_t)L;
    /*  Conjugate and scale output data */
    pSrc = p1;
    for (l = 0; l < L; l++) {
      *pSrc++ *= invL;
      *pSrc = -(*pSrc) * invL;
      pSrc++;
    }
  }
}
