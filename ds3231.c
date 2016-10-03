#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

#include "unixtime.h"
#include "i2cmaster.h"
#include "util.h"
 
#define DS3231_ADDRESS 0x68

char dayname[][4]={"Err", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

void ds3231_write(uint8_t addr, uint8_t data)
{

	safe_i2c_start_wait(DS3231_ADDRESS<<1 | 0x00);
	i2c_write(addr);
	i2c_write(data);
	safe_i2c_stop();
}

uint8_t ds3231_read(uint8_t addr)
{
        uint8_t datain;

	safe_i2c_start_wait(DS3231_ADDRESS<<1 | 0x00);
	i2c_write(addr);
        i2c_rep_start(DS3231_ADDRESS<<1 | 0x01);
        datain = i2c_readNak();
        safe_i2c_stop();

        return datain;
}


struct tm * ds3231_readtime(void)
{
	static struct tm ds3231_tm;

	ds3231_tm.tm_sec = BcdToUint8(ds3231_read(0x00));
	ds3231_tm.tm_min = BcdToUint8(ds3231_read(0x01));
	ds3231_tm.tm_hour= BcdToBin24Hour(ds3231_read(0x02));
	ds3231_tm.tm_mday = BcdToUint8(ds3231_read(0x04));
	ds3231_tm.tm_mon = BcdToUint8(ds3231_read(0x05));
	ds3231_tm.tm_wday = ds3231_read(0x03);
	ds3231_tm.tm_year = BcdToUint8(ds3231_read(0x06));


/*
	AL1_mask=0;

	AL_tmp = ds3231_read(0x07);
	AL1_sec = BcdToUint8(AL_tmp & 0x7F);
	AL1_mask |= (AL_tmp & (1<<7))>>7;

	AL_tmp = ds3231_read(0x08);
	AL1_min = BcdToUint8(AL_tmp & 0x7F);
	AL1_mask |= (AL_tmp & (1<<6))>>6;

	AL_tmp = ds3231_read(0x09);
	AL1_hour = BcdToBin24Hour(AL_tmp & 0x7F);
	AL1_mask |= (AL_tmp & (1<<5))>>5;

	AL_tmp = ds3231_read(0x0A);
	AL1_day = BcdToUint8(AL_tmp & 0x7F);
	AL1_mask |= (AL_tmp & (1<<4))>>4;

	AL2_mask=0;

	AL_tmp = ds3231_read(0x0B);
	AL2_min = BcdToUint8(AL_tmp & 0x7F);
	AL2_mask |= (AL_tmp & (1<<6))>>6;

	AL_tmp = ds3231_read(0x0C);
	AL2_hour = BcdToBin24Hour(AL_tmp & 0x7F);
	AL2_mask |= (AL_tmp & (1<<5))>>5;

	AL_tmp = ds3231_read(0x0D);
	AL2_day = BcdToUint8(AL_tmp & 0x7F);
	AL2_mask |= (AL_tmp & (1<<4))>>4;

	ds3231_control = ds3231_read(0x0E);
	ds3231_status = ds3231_read(0x0F);
	ds3231_temp_degrees = ds3231_read(0x11);
	ds3231_temp_fract = (ds3231_read(0x11) >> 6) * 25;
        ds3231_temp_float = ds3231_temp_degrees += (float)ds3231_temp_fract / ((ds3231_temp_degrees < 0) ? -100.0f : 100.0f) ;

*/

/*
	fprintf(stderr, "%s %02d/%02d/%02d%02d %02d:%02d %02d\n", dayname[ds3231_tm.tm_wday], ds3231_tm.tm_mday, ds3231_tm.tm_mon, 20, ds3231_tm.tm_year, ds3231_tm.tm_hour, ds3231_tm.tm_min, ds3231_tm.tm_sec);
	fprintf(stderr, "AL1: %02d %02d:%02d %02d\n", AL1_day, AL1_hour, AL1_min, AL1_sec);
	fprintf(stderr, "AL2: %02d %02d:%02d \n", AL2_day, AL2_hour, AL2_min);
	fprintf(stderr, "Temperature: [%d / %d] %.2f\n", ds3231_temp_degrees, ds3231_temp_fract, ds3231_temp_float);
	fprintf(stderr, "         84218421\n");
	fprintf(stderr, "Control: ");
	binprint(ds3231_control, 8);
	fprintf(stderr, "\n");

	fprintf(stderr, " Status: ");
	binprint(ds3231_status, 8);
	fprintf(stderr, "\n");
*/
	return (&ds3231_tm);
}
