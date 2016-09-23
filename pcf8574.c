#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

#include "i2cmaster.h"
 
void pcf8574_write(uint8_t addr, uint8_t data)
{

	i2c_start((0x38 | addr)<<1 | 0x00);
	i2c_write(data);
	i2c_stop();
}

uint8_t pcf8574_read(uint8_t addr)
{
        uint8_t datain;

        i2c_start((0x38 | addr)<<1 | 0x01);
        datain = i2c_readNak();
        i2c_stop();

        return datain;
}

