/*
 * SPIDriver.h
 *
 *  Created on: Oct 5, 2021
 *      Author: Roehl
 */

#ifndef SPIDRIVER_H_
#define SPIDRIVER_H_

/*
 * <summary>
 *  Initializes SPI registers
 * </summary>
 */
void SPI_Master_Init();

/*
 * <summary>
 *  Send and receive serial data
 * </summary>
 */
uint16_t SPI_Transmit(uint16_t transmitdata);

#endif /* SPIDRIVER_H_ */
