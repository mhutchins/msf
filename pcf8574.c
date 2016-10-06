#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include "pcf8574.h"

#include "util.h"

#include "i2cmaster.h"
 
void pcf8574_write(uint8_t addr, uint8_t data)
{

	safe_i2c_start((0x38 | addr)<<1 | 0x00);
	i2c_write(data);
	safe_i2c_stop();
}

uint8_t pcf8574_read(uint8_t addr)
{
        uint8_t datain;

        safe_i2c_start((0x38 | addr)<<1 | 0x01);
        datain = i2c_readNak();
        safe_i2c_stop();

        return datain;
}

