/*
 * SWandLED.c
 *
 *  Created on: Oct 18, 2021
 *      Author: Roehl
 */

#include <F28x_Project.h>
#include <F2837xD_Device.h>
#include "SWandLED.h"

void GPIO_Init() {
    //initialize LEDs
    GpioCtrlRegs.GPAMUX1.all &= 0xFFFF0000;
    GpioCtrlRegs.GPADIR.all  |= 0x000000FF;
    //initialize switches
    GpioCtrlRegs.GPAMUX1.all &= 0x0F00FFFF;
    GpioCtrlRegs.GPAMUX2.all &= 0xFFFFFFFC;
    GpioCtrlRegs.GPAPUD.all  &= 0xFFFE30FF;
    GpioCtrlRegs.GPADIR.all  &= 0xFFFE30FF;
}

void SW_Check(uint32_t value) {
    uint32_t switches = GpioDataRegs.GPADAT.all;
    while(switches != value) {
        switches = GpioDataRegs.GPADAT.all;
        switches = ((switches>>8) & 0x0F) | ((switches>>10) & 0x70);// | 0x80;
        switches >>= 3;
    }
}

uint32_t DIP_ReadAll() {
    uint32_t switches = GpioDataRegs.GPADAT.all;
    switches = ((switches>>8) & 0x0F) | ((switches>>10) & 0x70);
    switches >>= 3;
    return switches;
}

/*
 * Boolean check for pushbutton switch press.
 * n = 1: leftmost button
 * n = 2: center button
 * n = 3: rightmost button
 * Buttons are active low, so GPIO is 0 when pressed
 * This might be too slow
 */
bool PBn_Pressed(uint16_t n) {
    uint32_t switches = GpioDataRegs.GPADAT.all;
    switches = ((switches>>8) & 0x0F) | ((switches>>10) & 0x70);
    if(n==1)
        return (switches & 0x04)==0;
    if(n==2)
        return (switches & 0x02)==0;
    if(n==3)
        return (switches & 0x01)==0;
    return false;
}

bool PB2_Pressed() {
    return (GpioDataRegs.GPADAT.all & 0x400)==0;
}

bool PB1_Pressed() {
    return (GpioDataRegs.GPADAT.all & 0x200)==0;
}

bool PB0_Pressed() {
    return (GpioDataRegs.GPADAT.all & 0x100)==0;
}

bool DIP0_Read() {
    return GpioDataRegs.GPADAT.all & 0x800;
}

void LED_Write(uint16_t value) {
    GpioDataRegs.GPADAT.all = ~value;
}
