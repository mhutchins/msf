#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include "tm1637.h"

#define	TM_CLOCK	PC1
#define	TM_DATA		PC0
#define	TM_BRIGHTNESS	0x02

void tm_init(void)
{
	DDRC |= (1 << TM_DATA);     // (OUTPUT)
	DDRC |= (1 << TM_CLOCK);     // (OUTPUT)

	PORTC |= (1 << TM_DATA);	// DATA HIGH
	PORTC |= (1 << TM_CLOCK);	// CLOCK HIGH

/*
	tm_start();
	fprintf(stderr, "TS Returns %d\n", tm_send(0x88));
	tm_stop();
*/

	tm_start();
	tm_send(0x88 | TM_BRIGHTNESS);	// Display on
	tm_stop();

	tm_update((uint8_t[]){0,0,0,0});

}

void tm_update(uint8_t data[4])
{
	uint8_t i;

	tm_start();
	tm_send(0x40);	// Get ready for data
	tm_stop();

	tm_start();
	tm_send(0xC0);	// Digit 1
	for (i=0;i<4;i++)
		tm_send(data[i]);
	tm_stop();
}

void tm_stop(void)
{
	PORTC &= ~(1 << TM_CLOCK);	// CLOCK LOW
	_delay_us(100);
	PORTC &= ~(1 << TM_DATA);	// DATA LOW
	_delay_us(100);

	PORTC |= (1 << TM_CLOCK);	// CLOCK HIGH
	_delay_us(100);
	PORTC |= (1 << TM_DATA);	// DATA HIGH
	_delay_us(100);
}
void tm_start(void)
{
	PORTC |= (1 << TM_CLOCK);	// CLOCK HIGH
	PORTC |= (1 << TM_DATA);	// DATA HIGH
	_delay_us(100);
	PORTC &= ~(1 << TM_DATA);	// DATA LOW
	_delay_us(100);
	PORTC &= ~(1 << TM_CLOCK);	// CLOCK LOW
	_delay_us(100);

}
uint8_t tm_send(uint8_t data)
{
	uint8_t i;

	for(i=0;i<8;i++)
	{
		PORTC &= ~(1 << TM_CLOCK);	// CLOCK LOW
		if (data & 0x01)
			PORTC |= (1 << TM_DATA);	// DATA HIGH
		else
			PORTC &= ~(1 << TM_DATA);	// DATA LOW
			
		_delay_us(100);
		data = data >> 1;
		PORTC |= (1 << TM_CLOCK);	// CLOCK HIGH
		_delay_us(100);
	}
	return(tm_ack());
}

uint8_t tm_ack(void)
{
	 uint8_t ack;

	PORTC &= ~(1 << TM_CLOCK);	// CLOCK LOW
	DDRC &= ~(1 << TM_DATA);     // (INPUT)
	_delay_us(100);
	ack=((PINC & (1 << TM_DATA)) > 0);
	PORTC |= (1 << TM_CLOCK);	// CLOCK HIGH
	_delay_us(100);
	PORTC &= ~(1 << TM_CLOCK);	// CLOCK LOW
	DDRC |= (1 << TM_DATA);     // (OUTPUT)

	return ack;
}

/*
uint8_t SevenSegmentTM1637::comReadByte(void) const {
  uint8_t readKey = 0;

  comStart();
  comWriteByte(TM1637_COM_SET_DATA | TM1637_SET_DATA_READ);
  comAck();

  pinAsInput(_pinDIO);
  digitalHigh(_pinDIO);
  delayMicroseconds(5);

  for ( uint8_t i=0; i < 8; i++) {

    readKey >>= 1;
    digitalLow(_pinClk);
    delayMicroseconds(30);

    digitalHigh(_pinClk);

    if ( isHigh(_pinDIO) ) {
      readKey = readKey | B1000000;
    };

    delayMicroseconds(30);


  };
  pinAsOutput(_pinDIO);
  comAck();
  comStop();
  return readKey;
};


*/

