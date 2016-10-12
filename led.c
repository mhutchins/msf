#include <stdio.h>
#include <math.h>
#include "time.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

#include "max7219.h"
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
	int i;

	for(i=0;i<8;i++)
		max7219(MAX7219_DIGIT7 - i, led_framebuf[i][frame]);

	frame = (frame + 1) & 0x03;
}
