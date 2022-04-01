/*
 * Timer1andADCA.c
 *
 *  Created on: Oct 18, 2021
 *      Author: Roehl
 */

#include <F28x_Project.h>
#include <F2837xD_Device.h>
#include "Timer1andADCA.h"

char* msg2 = " Volts";
char* ascii_result = "0.00";
volatile uint16_t AdcData = 0;
float result = 0;
float resolution = 0.00073242187; //12 bit resolution for span of 3V
uint16_t ones, tenths, hundredths;
float tenths_FP;

void Adc2Ascii() {
    result = (float)(AdcData) * resolution;
    ones = (uint16_t)(result);
    tenths_FP = (result - (float)(ones))*10;
    tenths = (uint16_t)(tenths_FP);
    hundredths = (uint16_t)((tenths_FP - (float)(tenths))*10);
    ascii_result[0] = ones + 0x30;
    ascii_result[2] = tenths + 0x30;
    ascii_result[3] = hundredths + 0x30;
}

interrupt void Timer1_ISR() {
    AdcaRegs.ADCSOCFRC1.all = 0x1;          // Force conversion on channel 0
    AdcData = AdcaResultRegs.ADCRESULT0;    // Read ADC result into global variable
    //Adc2Ascii();
    //LCD_Clear();
    //LCD_WriteString(msg1);
    //LCD_WriteString(ascii_result);
    //LCD_WriteString(msg2);
}

void Timer1_Init() {                //Modeled after function in Timer_ADC_Main.c
    InitCpuTimers();                        //Initializes CPU timers
    ConfigCpuTimer(&CpuTimer1, 200, 1e5);   //CPU Timer1, 0.1s period
    PieVectTable.TIMER1_INT = &Timer1_ISR;  //puts function pointer for ISR in PIE vector table
    IER |= M_INT13;                         //Enable INT13
    //EnableInterrupts();                     //Enable PIE and CPU interrupts
    CpuTimer1.RegsAddr->TCR.bit.TSS = 0;    //starts timer 1
}

void ADCA_Init() {                  //Modeled after function in Timer_ADC_Main.c
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;                                 // Set ADCCLK to SYSCLK/4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE); // Initializes ADCA to 12-bit and single-ended mode. Performs internal calibration
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;                                 // Powers up ADC
    DELAY_US(1000);                                                    // Delay to allow ADC to power up
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;                                 // Sets SOC0 to channel 0 -> pin ADCINA0
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14;                                // Sets sample and hold window -> must be at least 1 ADC clock long
}
