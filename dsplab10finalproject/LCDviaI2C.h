/*
 * LCDviaI2C.h
 *
 *  Created on: Oct 6, 2021
 *      Author: Roehl
 */

#ifndef LCDVIAI2C_H_
#define LCDVIAI2C_H_

extern char LCD_charArray[16];
/*
 * <summary>
 *  Convert float to ascii for printing
 * </summary>
 */
void float2ascii(float32 result);

/*
 * <summary>
 *  Send a command (RS=0) or data (RS=1) to the LCD
 * </summary>
 */
void LCD_SendCmndOrData(uint16_t input, uint16_t RS);

/*
 * <summary>
 *  Initialize LCD. Cursor should be blinking in LCD after this function
 * </summary>
 */
void LCD_Init();

/*
 * <summary>
 *  Write a string of characters to the LCD
 * </summary>
 */
void LCD_WriteString(char*string);

/*
 * <summary>
 *  Clear LCD and return home
 * </summary>
 */
void LCD_Clear();

#endif /* LCDVIAI2C_H_ */
