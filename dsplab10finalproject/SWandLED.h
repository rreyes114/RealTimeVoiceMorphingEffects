/*
 * SWandLED.h
 *
 *  Created on: Oct 18, 2021
 *      Author: Roehl
 */

#ifndef SWANDLED_H_
#define SWANDLED_H_

void GPIO_Init();
void SW_Check(uint32_t value);
uint32_t DIP_ReadAll();
bool PBn_Pressed(uint16_t n);
void LED_Write(uint16_t value);
bool PB0_Pressed();
bool PB1_Pressed();
bool PB2_Pressed();
bool DIP0_Read();

#endif /* SWANDLED_H_ */
