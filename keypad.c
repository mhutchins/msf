#include <stdio.h>
#include <math.h>
#include "unixtime.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdlib.h>
#include "msf.h"
#include "max7219.h"
#include "pcf8574.h"

#include "lcd.h"

 
#define	KP_R1C1	0x7e
#define	KP_R2C1	0x7d
#define	KP_R3C1	0x7b
#define	KP_R4C1	0x77

#define	KP_R1C2	0xbe
#define	KP_R2C2	0xbd
#define	KP_R3C2	0xbb
#define	KP_R4C2	0xb7

#define	KP_R1C3	0xde
#define	KP_R2C3	0xdd
#define	KP_R3C3	0xdb
#define	KP_R4C3	0xd7

#define	KP_R1C4	0xee
#define	KP_R2C4	0xed
#define	KP_R3C4	0xeb
#define	KP_R4C4	0xe7

#define	KP_AL1	KP_R1C1
#define	KP_AL2	KP_R1C2
#define	KP_SET	KP_R1C3
#define	KP_DAY	KP_R2C1
#define	KP_HOUR KP_R2C2
#define	KP_MIN	KP_R2C3
#define	KP_RAD	KP_R3C1
#define	KP_SLP	KP_R3C2
#define	KP_BUZ	KP_R3C3

#define KEY_DOWN 0x01
#define KEY_UP 0x00


extern time_t *clock_time;

struct {
	uint8_t	scancode;
	char	name[6];
	uint16_t time;
	uint8_t  state;
} key;

typedef enum {
		ST_IDLE,
		ST_SET,
		ST_SET_AL1,
		ST_SET_AL2,
		ST_SET_TIME,
		ST_GET_SET
} States;

#define KEY_TIMEOUT	100

void keypad(void)
{
	static uint8_t	old_scancode=0;
	static States state=ST_IDLE;
	static uint16_t key_ticks=0;
	static uint16_t state_time=0;
	static uint8_t repeat=0;

	key_ticks++;

	uint8_t scancode=0;
	uint8_t inc_value;
	uint8_t divisor;

        pcf8574_write(0, 0x0f);
        scancode = pcf8574_read(0);
        pcf8574_write(0, 0xf0);
        scancode |= pcf8574_read(0);

	if (scancode != old_scancode)	
	{
		repeat=0;
		old_scancode = scancode;
	}
	else
		repeat++;

	
	if (repeat == 0)
		divisor=10;
	if (repeat > 10)
		divisor=5;
	if (repeat > 20)
		divisor=1;

	inc_value=(repeat%divisor) == 0;

	if (key_ticks - state_time > KEY_TIMEOUT && state != ST_IDLE)
	{
		state=ST_IDLE;
		fprintf(stderr, "Timeout!\n");
	}

	if (state != ST_IDLE)
		fprintf(stderr, "[0x%02x] [%d]-[%d]=%d ", scancode, key_ticks, state_time, (key_ticks-state_time));
	switch(state)
	{
		case ST_IDLE:	
			fprintf(stderr, "IDLE: ");
			if (scancode == KP_R2C1)
				LCD_write_byte(mode_WR|mode_CMD, 0x18, 0);
			if (scancode == KP_R2C2)
				LCD_write_byte(mode_WR|mode_CMD, 0x1C, 0);
			if (scancode == KP_R3C1)
				LCD_write_byte(mode_WR|mode_CMD, 0x1A, 0);
			if (scancode == KP_R3C2)
				LCD_write_byte(mode_WR|mode_CMD, 0x1E, 0);
			if (scancode == KP_SET)
			{
				state_time=key_ticks;
				state=ST_SET;
			}
			displaytime(&msf_time[0]);
			displaytime(&msf_time[1]);
			if (abs(msf_time[0] - msf_time[1]) == 60)
				fprintf(stderr, " GOOD\n");
			break;
		case ST_SET:
			fprintf(stderr, "SET: ");
			if (scancode == KP_AL1)
			{
				state_time=key_ticks;
				state=ST_SET_AL1;
				break;
			}
			if (scancode == KP_AL2)
			{
				state_time=key_ticks;
				state=ST_SET_AL2;
				break;
			}
			if (scancode == KP_SLP)
			{
				state_time=key_ticks;
				state=ST_SET_TIME;
				break;
			}
			break;
		case ST_SET_AL1:
			fprintf(stderr, "SET AL1: ");
			break;
		case ST_SET_AL2:
			fprintf(stderr, "SET AL2: ");
			break;
		case ST_SET_TIME:
			fprintf(stderr, "SET TIME: ");
			break;
		default:
			fprintf(stderr, "Illegal state: %d", state);
	}

	fprintf(stderr, "\n");
}

