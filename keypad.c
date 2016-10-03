#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

 
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

#define	KP_AL1 KP_R1C1

#define KEY_DOWN 0x01
#define KEY_UP 0x00

struct {
	uint8_t	scancode;
	char	name[6];
	uint16_t time;
	uint8_t  state;
} key;
void keypad(void)
{
	uint8_t val;
        pcf8574_write(0, 0x0f);
        val = pcf8574_read(0);
        pcf8574_write(0, 0xf0);
        val |= pcf8574_read(0);
}

