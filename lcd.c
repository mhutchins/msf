#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include "util.h"

/*
RS
00 0000 0001	0x01	 Clears entire display and sets DDRAM address 0 in address counter.
00 0000 0010	0x02	 Sets DDRAM address counter to 0. returns display from shifted to original position.  DDRAM contents unchanged.  1.52 ms
00 0000 0011	0x03	 Sets DDRAM address counter to 0. returns display from shifted to original position.  DDRAM contents unchanged.  1.52 ms

00 0000 0100	0x04	 Cursor DEC, no display shift
00 0000 0101	0x05	 Cursor DEC, display shift
00 0000 0110	0x06	 Cursor INC, no display shift
00 0000 0111	0x07	 Cursor INC, display shift

00 0000 1DCB	0x08	 Sets entire display (D) on/off, cursor on/off (C), and blinking of cursor position character (B).  37 µs
00 0000 1000	0x08	 Display off - Cursor off - Blink off
00 0000 1001	0x09	 Display off - Cursor off - Blink on
00 0000 1010	0x0A	 Display off - Cursor on - Blink off
00 0000 1011	0x0B	 Display off - Cursor on - Blink on
00 0000 1100	0x0C	 Display on - Cursor off - Blink off
00 0000 1101	0x0D	 Display on - Cursor off - Blink on
00 0000 1110	0x0E	 Display on - Cursor on - Blink off
00 0000 1111	0x0F	 Display on - Cursor on - Blink on

00 0001 SR..	0x10		 Moves cursor and shifts display without changing DDRAM contents.  37 µs
00 0001 00..	0x10		 Cursor left
00 0001 01..	0x14		 Cursor right
00 0001 10..	0x18		 Screen left
00 0001 11..	0x1C		 Screen right

00 001D NF..	0x20			 Sets interface data length (D), number of display lines (N), and character font (F).  37 µs
00 0010 00..	0x20			 4 bits, 1 line, 5x8 font
00 0010 01..	0x24			 4 bits, 1 line, 5x10 font
00 0010 10..	0x28			 4 bits, 2 line, 5x8 font
00 0010 11..	0x2C			 4 bits, 2 line, 5x10 font
00 0011 00..	0x30			 8 bits, 1 line, 5x8 font
00 0011 01..	0x34			 8 bits, 1 line, 5x10 font
00 0011 10..	0x38			 8 bits, 2 line, 5x8 font
00 0011 11..	0x3C			 8 bits, 2 line, 5x10 font

00 01AA AAAA	0x40	Sets CGRAM address.  CGRAM data is sent and received after this setting.  37 µs
00 0100 0000	0x40	 Address 0x00
00 01.. ....
00 0111 1111	0x4F	 Address 0x3F

00 1AAA AAAA	0x80	Sets DDRAM address.  DDRAM data is sent and received after this setting.  37 µs
00 1000 0000	0x80	 Address 0x00
00 1... ....
00 1111 1111	0x8F	 Address 0x7F

01 dddd dddd    Writes data into DDRAM or CGRAM.  37 µs tADD = 4 µs*

10 BF AC AC AC AC AC AC AC	 Reads busy flag (BF) indicating internal operation is being performed and reads address counter contents.  0 µs
11 DDDD DDDD	Reads data from DDRAM or CGRAM.  37 µs tADD = 4 µs*

R=R/W
S=RS
*/


#include "unixtime.h"
#include "i2cmaster.h"

#include "lcd.h"

static uint8_t  LCD_BL_Status=0;     // 1 for POSITIVE control, 0 for NEGATIVE con
static uint8_t LCD_pins;


void LCD_Open(void)
{
	return;
    //i2c_start_wait(LCD_PIC_I2C_ADDR<<1 | 0x00);
}

void LCD_Close(void)
{
	return;
    //i2c_stop();
}
	

void LCD_write_byte(uint8_t flags, uint8_t data, uint8_t slow)
{
        safe_i2c_start_wait(LCD_PIC_I2C_ADDR<<1 | 0x00);
	LCD_pins = (LCD_BL_Status << lcd_BL);
	LCD_pins |= (data &0xf0);
	LCD_pins |= ((flags & mode_RD) > 0)<<lcd_RW;
	LCD_pins |= ((flags & mode_DAT) > 0)<<lcd_RS;

	i2c_write(LCD_pins);
	// We are at E=0 and all data / control lines
	// are set correctly.

	// E ->HIGH
	LCD_pins |= (1 << lcd_E);
	i2c_write(LCD_pins);

	// E ->LOW
	LCD_pins &= ~(1 << lcd_E);
	i2c_write(LCD_pins);

	if (slow)
		_delay_ms(10);

	LCD_pins &= (0x0f);
	LCD_pins |= (data &0x0f)<<4;

	LCD_pins |= (1 << lcd_E);
	i2c_write(LCD_pins);

	LCD_pins &= ~(1 << lcd_E);
	i2c_write(LCD_pins);

	if (slow)
		_delay_ms(10);

	safe_i2c_stop();
	return;
}

void LCD_BL(uint8_t status)
{
	if (status)
	{
		LCD_BL_Status=1;
		LCD_pins |= (1 << lcd_BL);
	}
	else
	{
		LCD_BL_Status=0;
		LCD_pins &= ~(1 << lcd_BL);
	}

    safe_i2c_start_wait(LCD_PIC_I2C_ADDR<<1 | 0x00);
	i2c_write(LCD_pins);
	safe_i2c_stop();
}

void LCD_Init()
{
    safe_i2c_start_wait(LCD_PIC_I2C_ADDR<<1 | 0x00);
    i2c_write(0x00);
    safe_i2c_stop();
    _delay_ms(100);
    LCD_write_byte(mode_WR|mode_CMD, 0x33, 1);
    LCD_write_byte(mode_WR|mode_CMD, 0x32, 1);
    LCD_write_byte(mode_WR|mode_CMD, 0x28, 1);
    LCD_write_byte(mode_WR|mode_CMD, 0x08, 1);
    LCD_write_byte(mode_WR|mode_CMD, 0x01, 1);
    LCD_write_byte(mode_WR|mode_CMD, 0x06, 1);
    LCD_write_byte(mode_WR|mode_CMD, 0x0C, 1);
    //LCD_write_byte(mode_WR|mode_CMD, 0x01, 1);
    _delay_ms(100);
}

void LCD_Goto(uint8_t x, uint8_t y)
{
uint8_t address;

   switch(y)
     {
      case 1:
        address = LCD_PIC_LINE_1_ADDRESS;
        break;

      case 2:
        address = LCD_PIC_LINE_2_ADDRESS;
        break;

      case 3:
        address = LCD_PIC_LINE_3_ADDRESS;
        break;

      case 4:
        address = LCD_PIC_LINE_4_ADDRESS;
        break;

      default:
        address = LCD_PIC_LINE_1_ADDRESS;
        break;
     }

   address += x-1;
    LCD_write_byte(mode_WR|mode_CMD, 0x80 | address, 0);
}

//===================================
void LCD_Write_String(const char *str)
{
   // Writes a string text[] to LCD via I2C

   while (*str)
   {
        LCD_Write_Char(*str);
        str++;
   }
}

void LCD_Write_Char(char c)
{
    LCD_write_byte(mode_WR|mode_DAT, c, 0);
}

void LCD_Write_Int(int32_t num)
{
    int i;

    if (num < 0) { LCD_Write_String("-"); num *= -1; }

    uint8_t number[10];
    uint8_t num_count = 0;

    do {        
        number[num_count] = num % 10;
        num_count++;
        num /= 10;
    } while (num > 0);

    for (i = num_count-1; i>= 0; i--)
    {
        LCD_Write_Char(number[i] + __extension__ 0b00110000);
    }
}

void LCD_Clear()
{
    LCD_write_byte(mode_WR|mode_CMD, 0x01, 0);
    _delay_ms(5);
}

void LCD_Clear_Line(uint8_t line)
{
    int i;

    LCD_Goto(1,line);
    for (i = 0; i<20; i++)
    {
        LCD_Write_String(" ");
    }
    LCD_Goto(1,line);
}

