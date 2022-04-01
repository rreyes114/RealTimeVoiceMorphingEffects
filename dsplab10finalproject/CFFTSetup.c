/*
 * CCFTSetup.c
 *
 *  Created on: Dec 2, 2021
 *      Author: Roehl
 */

#include "fpu_cfft.h"
#include "math.h"
#include "examples_setup.h"
#include "CFFTSetup.h"

//*****************************************************************************
// globals
//*****************************************************************************

#pragma DATA_SECTION(CFFTin1Buff,"CFFTdata1") //"ramgs0")//
float CFFTin1Buff[CFFT_SIZE*2];

#pragma DATA_SECTION(CFFTin2Buff,"CFFTdata2")
float CFFTin2Buff[CFFT_SIZE*2];

#pragma DATA_SECTION(CFFTin3Buff,"CFFTdata3") //"ramgs0")//
float CFFTin3Buff[CFFT_SIZE*2];

#pragma DATA_SECTION(CFFTin4Buff,"CFFTdata4")
float CFFTin4Buff[CFFT_SIZE*2];

#pragma DATA_SECTION(CFFToutBuff,"CFFTdata5")
float CFFToutBuff[CFFT_SIZE*2];

#pragma DATA_SECTION(CFFTF32Coef,"CFFTdata6")
float CFFTF32Coef[CFFT_SIZE];

//! \brief Object of the structure CFFT_F32_STRUCT
//!
CFFT_F32_STRUCT cfft;

//! \brief Handle to the CFFT_F32_STRUCT object
//!
CFFT_F32_STRUCT_Handle hnd_cfft = &cfft;

#ifdef USE_TABLES
//Linker defined variables
extern uint16_t  FFTTwiddlesRunStart;
extern uint16_t  FFTTwiddlesLoadStart;
extern uint16_t  FFTTwiddlesLoadSize;
#endif //USE_TABLES

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

    //FPU_initSystemClocks();

    //FPU_initEpie();

    // Clear input buffers:
    for(i=0; i < (CFFT_SIZE*2); i=i+2){
        CFFTin1Buff[i] = 0.0f;
        CFFTin1Buff[i+1] = 0.0f;
        CFFToutBuff[i] = 0.0f;
        CFFToutBuff[i+1] = 0.0f;
    }

    //
    // Off-Place Algorithm
    //
    // Note: In this version, CFFTin1Buff and CFFToutBuff are used in
    //       ping-pong fashion. The input data is first stored in CFFTin1Buff
    //       where the FFT, including bit reversed ordering, is initially done.
    //       At each successive stage of the FFT the cfft.CurrentInPtr pointer
    //       will point to the buffer that is the input for that stage. In this
    //       manner the "CurrentInPtr" and "CurrentOutPtr" are exchanged at the
    //       start of each stage. Depending on the number of FFT stages, the
    //       final output will be in either CFFTin1Buff (#Stages is odd) or
    //       CFFToutBuff (#stages is even).
    //Input/output or middle stage of ping-pong buffer
    hnd_cfft->InPtr   = CFFTin1Buff;
    //Output or middle stage of ping-pong buffer
    hnd_cfft->OutPtr  = CFFToutBuff;
    hnd_cfft->Stages  = CFFT_STAGES;  // FFT stages
    hnd_cfft->FFTSize = CFFT_SIZE;    // FFT size
//#ifdef USE_TABLES
    hnd_cfft->CoefPtr = CFFT_f32_twiddleFactors;  //Twiddle factor table
//#else
//    hnd_cfft->CoefPtr = CFFTF32Coef;  //Twiddle factor table
//    CFFT_f32_sincostable(hnd_cfft);   // Calculate twiddle factor
//#endif //USE_TABLES

}
