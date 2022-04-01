/*
 * RFFTandICFFT.h
 *
 *  Created on: Dec 4, 2021
 *      Author: Roehl
 */

#ifndef RFFTANDICFFT_H_
#define RFFTANDICFFT_H_

#include "fpu_rfft.h"

#define RFFT_STAGES     9
#define RFFT_SIZE       (1 << RFFT_STAGES)
extern float RFFTin0Buff[RFFT_SIZE];
extern RFFT_F32_STRUCT_Handle hnd_rfft;
void RFFT_Init(void);

#include "fpu_cfft.h"

#define CFFT_STAGES     RFFT_STAGES-1//8
#define CFFT_SIZE       (1 << CFFT_STAGES)
extern float CFFToutBuff[CFFT_SIZE*2];
extern CFFT_F32_STRUCT_Handle hnd_cfft;
void CFFT_Init(void);

#endif /* RFFTANDICFFT_H_ */
