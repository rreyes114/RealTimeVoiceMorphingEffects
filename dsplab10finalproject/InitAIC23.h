/*
 * InitAIC23.h
 *
 *  Created on: Oct 13, 2021
 *      Author: Roehl
 */

#ifndef INITAIC23_H_
#define INITAIC23_H_

void InitMcBSPb();
void InitSPIA();
void InitAIC23();
void SpiTransmit(uint16_t data);

#endif /* INITAIC23_H_ */
