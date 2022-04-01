/*
 * AudioMixFunctions.h
 *
 *  Created on: Oct 20, 2021
 *      Author: Roehl
 */

#ifndef AUDIOMIXFUNCTIONS_H_
#define AUDIOMIXFUNCTIONS_H_

void Check_AudioFlags(int32_t sample_L);
void Check_SRControl();
extern volatile uint32_t circBuffPtr;
extern uint32_t mask;

#endif /* AUDIOMIXFUNCTIONS_H_ */
