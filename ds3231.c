#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

#include "i2cmaster.h"
 
#define DS3231_ADDRESS 0x68

void ds3231_write(uint8_t addr, uint8_t data)
{

	i2c_start(DS3231_ADDRESS<<1 | 0x00);
	i2c_write(data);
	i2c_stop();
}

uint8_t ds3231_read(uint8_t addr)
{
        uint8_t datain;

	i2c_start(DS3231_ADDRESS<<1 | 0x00);
	i2c_write(addr);
        i2c_rep_start(DS3231_ADDRESS<<1 | 0x01);
        datain = i2c_readNak();
        i2c_stop();

        return datain;
}

