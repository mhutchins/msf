#include <stdio.h>
#include <math.h>
#include "time.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "msf.h"
#include "max7219.h"
#include "pcf8574.h"
#include "keypad.h"

#include "lcd.h"

#include "main.h"
#include "led.h"

 
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

#define STMODE_CLOCK 1
#define STMODE_ALARM1 2
#define STMODE_ALARM2 3

#define SETITEM_NONE 0
#define SETITEM_HOUR 1
#define SETITEM_MIN 2

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
		ST_DOSET
} States;

#define KEY_TIMEOUT	100

void keypad(void)
{
	static uint8_t	old_scancode=0;
	static States state=ST_IDLE;
	static uint16_t key_ticks=0;
	static uint16_t state_time=0;
	static uint8_t repeat=0;
	static struct tm temp_time;
	static uint8_t set_source;
	static uint8_t set_item;

	key_ticks++;

	uint8_t scancode=0;
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
			if (scancode == KP_SET)
			{
				state_time=key_ticks;
				state=ST_SET;
			}
			gmtime_r(clock_time, &temp_time);

		        set_led(0, (temp_time.tm_hour/10)+'0', 0);
		        set_led(1, (temp_time.tm_hour%10)+'0', 0);
		        set_led(2, (temp_time.tm_min/10)+'0', 0);
		        set_led(3, (temp_time.tm_min%10)+'0', 0);

			break;
		case ST_SET:
			fprintf(stderr, "SET: ");
			if (scancode == KP_AL1)
			{
				state_time=key_ticks;
				state=ST_SET_AL1;
				break;
			}
		        set_led(0, 'S', 1);
		        set_led(1, 't', 1);
		        set_led(2, ' ', 0);
		        set_led(3, ' ', 0);
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
			state_time=key_ticks;
			fprintf(stderr, "SET AL1: ");
		        set_led(0, 'A', 1);
		        set_led(1, '1', 1);
		        set_led(2, ' ', 0);
		        set_led(3, ' ', 0);
			set_source=STMODE_ALARM1;
			set_item=SETITEM_NONE;
			gmtime_r(&alarm_time[0], &temp_time);
			state = ST_DOSET;
			break;
		case ST_SET_AL2:
			state_time=key_ticks;
			fprintf(stderr, "SET AL2: ");
		        set_led(0, 'A', 1);
		        set_led(1, '2', 1);
		        set_led(2, ' ', 0);
		        set_led(3, ' ', 0);
			set_source=STMODE_ALARM2;
			set_item=SETITEM_NONE;
			gmtime_r(&alarm_time[1], &temp_time);
			state = ST_DOSET;
			break;
		case ST_SET_TIME:
			state_time=key_ticks;
			fprintf(stderr, "SET TIME: ");
		        set_led(0, 'C', 1);
		        set_led(1, 'l', 1);
		        set_led(2, ' ', 0);
		        set_led(3, ' ', 0);
			set_source=STMODE_CLOCK;
			set_item=SETITEM_NONE;
			gmtime_r(clock_time, &temp_time);
			state = ST_DOSET;
			break;
		case ST_DOSET:
			fprintf(stderr, "Divisor: %d", divisor);
			switch(scancode)
			{
				case KP_HOUR:
					state_time=key_ticks;
					if (set_item != SETITEM_HOUR)
						set_item=SETITEM_HOUR;
					else
						temp_time.tm_hour = (temp_time.tm_hour < 23 ? temp_time.tm_hour + 1 : 0);
					break;

				case KP_MIN:
					state_time=key_ticks;
					if (set_item != SETITEM_MIN)
						set_item=SETITEM_MIN;
					else
						temp_time.tm_min = (temp_time.tm_min < 58 ? temp_time.tm_min + 1 : 0);
					break;
				case KP_SET:
					if (set_source == STMODE_CLOCK)
						fprintf(stderr, "Setting clock..\n");
					if (set_source == STMODE_ALARM1)
						fprintf(stderr, "Setting Alarm 1..\n");
					if (set_source == STMODE_ALARM2)
						fprintf(stderr, "Setting Alarm 2..\n");
					state=ST_IDLE;
					break;
			}

			switch(set_item)
			{
				case SETITEM_HOUR:
					set_led(0, 'H', 1);
					set_led(1, 'r', 1);
					set_led(2, ((temp_time.tm_hour / 10) + '0'), 0);
					set_led(3, ((temp_time.tm_hour % 10) + '0'), 0);
					break;

				case SETITEM_MIN:
					set_led(0, 'N', 1);
					set_led(1, 'n', 1);
					set_led(2, ((temp_time.tm_min / 10) + '0'), 0);
					set_led(3, ((temp_time.tm_min % 10) + '0'), 0);
					break;
			}
			break;
		default:
			fprintf(stderr, "Illegal state: %d", state);
	}

	fprintf(stderr, "\n");
}

