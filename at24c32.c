#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

#include "i2cmaster.h"
#include "util.h"
#include "at24c32.h"
 
#define AT24C32_ADDR 0x50

void at24c32_write(uint8_t unit, uint16_t addr, uint8_t data)
{
	safe_i2c_start_wait((AT24C32_ADDR | unit)<<1 | 0x00);
	i2c_write((uint8_t)(addr>>8));
	i2c_write((uint8_t)(addr & 0xff));
	i2c_write(data);
	safe_i2c_stop();
}

uint8_t at24c32_read(uint8_t unit, uint16_t addr)
{
        uint8_t datain;

	safe_i2c_start_wait((AT24C32_ADDR | unit)<<1 | 0x00);
	i2c_write((uint8_t)(addr>>8));
	i2c_write((uint8_t)(addr & 0xff));
        i2c_rep_start((AT24C32_ADDR | unit)<<1 | 0x01);
        datain = i2c_readNak();
        safe_i2c_stop();

        return datain;
}

