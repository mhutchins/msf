#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <time.h>
#include <util/atomic.h>
#include "i2cmaster.h"
#include "util.h"
#include "main.h"
#include "at24c32.h"

char dayname[][4]={"Err", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

#define AT24_ALARM_BASE_ADDR 0x10

uint8_t Uint8ToBcd(uint8_t ival)
{
 	return ((ival / 10) << 4) | (ival % 10);
}
 
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

void writealarm(uint8_t idx)
{
	uint8_t *ptr=(uint8_t *)&alarm_time[idx];

	uint8_t i;

	for (i=0;i<sizeof(packed_time);i++)
	    at24c32_write(7, AT24_ALARM_BASE_ADDR + i + (idx * sizeof(packed_time)), *ptr++);
}

void readalarm(uint8_t idx)
{
	uint8_t *ptr=(uint8_t *)&alarm_time[idx];

	uint8_t i;

	for (i=0;i<sizeof(packed_time);i++)
	    *(ptr++)=at24c32_read(7, AT24_ALARM_BASE_ADDR + i + (idx * sizeof(packed_time)));

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

