#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

#include "i2cmaster.h"
#include "spi.h"
#include "pcf8574.h"
#include "ds3231.h"

#define BAUD 115200
#include <util/setbaud.h>

#define BVV(bit, val) ((val)?_BV(bit):0)

#include "timer.h"


char dayname[][4]={"Err", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };


uint8_t BcdToUint8(uint8_t val)
{
    return val - 6 * (val >> 4);
}

uint8_t BcdToBin24Hour(uint8_t bcdHour)
{
    uint8_t hour;
    if (bcdHour & 0x40)
    {
        // 12 hour mode, convert to 24
        bool isPm = ((bcdHour & 0x20) != 0);

        hour = BcdToUint8(bcdHour & 0x1f);
        if (isPm)
        {
           hour += 12;
        }
    }
    else
    {
        hour = BcdToUint8(bcdHour);
    }
    return hour;
}
 
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
 
static void usart_puts(const char *s)
{
    while(*s != '\0')
    {
        usart_putchar(*s++);
    }
}

int usart_putchar_printf(char var, FILE *stream) {
    if (var == '\n') usart_putchar('\r');
    usart_putchar(var);
    return 0;
}

FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);

int main(void)
{
    uint8_t val;

    uint16_t oldoff=0;

    usart_init();
    //i2c_init();
    //spi_init();

   timer_init();

    stdout = stderr = &mystdout;

    DDRD = 0x00;	// ALL INPUT
    PORTD = 0x00;	// NO PULLUP
    DDRD = 0xff;	// ALL OUTPUT

    //DDRD | (1 << PD3);     // Set the PD3 pin (OUTPUT)
    DDRD &= ~(1 << PD2);     // Clear the PD2 pin (INPUT) 

    //PORTD |= (1 << PORTD2);    // turn On the Pull-up
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
		printf("Sleep %d\n", i);
		_delay_ms(1000);
	}
*/

	uint8_t c=1;

    uint8_t o_val=0x00;

    while(1)
    {
        /*main program loop here */
	//pcf8574_write(0, 0x0f);
	val = pcf8574_read(0);
	//pcf8574_write(0, 0xf0);
	val |= pcf8574_read(0);

	if (val != o_val)
	{

	
		printf("SCAN: ");
		printf("%02x\n", val);
		o_val = val;


	}

	if (val == 0x77)
		offset=offset+0x100;
	if (val == 0x7b)
		offset=offset+0x1;
	if (val == 0x7d)
		offset=offset-0x1;
	if (val == 0x7e)
		offset=offset-0x100;

	if (offset != oldoff)
	{
		printf("0x%04x\n", offset);
		oldoff=offset;
	}

	if (sync_flag == 1)
	{
		sync_flag=0;
		printf("SYNC\n");
		decode();
	}
	//_delay_ms(100);
/*

			max7219(MAX7219_SHUTDOWN, 0x01);
			max7219(MAX7219_SCANLIMIT, 0x07);
			max7219(MAX7219_INTENSITY, 0x02);
			max7219(MAX7219_TEST, 0x00);
			max7219(MAX7219_DECODE, 0xFF);
			max7219(MAX7219_DIGIT0, 0x01);
			max7219(MAX7219_DIGIT1, 0x02);
			max7219(MAX7219_DIGIT2, 0x03);
			max7219(MAX7219_DIGIT3, 0x04);
			max7219(MAX7219_DIGIT4, 0x05);
			max7219(MAX7219_DIGIT5, 0x06);
			max7219(MAX7219_DIGIT6, 0x07);
			max7219(MAX7219_DIGIT7, 0x08);
	uint8_t sec, min, hour, dow, dom, mon, year, century;
	uint8_t AL_tmp, AL1_mask, AL1_sec, AL1_min, AL1_hour, AL1_day;
	uint8_t AL2_mask, AL2_min, AL2_hour, AL2_day;
	uint8_t ds3231_control, ds3231_status;
	uint16_t ds3231_temp_degrees, ds3231_temp_fract;

	sec = BcdToUint8(ds3231_read(0x00));
	min = BcdToUint8(ds3231_read(0x01));
	hour= BcdToBin24Hour(ds3231_read(0x02));
	dow=ds3231_read(0x03);
	dom = BcdToUint8(ds3231_read(0x04));
	mon = BcdToUint8(ds3231_read(0x05));
	year = BcdToUint8(ds3231_read(0x06));
	if ( year & 0x40 )
		century=21;
	else
		century=20;

	AL1_mask=0;

	AL_tmp = ds3231_read(0x07);
	AL1_sec = BcdToUint8(AL_tmp & 0x7F);
	AL1_mask |= (AL_tmp & (1<<7))>>7;

	AL_tmp = ds3231_read(0x08);
	AL1_min = BcdToUint8(AL_tmp & 0x7F);
	AL1_mask |= (AL_tmp & (1<<6))>>6;

	AL_tmp = ds3231_read(0x09);
	AL1_hour = BcdToBin24Hour(AL_tmp & 0x7F);
	AL1_mask |= (AL_tmp & (1<<5))>>5;

	AL_tmp = ds3231_read(0x0A);
	AL1_day = BcdToUint8(AL_tmp & 0x7F);
	AL1_mask |= (AL_tmp & (1<<4))>>4;

	AL2_mask=0;

	AL_tmp = ds3231_read(0x0B);
	AL2_min = BcdToUint8(AL_tmp & 0x7F);
	AL2_mask |= (AL_tmp & (1<<6))>>6;

	AL_tmp = ds3231_read(0x0C);
	AL2_hour = BcdToBin24Hour(AL_tmp & 0x7F);
	AL2_mask |= (AL_tmp & (1<<5))>>5;

	AL_tmp = ds3231_read(0x0D);
	AL2_day = BcdToUint8(AL_tmp & 0x7F);
	AL2_mask |= (AL_tmp & (1<<4))>>4;

	ds3231_control = ds3231_read(0x0E);
	ds3231_status = ds3231_read(0x0F);
	ds3231_temp_degrees = ds3231_read(0x11);
	ds3231_temp_fract = (ds3231_read(0x11) >> 6) * 25;

	printf("%s %02d/%02d/%02d%02d %02d:%02d %02d\n", dayname[dow], dom, mon, century, year, hour, min, sec);
	printf("AL1: %02d %02d:%02d %02d\n", AL1_day, AL1_hour, AL1_min, AL1_sec);
	printf("AL2: %02d %02d:%02d \n", AL2_day, AL2_hour, AL2_min);
	printf("-- %d\n", tick);
	_delay_ms(100);
	printf("-- %d\n", tick);
*/

    }
}



/*ISR (INT0_vect)
{
	printf("!!!!!\n");
}
*/
