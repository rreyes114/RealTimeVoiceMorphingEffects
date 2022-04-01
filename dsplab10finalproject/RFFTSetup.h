/*
 * RFFTSetup.h
 *
 *  Created on: Nov 30, 2021
 *      Author: Roehl
 */

#ifndef RFFTSETUP_H_
#define RFFTSETUP_H_

#include "fpu_rfft.h"

#define RFFT_STAGES     9
#define RFFT_SIZE       (1 << RFFT_STAGES)
extern float RFFTin0Buff[RFFT_SIZE];
extern RFFT_F32_STRUCT_Handle hnd_rfft;
void RFFT_Init();

#endif /* RFFTSETUP_H_ */
