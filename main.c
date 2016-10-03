#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "unixtime.h"

#include "i2cmaster.h"
#include "spi.h"
#include "pcf8574.h"
#include "ds3231.h"
#include "max7219.h"
#include <util/delay.h>

#define BAUD 115200
#include <util/setbaud.h>

#define BVV(bit, val) ((val)?_BV(bit):0)

#include "lcd.h"
#include "msf.h"
#include "at24c32.h"
#include "keypad.h"


time_t	*clock_time;

 
static void usart_init(void)
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~_BV(U2X0);
#endif
    UCSR0B = BVV(TXEN0, 1) | BVV(RXEN0, 0); /* Only TX */
}
 
static void usart_putchar(char c) {
    while(!(UCSR0A & _BV(UDRE0)));
    UDR0 = c;
}
 
/*
static void usart_puts(const char *s)
{
    while(*s != '\0')
    {
        usart_putchar(*s++);
    }
}
*/

int usart_putchar_printf(char var, FILE *stream) {
    if (var == '\n') usart_putchar('\r');
    usart_putchar(var);
    return 0;
}

int lcd_printf_char(char var, FILE *stream) {
    LCD_Write_Char(var);
    return 0;
}

FILE display_serial = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);
FILE display_lcd = FDEV_SETUP_STREAM(lcd_printf_char, NULL, _FDEV_SETUP_WRITE);

int main(void)
{
	time_t rtc_time=0;
	clock_time = &rtc_time;
	uint8_t count=0;
	uint8_t lcd_bl=0;

	usart_init();
	i2c_init();
	spi_init();
	timer_init();


	max7219(MAX7219_SHUTDOWN, 0x01);
	max7219(MAX7219_SCANLIMIT, 0x07);
	max7219(MAX7219_INTENSITY, 0x02);
	max7219(MAX7219_TEST, 0x00);
	max7219(MAX7219_DECODE, 0xFF);

	stderr = &display_serial;
	stdout = &display_lcd;

/*    DDRD = 0x00;	// ALL INPUT
    PORTD = 0x00;	// NO PULLUP
    PIND = 0x00;	// NO PULLUP
*/
    DDRD = 0xff;	// ALL OUTPUT

    //DDRD | (1 << PD3);     // Set the PD3 pin (OUTPUT)
    DDRD &= ~(1 << PD2);     // Clear the PD2 pin (INPUT) 
    //PORTD |= ~(1 << PORTD2);    // turn On the Pull-up
    // PD2 is now an input with pull-up enabled

//PORTD &= ~(1 << PD3);        // LO

/*
    EICRA |= (1 << ISC00);    // set INT0 to trigger on ANY logic change
    EIMSK |= (1 << INT0);     // Turns on INT0
*/


    sei();                    // turn on interrupts


/*
	uint8_t i;
	for(i=0;i<10;i++)
	{
		fprintf(stderr, "Sleep %d\n", i);
		_delay_ms(1000);
	}
*/


    //at24c32_write(7, 11, 0x23);
    //at24c32_write(7, 12, 0x34);
	// Turn on 32khz output
/*
	uint8_t c=1;
	c = ds3231_read(0x0F);
	c |= (1 << 3);
	ds3231_write(0x0F, c);
*/

	LCD_Init();
    while(1)
    {
	count++;
        /*main program loop here */

	rtc_time = mktime(ds3231_readtime());
	clock_time = &rtc_time;

	//LCD_Clear();
	//fprintf(stdout, "Hi ");

	if (count % 10)
		lcd_bl= 1 - lcd_bl;

	LCD_BL(lcd_bl);

/*
	if (sync_flag == 1)
	{
		sync_flag=0;
		fprintf(stderr, "SYNC\n");
		decode();
	}
	_delay_ms(100);
*/

	_delay_ms(100);

	keypad();

	fprintf(stderr, "LOOP [%d]\n", lcd_bl);
    }
}



/*ISR (INT0_vect)
{
	fprintf(stderr, "!!!!!\n");
}
*/
