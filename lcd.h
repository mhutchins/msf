/*
 * File:   l.h
 * Author: danny
 *
 * Created on November 4, 2014, 9:45 PM
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {

#endif


/*
 *
 * LCD Pinout:
 *
 *
  
 14   13   12   11  10   9   8   7   6   5   4   3   2   1  15  16
  D    D    D    D   D   D   D   D   E   R   R   C   V   G   L    L  
  B    B    B    B   B   B   B   B       /   S   o   c   n   E    E  
  7    6    5    4   3   2   1   0       W       n   c   d   D    D  
                                                 t           +a  -k
                     X   X   X   X               r          
                                                 a          
                     7   6   5   4   2   1   0   s          
                                                 t          
*/

#define	lcd_RS	0
#define	lcd_RW	1
#define	lcd_E	2
#define	lcd_BL	3
#define	lcd_D0	4
#define	lcd_D1	5
#define	lcd_D2	6
#define	lcd_D3	7

#define mode_RD         0x02
#define mode_WR         0x01
#define mode_DAT        0x20
#define mode_CMD        0x10

#define LCD_PIC_I2C_ADDR  0x27

// Line addresses for LCDs which use
// the Hitachi HD44780U controller chip
#define LCD_PIC_LINE_1_ADDRESS 0x00
#define LCD_PIC_LINE_2_ADDRESS 0x40
#define LCD_PIC_LINE_3_ADDRESS 0x14
#define LCD_PIC_LINE_4_ADDRESS 0x54

void LCD_Init();
void LCD_Goto(uint8_t x, uint8_t y);
void LCD_Write_Char(char c);
void LCD_Write_String(const char *str);
void LCD_Write_Int(int32_t num);
void LCD_Write_Byte(uint8_t address, uint8_t n);
void LCD_Clear();
void LCD_Clear_Line(uint8_t line);

void LCD_Open(); // Open I2C Connection
void LCD_Close(); // Close I2C Connection
void LCD_BL(uint8_t status); // set blacklight

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

