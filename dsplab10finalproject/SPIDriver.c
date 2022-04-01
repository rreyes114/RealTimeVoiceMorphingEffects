/*
 * SPIDriver.c
 *
 *  Created on: Oct 5, 2021
 *      Author: Roehl
 */

#include <F2837xD_Device.h>
#include "SPIDriver.h"

/*
 * <summary>
 *  Initializes the GPIO for the SPI
 * </summary>
 */
void InitSPIGpio()
{
    EALLOW;
    //initialize P65:63 GPyGMUX bits for SPI function
    GpioCtrlRegs.GPBGMUX2.bit.GPIO63 = 3;   //SIMO
    GpioCtrlRegs.GPCGMUX1.bit.GPIO64 = 3;   //SOMI
    GpioCtrlRegs.GPCGMUX1.bit.GPIO65 = 3;   //CLK

    //initialize P65:63 GPyMUX bits for SPI function
    GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 3;    //SIMO
    GpioCtrlRegs.GPCMUX1.bit.GPIO64 = 3;    //SOMI
    GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 3;    //CLK

    //initialize P67:66 GPyMUX bits for GPIO
    GpioCtrlRegs.GPCMUX1.bit.GPIO66 = 0;
    GpioCtrlRegs.GPCMUX1.bit.GPIO67 = 0;

    //initialize GPyDIR bits to set pins as inputs/outputs
    GpioCtrlRegs.GPBDIR.bit.GPIO63 = 1;     //master output
    GpioCtrlRegs.GPCDIR.bit.GPIO64 = 0;     //master input
    GpioCtrlRegs.GPCDIR.bit.GPIO65 = 1;     //clock output
    GpioCtrlRegs.GPCDIR.bit.GPIO66 = 1;     //CS0 output
    GpioCtrlRegs.GPCDIR.bit.GPIO67 = 1;     //CS1 output

    //initialize GPxQSELn bits to asynchronous mode for inputs
    GpioCtrlRegs.GPCQSEL1.bit.GPIO64 = 3;

    //initialize GPyPUD bits to enable pull-ups on inputs
    GpioCtrlRegs.GPCPUD.bit.GPIO64 = 0;

    //drive enables high
    GpioDataRegs.GPCDAT.bit.GPIO66 = 1;
    GpioDataRegs.GPCDAT.bit.GPIO67 = 1;
}

/*
 * <summary>
 *  Initializes SPI registers
 * </summary>
 */
void SPI_Master_Init()
{
    //initialize GPIO
    InitSPIGpio();

    EALLOW;

    //clear SPISWRESET bit
    SpibRegs.SPICCR.bit.SPISWRESET = 0;

    //select master mode
    SpibRegs.SPICTL.bit.MASTER_SLAVE = 1;

    //initialize clock polarity to 0 and phase to 1
    SpibRegs.SPICCR.bit.CLKPOLARITY = 0;
    SpibRegs.SPICTL.bit.CLK_PHASE = 1;

    //enable transmit
    SpibRegs.SPICTL.bit.TALK = 1;

    //TODO: high speed mode?
    //SpibRegs.SPICCR.bit.HS_MODE = 1;

    //determine SPI character length and if you need to manually control chip selects
    //SpibRegs.SPICCR.bit.SPICHAR = 0xF;  //16-bit, need to use sequential operation (default)
    SpibRegs.SPICCR.bit.SPICHAR = 0x7;  //8-bit to avoid die boundary

    //TODO: clear overrun and interrupt flags?

    //calculate clock prescaler and baud rate register for 1MHz to 45MHz
    EALLOW;
    ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 0; //LSPCLK = 200MHz
    SpibRegs.SPIBRR.all = 4;            //baud rate = 200/(SPIBRR+1)

    //perform software reset and set free bit
    SpibRegs.SPICCR.bit.SPISWRESET = 1;
    SpibRegs.SPIPRI.bit.FREE = 1;

}

/*
 * <summary>
 *  Send and receive serial data
 * </summary>
 */
uint16_t SPI_Transmit(uint16_t transmitdata)
{
    //send data to SPI register
    //SpibRegs.SPIDAT = transmitdata;
    SpibRegs.SPITXBUF = transmitdata;

    //wait until the data has been sent by checking INT_FLAG
    while(!SpibRegs.SPISTS.bit.INT_FLAG);

    //return the data you received
    uint16_t received = SpibRegs.SPIRXBUF; //auto-clears INT_FLAG
    //make sure to flush any buffers and clear any flags

    return received;
}
