/*
 * RFFTandICFFT.c
 *
 *  Created on: Dec 4, 2021
 *      Author: Roehl
 */

#include "fpu_rfft.h"
#include "fpu_cfft.h"
#include "examples_setup.h"
#include "RFFTandICFFT.h"

#pragma DATA_SECTION(RFFTin0Buff,"ramgs0");//"RFFTdata0")
float RFFTin0Buff[RFFT_SIZE];

#pragma DATA_SECTION(RFFTin1Buff,"ramgs0");//"RFFTdata1")
float RFFTin1Buff[RFFT_SIZE];

#pragma DATA_SECTION(RFFTmagBuff,"ramgs1");//"RFFTdata2")
float RFFTmagBuff[RFFT_SIZE/2+1];

#pragma DATA_SECTION(RFFTphsBuff,"ramgs1");//"RFFTdata2")
float RFFTphsBuff[RFFT_SIZE/2+1];

#pragma DATA_SECTION(RFFToutBuff,"ramgs0");//"RFFTdata3")
float RFFToutBuff[RFFT_SIZE];

#pragma DATA_SECTION(RFFTF32Coef,"ramgs0");//"RFFTdata4")
float RFFTF32Coef[RFFT_SIZE];
RFFT_F32_STRUCT rfft;
RFFT_F32_STRUCT_Handle hnd_rfft = &rfft;

#pragma DATA_SECTION(CFFToutBuff, "ramgs1");
float CFFToutBuff[CFFT_SIZE*2];
#pragma DATA_SECTION(CFFTF32Coef,"ramgs1")//"CFFTdata4")
float CFFTF32Coef[CFFT_SIZE];
CFFT_F32_STRUCT cfft;
CFFT_F32_STRUCT_Handle hnd_cfft = &cfft;

extern uint16_t  FFTTwiddlesRunStart;
extern uint16_t  FFTTwiddlesLoadStart;
extern uint16_t  FFTTwiddlesLoadSize;

void RFFT_Init() {
    // Locals
    uint16_t i;
#ifdef FLASH
    EALLOW;
    Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0;
    memcpy((uint32_t *)&RamfuncsRunStart, (uint32_t *)&RamfuncsLoadStart,
            (uint32_t)&RamfuncsLoadSize );
    FPU_initFlash();
#ifdef USE_TABLES
    memcpy((uint32_t *)&FFTTwiddlesRunStart, (uint32_t *)&FFTTwiddlesLoadStart,
            (uint32_t)&FFTTwiddlesLoadSize );
#endif //USE_TABLES
#endif //FLASH

    // Clear input buffers:
    for(i=0; i < RFFT_SIZE; i++){
        RFFTin1Buff[i] = 0.0f;
    }

    hnd_rfft->FFTSize   = RFFT_SIZE;
    hnd_rfft->FFTStages = RFFT_STAGES;
    hnd_rfft->InBuf     = &RFFTin1Buff[0];  //Input buffer
    hnd_rfft->OutBuf    = &RFFToutBuff[0];  //Output buffer

    hnd_rfft->MagBuf    = &RFFTmagBuff[0];  //Magnitude buffer
    hnd_rfft->PhaseBuf  = &RFFTphsBuff[0];
//#ifdef USE_TABLES
//    hnd_rfft->CosSinBuf = RFFT_f32_twiddleFactors;  //Twiddle factor buffer
//#else
    hnd_rfft->CosSinBuf = &RFFTF32Coef[0];  //Twiddle factor buffer
    RFFT_f32_sincostable(hnd_rfft);         //Calculate twiddle factor
//#endif //USE_TABLES


    for (i=0; i < RFFT_SIZE; i++){
          RFFToutBuff[i] = 0.0f;               //Clean up output buffer
    }

    for (i=0; i <= RFFT_SIZE/2; i++){
         RFFTmagBuff[i] = 0;                //Clean up magnitude buffer
    }
}

void CFFT_Init(void)
{
    // Locals
    uint16_t i;
#ifdef FLASH
    EALLOW;
    Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0;
    memcpy((uint32_t *)&RamfuncsRunStart, (uint32_t *)&RamfuncsLoadStart,
            (uint32_t)&RamfuncsLoadSize );
    FPU_initFlash();
#ifdef USE_TABLES
    memcpy((uint32_t *)&FFTTwiddlesRunStart, (uint32_t *)&FFTTwiddlesLoadStart,
            (uint32_t)&FFTTwiddlesLoadSize );
#endif //USE_TABLES
#endif //FLASH

    // Clear input buffers:
    for(i=0; i < (CFFT_SIZE*2); i=i+2){
        //CFFTin1Buff[i] = 0.0f;
        //CFFTin1Buff[i+1] = 0.0f;
        CFFToutBuff[i] = 0.0f;
        CFFToutBuff[i+1] = 0.0f;
    }

    //Input/output or middle stage of ping-pong buffer
    hnd_cfft->InPtr   = RFFToutBuff;
    //Output or middle stage of ping-pong buffer
    hnd_cfft->OutPtr  = CFFToutBuff;
    hnd_cfft->Stages  = CFFT_STAGES;  // FFT stages
    hnd_cfft->FFTSize = CFFT_SIZE;    // FFT size
//#ifdef USE_TABLES
//    hnd_cfft->CoefPtr = CFFT_f32_twiddleFactors;  //Twiddle factor table
//#else
    hnd_cfft->CoefPtr = CFFTF32Coef;  //Twiddle factor table
    CFFT_f32_sincostable(hnd_cfft);   // Calculate twiddle factor
//#endif //USE_TABLES

}
