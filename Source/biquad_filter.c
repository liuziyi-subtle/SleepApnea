#include "biquad_filter.h"

/**
* @details
* @brief  Initialization function for the floating-point Biquad cascade filter.
* @param[in,out] *S           points to an instance of the floating-point Biquad cascade structure.
* @param[in]     numStages    number of 2nd order stages in the filter.
* @param[in]     *pCoeffs     points to the filter coefficients array.
* @param[in]     *pState      points to the state array.
* @return        none
*
*
* <b>Coefficient and State Ordering:</b>
*
* \par
* The coefficients are stored in the array <code>pCoeffs</code> in the following order:
* <pre>
*     {b10, b11, b12, a11, a12, b20, b21, b22, a21, a22, ...}
* </pre>
*
* \par
* where <code>b1x</code> and <code>a1x</code> are the coefficients for the first stage,
* <code>b2x</code> and <code>a2x</code> are the coefficients for the second stage,
* and so on.  The <code>pCoeffs</code> array contains a total of <code>5*numStages</code> values.
*
* \par
* The <code>pState</code> is a pointer to state array.
* Each Biquad stage has 4 state variables <code>x[n-1], x[n-2], y[n-1],</code> and <code>y[n-2]</code>.
* The state variables are arranged in the <code>pState</code> array as:
* <pre>
*     {x[n-1], x[n-2], y[n-1], y[n-2]}
* </pre>
* The 4 state variables for stage 1 are first, then the 4 state variables for stage 2, and so on.
* The state array has a total length of <code>4*numStages</code> values.
* The state variables are updated after each block of data is processed; the coefficients are untouched.
*
*/
void biquad_cascade_df1_init_f32(
        biquad_casd_df1_inst_f32 * S,
        uint8_t numStages,
        double * pCoeffs,
        double * pState)
{
    /* Assign filter stages */
    S->numStages = numStages;
    /* Assign coefficient pointer */
    S->pCoeffs = pCoeffs;
    /* Clear state buffer and size is always 4 * numStages */
    memset(pState, 0, (4u * (uint32_t)numStages) * sizeof(double));
    /* Assign state pointer */
    S->pState = pState;
}
/**
* @param[in]  *S         points to an instance of the floating-point Biquad cascade structure.
* @param[in]  *pSrc      points to the block of input data.
* @param[out] *pDst      points to the block of output data.
* @param[in]  blockSize  number of samples to process per call.
* @return     none.
*
*/
void biquad_cascade_df1_f32(
        const biquad_casd_df1_inst_f32 * S,
        double * pSrc,
        double * pDst,
        uint32_t blockSize)
{
    double *pIn = pSrc;                         /*  source pointer            */
    double *pOut = pDst;                        /*  destination pointer       */
    double *pState = S->pState;                 /*  pState pointer            */
    double *pCoeffs = S->pCoeffs;               /*  coefficient pointer       */
    double acc;                                 /*  Simulates the accumulator */
    double b0, b1, b2, a1, a2;                  /*  Filter coefficients       */
    double Xn1, Xn2, Yn1, Yn2;                  /*  Filter pState variables   */
    double Xn;                                  /*  temporary input           */
    uint32_t sample, stage = S->numStages;     /*   loop counters             */
    do
    {
        /* Reading the coefficients */
        b0 = *pCoeffs++;
        b1 = *pCoeffs++;
        b2 = *pCoeffs++;
        a1 = *pCoeffs++;
        a2 = *pCoeffs++;
        /* Reading the pState values */
        Xn1 = pState[0];
        Xn2 = pState[1];
        Yn1 = pState[2];
        Yn2 = pState[3];
        /* Apply loop unrolling and compute 4 output values simultaneously. */
        /*      The variable acc hold output values that are being computed:
        *
        *    acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1]   + a2 * y[n-2]
        *    acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1]   + a2 * y[n-2]
        *    acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1]   + a2 * y[n-2]
        *    acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1]   + a2 * y[n-2]
        */
        sample = blockSize >> 2u;
        /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
        ** a second loop below computes the remaining 1 to 3 samples. */
        while (sample > 0u)
        {
            /* Read the first input */
            Xn = *pIn++;
            /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
            // printf("b0: %f, Xn: %f, b1: %f, Xn1: %f, b2: %f, Xn2: %f, a1: %f, Yn1: %f, a2: %f, Yn2: %f\n", b0, Xn, b1, Xn1, b2, Xn2, a1, Yn1, a2, Yn2);
            Yn2 = (b0 * Xn) + (b1 * Xn1) + (b2 * Xn2) + (a1 * Yn1) + (a2 * Yn2);
            // printf("Yn2: %f\n", Yn2);
            /* Store the result in the accumulator in the destination buffer. */
            *pOut++ = Yn2;
            /* Every time after the output is computed state should be updated. */
            /* The states should be updated as:  */
            /* Xn2 = Xn1    */
            /* Xn1 = Xn     */
            /* Yn2 = Yn1    */
            /* Yn1 = acc   */
            /* Read the second input */
            Xn2 = *pIn++;
            /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
            Yn1 = (b0 * Xn2) + (b1 * Xn) + (b2 * Xn1) + (a1 * Yn2) + (a2 * Yn1);
            /* Store the result in the accumulator in the destination buffer. */
            *pOut++ = Yn1;
            /* Every time after the output is computed state should be updated. */
            /* The states should be updated as:  */
            /* Xn2 = Xn1    */
            /* Xn1 = Xn     */
            /* Yn2 = Yn1    */
            /* Yn1 = acc   */
            /* Read the third input */
            Xn1 = *pIn++;
            /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
            Yn2 = (b0 * Xn1) + (b1 * Xn2) + (b2 * Xn) + (a1 * Yn1) + (a2 * Yn2);
            /* Store the result in the accumulator in the destination buffer. */
            *pOut++ = Yn2;
            /* Every time after the output is computed state should be updated. */
            /* The states should be updated as: */
            /* Xn2 = Xn1    */
            /* Xn1 = Xn     */
            /* Yn2 = Yn1    */
            /* Yn1 = acc   */
            /* Read the forth input */
            Xn = *pIn++;
            /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
            Yn1 = (b0 * Xn) + (b1 * Xn1) + (b2 * Xn2) + (a1 * Yn2) + (a2 * Yn1);
            /* Store the result in the accumulator in the destination buffer. */
            *pOut++ = Yn1;
            /* Every time after the output is computed state should be updated. */
            /* The states should be updated as:  */
            /* Xn2 = Xn1    */
            /* Xn1 = Xn     */
            /* Yn2 = Yn1    */
            /* Yn1 = acc   */
            Xn2 = Xn1;
            Xn1 = Xn;
            /* decrement the loop counter */
            sample--;
        }
        /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
        ** No loop unrolling is used. */
        sample = blockSize & 0x3u;
        while (sample > 0u)
        {
            /* Read the input */
            Xn = *pIn++;
            /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */
            acc = (b0 * Xn) + (b1 * Xn1) + (b2 * Xn2) + (a1 * Yn1) + (a2 * Yn2);
            /* Store the result in the accumulator in the destination buffer. */
            *pOut++ = acc;
            /* Every time after the output is computed state should be updated. */
            /* The states should be updated as:    */
            /* Xn2 = Xn1    */
            /* Xn1 = Xn     */
            /* Yn2 = Yn1    */
            /* Yn1 = acc   */
            Xn2 = Xn1;
            Xn1 = Xn;
            Yn2 = Yn1;
            Yn1 = acc;
            /* decrement the loop counter */
            sample--;
        }
        /*  Store the updated state variables back into the pState array */
        *pState++ = Xn1;
        *pState++ = Xn2;
        *pState++ = Yn1;
        *pState++ = Yn2;
        /*  The first stage goes from the input buffer to the output buffer. */
        /*  Subsequent numStages  occur in-place in the output buffer */
        pIn = pDst;
        /* Reset the output pointer */
        pOut = pDst;
        /* decrement the loop counter */
        stage--;
    } while (stage > 0u);
}
