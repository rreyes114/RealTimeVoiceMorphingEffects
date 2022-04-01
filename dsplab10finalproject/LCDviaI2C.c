/*
 * LCDviaI2C.c
 *
 *  Created on: Oct 6, 2021
 *      Author: Roehl
 */


#include <F2837xD_Device.h>
#include "OneToOneI2CDriver.h"
#include "LCDviaI2C.h"

/*
 * <summary>
 *  Send a command (RS=0) or data (RS=1) to the LCD
 * </summary>
 */
void LCD_SendCmndOrData(uint16_t input, uint16_t RS)
{
    char nibble;
    uint16_t byteArray[3];
    for(int i = 0; i < 2; i++) {
        nibble = (input << (4*i)) & 0xF0;
        byteArray[0] = nibble | 0x8 | RS;
        byteArray[1] = nibble | 0xC | RS;
        byteArray[2] = nibble | 0x8 | RS;
        I2C_O2O_SendBytes(&byteArray[0], 3);
    }
}

/*
 * <summary>
 *  Initialize LCD. Cursor should be blinking in LCD after this function
 * </summary>
 */
void LCD_Init(){
    //initialize I2C
    I2C_O2O_Master_Init(0x27, 200, 12);

    //send command to initialize LCD
    LCD_SendCmndOrData(0x33, 0x00);
    LCD_SendCmndOrData(0x32, 0x00);
    LCD_SendCmndOrData(0x28, 0x00);
    LCD_SendCmndOrData(0x0F, 0x00);
    LCD_SendCmndOrData(0x01, 0x00);
}

/*
 * <summary>
 *  Write a string of characters to the LCD
 * </summary>
 */
void LCD_WriteString(char*string)
{
    while(*string) {
        LCD_SendCmndOrData(*string, 0x01);
        string++;
    }
}

/*
 * <summary>
 *  Clear LCD and return home
 * </summary>
 */
void LCD_Clear() {
    LCD_SendCmndOrData(0x01, 0x00);
}

/*
 * <summary>
 *  Convert float to ascii for printing
 * </summary>
 */
char LCD_charArray[16];
/*
void float2ascii(float32 result) {
    uint16_t tens, ones, tenths, hundredths;
    float32 tenths_FP;
    tens = (uint16_t)(result/10);
    ones = (uint16_t)(result) - tens*10;
    tenths_FP = (result - (float)((uint16_t)(result)))*10;
    tenths = (uint16_t)(tenths_FP);
    hundredths = (uint16_t)((tenths_FP - (float)(tenths))*10);
    LCD_charArray[0] = tens + 0x30;
    LCD_charArray[1] = ones + 0x30;
    LCD_charArray[2] = '.';
    LCD_charArray[3] = tenths + 0x30;
    LCD_charArray[4] = hundredths + 0x30;
}*/

void float2ascii(float32 result) {
    float32 integer, fraction;
    uint16_t digits = 1;
    uint16_t charIndex = 0;
    integer = (float32)((uint16_t)(result));
    fraction = result - integer;
    while(integer >= 10) {
        integer /= 10;
        digits++;
    }
    for(uint16_t i = 0; i < digits; i++){//store each integer digit
        LCD_charArray[charIndex&0xF] = (uint16_t)(integer) + 0x30;
        charIndex++;
        integer = integer - (float32)((uint16_t)(integer));
        integer *= 10;
    }
    if(fraction != 0.0) {
        LCD_charArray[charIndex&0xF] = '.';
        charIndex++;
        for(uint16_t i = 0; i < 2; i++){     //convert two decimal places
            fraction *= 10;
            LCD_charArray[charIndex&0xF] = (uint16_t)(fraction) + 0x30;
            charIndex++;
            fraction = fraction - (float32)((uint16_t)(fraction));
        }
    }
    while(charIndex < 10) {              //zero out rest of buffer
        LCD_charArray[charIndex&0xF] = 0x0;
        charIndex++;
    }
}
