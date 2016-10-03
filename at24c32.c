#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

#include "i2cmaster.h"
 
#define AT24C32_ADDR 0x50

void at24c32_write(uint8_t unit, uint16_t addr, uint8_t data)
{
	i2c_start_wait((AT24C32_ADDR | unit)<<1 | 0x00);
	i2c_write((uint8_t)(addr>>8));
	i2c_write((uint8_t)(addr & 0xff));
	i2c_write(data);
	i2c_stop();
}

uint8_t at24c32_read(uint8_t unit, uint16_t addr)
{
        uint8_t datain;

	i2c_start_wait((AT24C32_ADDR | unit)<<1 | 0x00);
	i2c_write((uint8_t)(addr>>8));
	i2c_write((uint8_t)(addr & 0xff));
        i2c_start((AT24C32_ADDR | unit)<<1 | 0x01);
        datain = i2c_readNak();
        i2c_stop();

        return datain;
}

