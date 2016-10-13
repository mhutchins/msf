#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <string.h>

#include "time.h"
#include "ds3231.h"
#include "i2cmaster.h"
#include "util.h"
#include "main.h"
 
#define DS3231_ADDRESS 0x68

struct tm  ds3231_tm;

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

void ds3231_writetime(time_t time)
{
	gmtime_r(&time, &ds3231_tm);

	ds3231_write(0x00, 0x00); // Seconds
	ds3231_write(0x01, (Uint8ToBcd(ds3231_tm.tm_min) & 0x7f)); // Minutes
	ds3231_write(0x02, (Uint8ToBcd(ds3231_tm.tm_hour) & 0x3f)); // Hours - Force 24 hour
	ds3231_write(0x03, (Uint8ToBcd(ds3231_tm.tm_wday-1) & 0x07)); // Weekday
	ds3231_write(0x04, (Uint8ToBcd(ds3231_tm.tm_mday) & 0x3f)); // Day of month
	ds3231_write(0x05, (Uint8ToBcd(ds3231_tm.tm_mon) & 0x3f) | ( (ds3231_tm.tm_year > 100) << 7) ); // Month / Century
	ds3231_write(0x06, (Uint8ToBcd(ds3231_tm.tm_year%100)) ); // year
}
time_t ds3231_readtime(void)
{
	time_t retval;
	uint8_t val;

	ds3231_tm.tm_sec = BcdToUint8(ds3231_read(0x00));
	ds3231_tm.tm_min = BcdToUint8(ds3231_read(0x01));
	ds3231_tm.tm_hour= BcdToBin24Hour(ds3231_read(0x02));
	ds3231_tm.tm_wday = ds3231_read(0x03);
	ds3231_tm.tm_mday = BcdToUint8(ds3231_read(0x04));
	val=ds3231_read(0x05);
	ds3231_tm.tm_mon = BcdToUint8(val & 0x1f);
	//ds3231_tm.tm_year = (((val & 0x80) > 0) * 100) + BcdToUint8(ds3231_read(0x06));
	ds3231_tm.tm_year =  100 + BcdToUint8(ds3231_read(0x06));
	ds3231_tm.tm_isdst = 0;

	//fprintf(stderr, "%s %02d/%02d/%02d %02d:%02d %02d\n", dayname[ds3231_tm.tm_wday-1], ds3231_tm.tm_mday, ds3231_tm.tm_mon, 1900+ds3231_tm.tm_year, ds3231_tm.tm_hour, ds3231_tm.tm_min, ds3231_tm.tm_sec);

	retval=mktime(&ds3231_tm);
	fprintf(&display_lcd, "%02d:%02d %02d\n", ds3231_tm.tm_hour, ds3231_tm.tm_min, ds3231_tm.tm_sec);
	return retval;
}
