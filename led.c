#include <stdio.h>
#include <math.h>
#include "unixtime.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

#include "max7219.h"
#include "led.h"

volatile uint8_t led_framebuf[8][4];

void update_led(void)
{
	static uint8_t frame=0;
	int i;

	for(i=0;i<8;i++)
		max7219(MAX7219_DIGIT7 - i, led_framebuf[i][frame]);

	frame = (frame + 1) & 0x03;
}
