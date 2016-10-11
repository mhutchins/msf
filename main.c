#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "time.h"
//#include "time.h"

#include "i2cmaster.h"
#include "spi.h"
#include "led.h"
#include "pcf8574.h"
#include "ds3231.h"
#include "max7219.h"
#include <util/delay.h>


#define BAUD 115200
#define BAUD_TOL 5

//#define BAUD 57600
#include <util/setbaud.h>

#define BVV(bit, val) ((val)?_BV(bit):0)

#include "lcd.h"
#include "msf.h"
#include "at24c32.h"
#include "keypad.h"
#include "main.h"


time_t	rtc_time;
time_t	msf_time[2];

 
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


int led_printf_char(char var, FILE *stream) {
	static uint8_t idx=0;
	uint8_t bitval;

	if (var == '\n')
	{
		idx = 0;
		return 0;
	}
        bitval = getled(var);
        //max7219(MAX7219_DIGIT7 - idx, bitval);
	//fprintf(stderr, "Sending %d to pos %d\n", bitval, idx);
	led_framebuf[idx][0]=bitval;
	led_framebuf[idx][1]=bitval;
	led_framebuf[idx][2]=bitval;
	led_framebuf[idx][3]=bitval;

	idx = ((idx + 1) & 0x07);

	return 0;
}
FILE display_serial = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);
FILE display_lcd = FDEV_SETUP_STREAM(lcd_printf_char, NULL, _FDEV_SETUP_WRITE);
FILE display_led = FDEV_SETUP_STREAM(led_printf_char, NULL, _FDEV_SETUP_WRITE);

int main(void)
{
	uint8_t count=0;
	//uint8_t lcd_bl=0;

	usart_init();
	i2c_init();
	spi_init();
	timer_init();
	struct tm *temp_tm;
	long unixtime;


	max7219(MAX7219_SHUTDOWN, 0x01);
	max7219(MAX7219_SCANLIMIT, 0x07);
	max7219(MAX7219_INTENSITY, 0x02);
	max7219(MAX7219_TEST, 0x00);
	max7219(MAX7219_DECODE, 0x00);

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

	fprintf(stderr, "Libc version: %s\n", 	__AVR_LIBC_VERSION_STRING__);


	set_zone(0);
	
    while(1)
    {
	//max7219(MAX7219_SHUTDOWN, 0x01);

        //fprintf(&display_led, "Hello000");
	//max7219(MAX7219_DIGIT0, (1 << count));
	count++;
	//count = count & 0x07;
        /*main program loop here */

	rtc_time = ds3231_readtime();
	unixtime=rtc_time + UNIX_OFFSET;
	temp_tm = localtime(&rtc_time);
	fprintf(stderr, "RTC RET: %02d:%02d\n", temp_tm->tm_hour, temp_tm->tm_min);
	fprintf(stderr, "RTC RET: %lu\n", unixtime);

	//LCD_Clear();
	//fprintf(stdout, "Hi ");

/*
	if (count % 10)
		lcd_bl= 1 - lcd_bl;

*/

	LCD_BL(1);


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

	fprintf(&display_lcd, "LOOP [%02x]\n", count);
    }
}



/*ISR (INT0_vect)
{
	fprintf(stderr, "!!!!!\n");
}
*/

time_t time_master(void)
{
	time_t clock_time=0;

	if (msf_time[0] != 0 && msf_time[1] != 0)
	{
		if (abs(msf_time[0] - msf_time[1]) == 60)
		{
			if(msf_time[0] > msf_time[1])
				clock_time=msf_time[0];
			else
				clock_time=msf_time[1];
		}
	}

	if (clock_time == 0)
		clock_time = rtc_time;

	fprintf(stderr, "TIME_MASTER: returning %u\n", (uint16_t) clock_time);
	return clock_time;
}
