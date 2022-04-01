/*
 * SRAMviaSPI.c
 *
 *  Created on: Oct 6, 2021
 *      Author: Roehl
 */
#include <F2837xD_Device.h>
#include "SPIDriver.h"
#include "SRAMviaSPI.h"

//******************** Main functions ********************
void SRAM_WriteWord(uint32_t address, uint16_t word)
{
    uint16_t CS = 0;
    if(address != 0x3FFFF && address != 0x7FFFF) { //non-boundary conditions
        if(address > 0x3FFFF) {
            address %= 0x40000;
            CS = 1;
        }
        if(CS==0){CS0_Low();}
        else{CS1_Low();}

        SPI_Transmit(0x02<<8);             //send instruction for write
        SPI_Transmit(address>>8);
        SPI_Transmit(address);
        SPI_Transmit(address<<8);
        SPI_Transmit(word);                //send upper byte
        SPI_Transmit(word<<8);             //send lower byte

        if(CS==0){CS0_High();}
        else{CS1_High();}
    }
    else if(address == 0x3FFFF) { //first boundary condition
        SRAMn_WriteByte(0,address,word>>8); //send upper byte to top address of SRAM0
        SRAMn_WriteByte(1,0x00000,word);    //send lower byte to bottom address of SRAM1
    }
    else if(address == 0x7FFFF) { //second boundary condition
        SRAMn_WriteByte(1,0x3FFFF,word>>8); //send upper byte to top address of SRAM1
        SRAMn_WriteByte(0,0x00000,word);    //send lower byte to bottom address of SRAM0 (roll-over effect)
    }
}

uint16_t SRAM_ReadWord(uint32_t address)
{
    uint16_t data;
    uint16_t temp;
    uint16_t CS = 0;
    if(address != 0x3FFFF && address != 0x7FFFF) { //non-boundary conditions
        if(address > 0x3FFFF) {
            address %= 0x40000;
            CS = 1;
        }
        if(CS==0){CS0_Low();}
        else{CS1_Low();}

        SPI_Transmit(0x03<<8);             //send instruction for read
        SPI_Transmit(address>>8);
        SPI_Transmit(address);
        SPI_Transmit(address<<8);
        SPI_Transmit(0x00);                //send dummy byte
        temp = SPI_Transmit(0x00);         //send dummy byte and read upper byte
        data = SPI_Transmit(0x00);         //send dummy byte and read lower byte

        if(CS==0){CS0_High();}
        else{CS1_High();}
    }
    else if(address == 0x3FFFF) { //first boundary condition
        temp = SRAMn_ReadByte(0,address);   //read upper byte from top address of SRAM0
        data = SRAMn_ReadByte(1,0x00000);   //read lower byte from bottom address of SRAM1
    }
    else if(address == 0x7FFFF) { //second boundary condition, TODO: for some reason, only works when restarted after failing once (lower byte is wrong)
        temp = SRAMn_ReadByte(1,0x3FFFF);   //read upper byte to top address of SRAM1
        data = SRAMn_ReadByte(0,0x00000);   //read lower byte from bottom address of SRAM0
    }
    return (temp<<8) | data;
}

//******************* Helper functions *******************
void SRAMn_WriteByte(uint16_t CS, uint32_t address,uint16_t byte)
{
    if(CS==0){CS0_Low();}
    else{CS1_Low();}

    SPI_Transmit(0x02<<8);             //send instruction for write
    SPI_Transmit(address>>8);
    SPI_Transmit(address);
    SPI_Transmit(address<<8);
    SPI_Transmit(byte<<8);             //send byte

    if(CS==0){CS0_High();}
    else{CS1_High();}
}

uint16_t SRAMn_ReadByte(uint16_t CS, uint32_t address)
{
    uint16_t data;
    if(CS==0){CS0_Low();}
    else{CS1_Low();}

    SPI_Transmit(0x03<<8);              //send instruction for read
    SPI_Transmit(address>>8);
    SPI_Transmit(address);
    SPI_Transmit(address<<8);
    SPI_Transmit(0x00);                 //send dummy byte
    data = SPI_Transmit(0x00);          //another dummy byte and read data

    if(CS==0){CS0_High();}
    else{CS1_High();}

    return data;
}

void CS0_Low() {
    GpioDataRegs.GPCDAT.bit.GPIO66 = 0;
}

void CS0_High() {
    GpioDataRegs.GPCDAT.bit.GPIO66 = 1;
}

void CS1_Low() {
    GpioDataRegs.GPCDAT.bit.GPIO67 = 0;
}

void CS1_High() {
    GpioDataRegs.GPCDAT.bit.GPIO67 = 1;
}
