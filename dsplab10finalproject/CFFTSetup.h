/*
 * CFFTSetup.h
 *
 *  Created on: Dec 2, 2021
 *      Author: Roehl
 */

#ifndef CFFTSETUP_H_
#define CFFTSETUP_H_

#include "fpu_cfft.h"

#define CFFT_STAGES     9 //originally 8 for a size of 512 real or 256 complex
#define CFFT_SIZE       (1 << CFFT_STAGES)
extern float CFFTin1Buff[CFFT_SIZE*2];
extern float CFFTin2Buff[CFFT_SIZE*2];
extern float CFFTin3Buff[CFFT_SIZE*2];
extern float CFFTin4Buff[CFFT_SIZE*2];
extern float CFFToutBuff[CFFT_SIZE*2];
extern CFFT_F32_STRUCT_Handle hnd_cfft;
void CFFT_Init(void);

#endif /* CFFTSETUP_H_ */
