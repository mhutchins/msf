#include <stdio.h>
#include <math.h>
#include "unixtime.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "spi.h"
#include "max7219.h"
#include <util/delay.h>

void max7219(uint8_t addr, uint8_t data)
{
        PORTB &= ~(1 << SPI_SS);        // SS LOW
        SPDR = addr;
        while(!(SPSR & (1 << SPIF)));
        SPDR = data;
        while(!(SPSR & (1 << SPIF)));
        PORTB |= (1 << SPI_SS);         // SS HIGH
        _delay_us(10);
        PORTB &= ~(1 << SPI_SS);        // SS LOW
}


void displaytime(time_t *clock)
{
	struct tm *timeptr;

	timeptr=gmtime(clock);

	
        max7219(MAX7219_DIGIT4, timeptr->tm_min%10);
        max7219(MAX7219_DIGIT5, timeptr->tm_min/10);
        max7219(MAX7219_DIGIT6, timeptr->tm_hour%10);
        max7219(MAX7219_DIGIT7, timeptr->tm_hour/10);

        //fprintf(&mylcd, "Hi ");

	LCD_Clear();
	fprintf(stdout, "CLOCK -> %02d:%02d\n", timeptr->tm_hour, timeptr->tm_min);
}

