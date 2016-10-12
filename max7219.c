#include <stdio.h>
#include <math.h>
#include "time.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "spi.h"
#include "max7219.h"
#include <util/delay.h>
#include "lcd.h"
#include "main.h"


void max7219(uint8_t addr, uint8_t data)
{
        PORTB &= ~(1 << SPI_SS);        // SS LOW
        //_delay_us(10);

        SPDR = addr;
        while(!(SPSR & (1 << SPIF)));
        SPDR = data;
        while(!(SPSR & (1 << SPIF)));
        //_delay_us(10);
        PORTB |= (1 << SPI_SS);         // SS HIGH
        //_delay_us(10);
        PORTB &= ~(1 << SPI_SS);        // SS LOW
        //_delay_us(50);
}


void displaytime(time_t clock)
{
	struct tm time;

	gmtime_r(&clock, &time);

	//fprintf(&display_led, "%02u%02u T\n", time.tm_hour, time.tm_min);

/*
        max7219(MAX7219_DIGIT4, timeptr->tm_min%10);
        max7219(MAX7219_DIGIT5, timeptr->tm_min/10);
        max7219(MAX7219_DIGIT6, timeptr->tm_hour%10);
        max7219(MAX7219_DIGIT7, timeptr->tm_hour/10);
*/

        //fprintf(&mylcd, "Hi ");

	LCD_Clear();
	fprintf(stderr, "CLOCK -> %02d:%02d\n", time.tm_hour, time.tm_min);
}

uint8_t getled(unsigned char ch)
{
	uint8_t retval;

    switch (ch) {
    case 'A':
    case 'a':
	retval = (__extension__ 0b01110111);
	break;
    case 'B':
	retval = (__extension__ 0b01111111);
	break;
    case 'b':
	retval = (__extension__ 0b00011111);
	break;
    case 'C':
	retval = (__extension__ 0b01001110);
	break;
    case 'c':
	retval = (__extension__ 0b00001101);
	break;
    case 'D':
	retval = (__extension__ 0b01111110);
	break;
    case 'd':
	retval = (__extension__ 0b00111101);
	break;
    case 'E':
    case 'e':
	retval = (__extension__ 0b01001111);
	break;
    case 'f':
    case 'F':
	retval = (__extension__ 0b01000111);
	break;
    case 'g':
    case 'G':
	retval = (__extension__ 0b01111011);
	break;
    case 'H':
	retval = (__extension__ 0b00110111);
	break;
    case 'h':
	retval = (__extension__ 0b00010111);
	break;
    case 'i':
    case 'I':
	retval = (__extension__ 0b00110000);
	break;
    case 'j':
    case 'J':
	retval = (__extension__ 0b00111100);
	break;
    case 'k':
    case 'K':
	retval = (__extension__ 0b00110111);
	break;
    case 'L':
    case 'l':
	retval = (__extension__ 0b00001110);
	break;
    case 'm':
    case 'M':
    case 'n':
    case 'N':
	retval = (__extension__ 0b00010101);
	break;
    case 'o':
	retval = (__extension__ 0b00011101);
	break;
    case 'O':
	retval = (__extension__ 0b01111110);
	break;
    case 'P':
    case 'p':
	retval = (__extension__ 0b01100111);
	break;
    case 'q':
    case 'Q':
	retval = (__extension__ 0b01110011);
	break;
    case 'r':
	retval = (__extension__ 0b00000101);
	break;
    case 'R':
	retval = (__extension__ 0b01000110);
	break;
    case 's':
    case 'S':
	retval = (__extension__ 0b01011011);
	break;
    case 't':
    case 'T':
	retval = (__extension__ 0b00001111);
	break;
    case 'u':
	retval = (__extension__ 0b00011100);
	break;
    case 'U':
    case 'v':
    case 'V':
    case 'w':
    case 'W':
	retval = (__extension__ 0b00111110);
	break;
    case 'x':
    case 'X':
	retval = (__extension__ 0b00110111);
	break;
    case 'y':
    case 'Y':
	retval = (__extension__ 0b00111011);
	break;
    case 'z':
    case 'Z':
	retval = (__extension__ 0b01101101);
	break;
    case '0':
	retval = (__extension__ 0b01111110);
	break;
    case '1':
	retval = (__extension__ 0b00110000);
	break;
    case '2':
	retval = (__extension__ 0b01101101);
	break;
    case '3':
	retval = (__extension__ 0b01111001);
	break;
    case '4':
	retval = (__extension__ 0b00110011);
	break;
    case '5':
	retval = (__extension__ 0b01011011);
	break;
    case '6':
	retval = (__extension__ 0b01011111);
	break;
    case '7':
	retval = (__extension__ 0b01110000);
	break;
    case '8':
	retval = (__extension__ 0b01111111);
	break;
    case '9':
	retval = (__extension__ 0b01111011);
	break;
    case '_':
	retval = (__extension__ 0b00000001);
	break;
    case ' ':
	retval = (__extension__ 0b00000000);
	break;
    default:
	retval = (__extension__ 0b00000001);
	break;
    }

	//fprintf(stderr, "GETLED: Returning %d for char [%c]\n", retval, ch);
	//printled(retval);
    return retval;
}

void printled(uint8_t bitval)
{
    //uint8_t bitval = getled(ch);
    int i = 0;
    int n = 0;

    uint8_t r[5];
    r[0] = 0;
    r[1] = 0;
    r[2] = 0;
    r[3] = 0;
    r[4] = 0;

    if (bitval & (1 << D_A))
	r[0] = __extension__ 0b00111100;
    if (bitval & (1 << D_B))
	r[1] |= __extension__ 0b00000100;
    if (bitval & (1 << D_C))
	r[3] |= __extension__ 0b00000100;
    if (bitval & (1 << D_D))
	r[4] = __extension__ 0b00111100;
    if (bitval & (1 << D_E))
	r[3] |= __extension__ 0b00100000;
    if (bitval & (1 << D_F))
	r[1] |= __extension__ 0b00100000;
    if (bitval & (1 << D_G))
	r[2] |= __extension__ 0b00111100;

    for (n = 0; n < 5; n++) {
	for (i = 7; i >= 0; i--)
	    if (((r[n] & (1 << i)) > 0))
		fprintf(stderr, "X");
	    else
		fprintf(stderr, " ");

	fprintf(stderr, "\n");
    }

    fprintf(stderr, "\n");
}

