#include <stdio.h>
#include <math.h>
#include "time.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

#include "max7219.h"
#include "tm1637.h"
#include "led.h"

volatile uint8_t led_framebuf[8][4];

void set_led(uint8_t idx, char ch, uint8_t blink)
{
	uint8_t led_patt=getled(ch);
	uint8_t i;

	for(i=0; i<4;i++)
	{
		if (i >= blink)
			led_framebuf[idx][i]=led_patt;
		else
			led_framebuf[idx][i]=0;
	}
}
void update_led(void)
{
	static uint8_t frame=0;
	uint8_t temp[4];
	int i;

	for(i=0;i<8;i++)
		max7219(MAX7219_DIGIT7 - i, led_framebuf[i][frame]);

	temp[0]=led_fixup(led_framebuf[0][frame]);
	temp[1]=led_fixup(led_framebuf[1][frame]);
	temp[2]=led_fixup(led_framebuf[2][frame]);
	temp[3]=led_fixup(led_framebuf[3][frame]);
	tm_update(temp);

	frame = (frame + 1) & 0x03;
}

uint8_t led_fixup(uint8_t val)
{
	uint8_t retval;

	retval = \
		((val & 0x01)>0) << 6 | \
		((val & 0x02)>0) << 5 | \
		((val & 0x04)>0) << 4 | \
		((val & 0x08)>0) << 3 | \
		((val & 0x10)>0) << 2 | \
		((val & 0x20)>0) << 1 | \
		((val & 0x40)>0) << 0 | \
		((val & 0x80)>0) << 7 \
	;
	return retval;
}
