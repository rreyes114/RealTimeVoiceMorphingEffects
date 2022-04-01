/*
 * SRAMviaSPI.h
 *
 *  Created on: Oct 6, 2021
 *      Author: Roehl
 */

#ifndef SRAMVIASPI_H_
#define SRAMVIASPI_H_

//main functions
void SRAM_WriteWord(uint32_t address, uint16_t word);
uint16_t SRAM_ReadWord(uint32_t address);

//helper functions
void SRAMn_WriteByte(uint16_t CS, uint32_t address,uint16_t byte);
uint16_t SRAMn_ReadByte(uint16_t CS, uint32_t address);
void CS0_Low();
void CS0_High();
void CS1_Low();
void CS1_High();

#endif /* SRAMVIASPI_H_ */
