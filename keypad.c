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
#include "ds3231.h"

#include "lcd.h"

#include "main.h"
#include "util.h"
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
#define	KP_NONE	0xff

#define KEY_DOWN 0x01
#define KEY_UP 0x00

#define STMODE_CLOCK 1
#define STMODE_ALARM1 2
#define STMODE_ALARM2 3

struct {
	uint8_t	scancode;
	char	name[6];
	uint16_t time;
	uint8_t  state;
} key;

typedef enum {
	SETITEM_NONE,
	SETITEM_MIN,
	SETITEM_HOUR,
	SETITEM_DOW,
	SETITEM_DOM,
	SETITEM_MONTH,
	SETITEM_YEAR
} SetItems;

typedef enum {
		ST_IDLE,
		ST_SET,
		ST_AL1,
		ST_AL2,
		ST_SET_AL1,
		ST_SET_AL2,
		ST_SET_TIME,
		ST_DOSET,
		ST_DOSETWAIT
} States;

#define KEY_TIMEOUT	100

void keypad(void)
{
	static uint8_t	old_scancode=0;
	static States state=ST_IDLE;
	static States next_state=ST_IDLE;
	static uint16_t key_ticks=0;
	static uint16_t state_time=0;
	static uint8_t kp_repeat=0;
	static uint8_t set_source;
	static SetItems set_item;
	uint8_t kp_delay;

	key_ticks++;

	uint8_t kp_val;

	uint8_t scancode=0;
	static uint8_t kp_threshold;

        pcf8574_write(0, 0x0f);
        scancode = pcf8574_read(0);
        pcf8574_write(0, 0xf0);
        scancode |= pcf8574_read(0);

	
	kp_val=0;

	kp_delay=20;
	if (kp_repeat > 10)
		kp_delay = 5;
	if (kp_repeat > 40)
		kp_delay = 1;
	if (kp_repeat > 80)
		kp_delay = 0;

	if (kp_threshold-- == 0)
	{
		kp_threshold=kp_delay;
		kp_val=1;
	}


	if (scancode != old_scancode)	
	{
		kp_repeat=0;
		old_scancode = scancode;
		kp_val=1;
	}
	else
		kp_repeat++;

	
	if (key_ticks - state_time > KEY_TIMEOUT && state != ST_IDLE)
	{
		state=ST_IDLE;
		fprintf(stderr, "Timeout!\n");
	}

	if (state != ST_IDLE)
		fprintf(stderr, "[0x%02x] [%d]-[%d]=%d \n", scancode, key_ticks, state_time, (key_ticks-state_time));
	switch(state)
	{
		case ST_IDLE:	
			if (scancode == KP_SET)
			{
				state_time=key_ticks;
				state=ST_SET;
			}

			if (scancode == KP_AL1)
			{
				state_time=key_ticks;
				state=ST_AL1;
			}

			if (scancode == KP_AL2)
			{
				state_time=key_ticks;
				state=ST_AL2;
			}

		        set_led(0, (rtc_time.ten_hour)+'0', 0);
		        set_led(1, (rtc_time.one_hour)+'0', 0);
		        set_led(2, (rtc_time.ten_minute)+'0', 0);
		        set_led(3, (rtc_time.one_minute)+'0', 0);

		        set_led(4, (rtc_time.ten_second)+'0', 0);
		        set_led(5, (rtc_time.one_second)+'0', 0);

			break;

		case ST_AL1:
		        set_led(0, (alarm_time[0].stm_hour/10)+'0', 0);
		        set_led(1, (alarm_time[0].stm_hour%10)+'0', 0);
		        set_led(2, (alarm_time[0].stm_min/10)+'0', 0);
		        set_led(3, (alarm_time[0].stm_min%10)+'0', 0);
			next_state=ST_IDLE;
			state=ST_DOSETWAIT;
			break;

		case ST_AL2:
		        set_led(0, (alarm_time[1].stm_hour/10)+'0', 0);
		        set_led(1, (alarm_time[1].stm_hour%10)+'0', 0);
		        set_led(2, (alarm_time[1].stm_min/10)+'0', 0);
		        set_led(3, (alarm_time[1].stm_min%10)+'0', 0);
			next_state=ST_IDLE;
			state=ST_DOSETWAIT;
			break;
		case ST_SET:
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
		        set_led(0, 'A', 1);
		        set_led(1, '1', 1);
		        set_led(2, ' ', 0);
		        set_led(3, ' ', 0);
			set_source=STMODE_ALARM1;
			set_item=SETITEM_NONE;
			memset(&set_time, 0, (sizeof(packed_time)));
			set_time.ten_hour = alarm_time[0].stm_hour/10;
			set_time.one_hour = alarm_time[0].stm_hour%10;
			set_time.ten_minute = alarm_time[0].stm_min/10;
			set_time.one_minute = alarm_time[0].stm_min%10;
			state = ST_DOSET;
			break;
		case ST_SET_AL2:
			state_time=key_ticks;
		        set_led(0, 'A', 1);
		        set_led(1, '2', 1);
		        set_led(2, ' ', 0);
		        set_led(3, ' ', 0);
			set_source=STMODE_ALARM2;
			set_item=SETITEM_NONE;
			memset(&set_time, 0, (sizeof(packed_time)));
			set_time.ten_hour = alarm_time[1].stm_hour/10;
			set_time.one_hour = alarm_time[1].stm_hour%10;
			set_time.ten_minute = alarm_time[1].stm_min/10;
			set_time.one_minute = alarm_time[1].stm_min%10;
			state = ST_DOSET;
			break;
		case ST_SET_TIME:
			state_time=key_ticks;
		        set_led(0, 'C', 1);
		        set_led(1, 'l', 1);
		        set_led(2, ' ', 0);
		        set_led(3, ' ', 0);
			set_source=STMODE_CLOCK;
			set_item=SETITEM_NONE;
			memcpy(&set_time, &rtc_time, (sizeof(packed_time)));
			state = ST_DOSET;
			break;
		case ST_DOSET:
			switch(scancode)
			{
				case KP_HOUR:
					state_time=key_ticks;
					if (set_item != SETITEM_HOUR)
					{
						set_item=SETITEM_HOUR;
						next_state=ST_DOSET;
						state=ST_DOSETWAIT;
					}
					else
					{
						uint8_t hour = (set_time.ten_hour * 10 ) + (set_time.one_hour);
						hour = hour + kp_val ;
						if ( hour > 23 ) hour = 0;
						set_time.ten_hour = hour/10;
						set_time.one_hour = hour%10;
					}

					break;

				case KP_MIN:
					state_time=key_ticks;
					if (set_item != SETITEM_MIN)
					{
						set_item=SETITEM_MIN;
						next_state=ST_DOSET;
						state=ST_DOSETWAIT;
					}
					else
					{
						uint8_t min = (set_time.ten_minute * 10 ) + (set_time.one_minute);
						min = min + kp_val ;
						if ( min > 59 ) min = 0;
						set_time.ten_minute = min/10;
						set_time.one_minute = min%10;
					}
					break;
				case KP_DAY:
					state_time=key_ticks;
					if (set_item != SETITEM_DOW)
					{
						set_item=SETITEM_DOW;
						next_state=ST_DOSET;
						state=ST_DOSETWAIT;
					}
					else
					{
						uint8_t day = set_time.dow;
						day = day + kp_val ;
						if ( day > 6 ) day = 0;
						set_time.dow = day;
					}
					break;
				case KP_SET:
					if (set_source == STMODE_CLOCK)
					{
						fprintf(stderr, "Setting clock..\n");
						set_time.ten_second=0;
						set_time.one_second=0;
						memcpy(&rtc_time, &set_time, (sizeof(packed_time)));
						ds3231_writetime(&rtc_time);
					}
					if (set_source == STMODE_ALARM1)
					{
						fprintf(stderr, "Setting Alarm 1..\n");
						alarm_time[0].stm_sec=0;
						alarm_time[0].stm_min=(set_time.ten_minute*10)+(set_time.one_minute);
						alarm_time[0].stm_hour=(set_time.ten_hour*10)+(set_time.one_hour);
						alarm_time[0].stm_day=set_time.dow;
						writealarm(0);
					}
					if (set_source == STMODE_ALARM2)
					{
						fprintf(stderr, "Setting Alarm 2..\n");
						alarm_time[1].stm_sec=0;
						alarm_time[1].stm_min=(set_time.ten_minute*10)+(set_time.one_minute);
						alarm_time[1].stm_hour=(set_time.ten_hour*10)+(set_time.one_hour);
						alarm_time[1].stm_day=set_time.dow;
						writealarm(1);
					}
					next_state=ST_IDLE;
					state=ST_DOSETWAIT;
					break;
			}

			switch(set_item)
			{
				case SETITEM_HOUR:
					set_led(0, 'H', 1);
					set_led(1, 'r', 1);
					set_led(2, ((set_time.ten_hour) + '0'), 0);
					set_led(3, ((set_time.one_hour) + '0'), 0);
					break;

				case SETITEM_MIN:
					set_led(0, 'N', 1);
					set_led(1, 'n', 1);
					set_led(2, ((set_time.ten_minute) + '0'), 0);
					set_led(3, ((set_time.one_minute) + '0'), 0);
					break;

				case SETITEM_DOW:
					set_led(0, 'd', 1);
					set_led(1, 'y', 1);
					set_led(2, ((set_time.dow) + '0'), 0);
					set_led(3, ' ', 0);
					break;

				default:
					break;
			}
			break;
		case ST_DOSETWAIT:
			state_time=key_ticks;
			if (scancode == KP_NONE)
				state=next_state;
			break;
		default:
			fprintf(stderr, "Illegal state: %d\n", state);
	}

}

