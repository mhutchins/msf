#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <util/atomic.h>
#include "i2cmaster.h"
#include "util.h"

uint8_t BcdToUint8(uint8_t val)
{
    return val - 6 * (val >> 4);
}

uint8_t BcdToBin24Hour(uint8_t bcdHour)
{
    uint8_t hour;
    if (bcdHour & 0x40)
    {
        // 12 hour mode, convert to 24
        bool isPm = ((bcdHour & 0x20) != 0);

        hour = BcdToUint8(bcdHour & 0x1f);
        if (isPm)
        {
           hour += 12;
        }
    }
    else
    {
        hour = BcdToUint8(bcdHour);
    }
    return hour;
}

void binprint(uint16_t data, uint8_t bits)
{
    int i;

    for (i = (bits-1); i >= 0; i--) {
       	fprintf(stderr, "%d", ((data & (1 << i)) > 0));
    }
}


static volatile uint8_t i2c_bus_busy=0;

void safe_i2c_stop(void)
{
	uint8_t flag=0;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (i2c_bus_busy == 1)
			i2c_bus_busy = 0;
		else
			flag=1;
	}

	if (flag == 1)
		fprintf(stderr, "SEMAPHORE FAIL - I2C BUS ACCESS!!\n");

	i2c_stop();
}

unsigned char safe_i2c_start(unsigned char addr)
{
	uint8_t flag=0;

	while(flag == 0)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			if (i2c_bus_busy == 0)
			{
				i2c_bus_busy = 1;
				flag=1;
			}
		}
	}

	return i2c_start(addr);
}

void safe_i2c_start_wait(unsigned char addr)
{
	uint8_t flag=0;

	while(flag == 0)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			if (i2c_bus_busy == 0)
			{
				i2c_bus_busy = 1;
				flag=1;
			}
		}
	}

	i2c_start_wait(addr);
	return;
}

