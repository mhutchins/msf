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

void ds3231_writetime(packed_time *tm)
{
	uint8_t *ptr = (uint8_t *) tm;

	ds3231_write(0x00, *ptr++);
	ds3231_write(0x01, *ptr++);
	ds3231_write(0x02, *ptr++);
	ds3231_write(0x03, *ptr++);
	ds3231_write(0x04, *ptr++);
	ds3231_write(0x05, *ptr++);
	ds3231_write(0x06, *ptr++);
/********
	ds3231_write(0x00, 0x00); // Seconds
	ds3231_write(0x01, (Uint8ToBcd(tm->tm_min) & 0x7f)); // Minutes
	ds3231_write(0x02, (Uint8ToBcd(tm->tm_hour) & 0x3f)); // Hours - Force 24 hour
	ds3231_write(0x03, (Uint8ToBcd(tm->tm_wday-1) & 0x07)); // Weekday
	ds3231_write(0x04, (Uint8ToBcd(tm->tm_mday) & 0x3f)); // Day of month
	ds3231_write(0x05, (Uint8ToBcd(tm->tm_mon) & 0x3f) | ( (tm->tm_year > 100) << 7) ); // Month / Century
	ds3231_write(0x06, (Uint8ToBcd(tm->tm_year%100)) ); // year
*/
}

void ds3231_readtime(packed_time *tm)
{
	uint8_t *ptr = (uint8_t *)tm;

	*ptr++ = ds3231_read(0x00);
	*ptr++ = ds3231_read(0x01);
	*ptr++ = ds3231_read(0x02);
	*ptr++ = ds3231_read(0x03);
	*ptr++ = ds3231_read(0x04);
	*ptr++ = ds3231_read(0x05);
	*ptr++ = ds3231_read(0x06);

/*
	fprintf(stderr, "%s %02d/%02d/%02d %02d:%02d %02d\n", dayname[tm->dow],
		(tm->ten_dom * 10) + tm->one_dom,
		(tm->ten_month * 10) + tm->one_month,
		(tm->ten_year * 10) + tm->one_year,
		(tm->ten_hour * 10) + tm->one_hour,
		(tm->ten_minute * 10) + tm->one_minute,
		(tm->ten_second * 10) + tm->one_second);
*/



/*
	uint8_t val;

	tm->tm_sec = BcdToUint8(ds3231_read(0x00));
	tm->tm_min = BcdToUint8(ds3231_read(0x01));
	tm->tm_hour= BcdToBin24Hour(ds3231_read(0x02));
	tm->tm_wday = ds3231_read(0x03);
	tm->tm_mday = BcdToUint8(ds3231_read(0x04));
	val=ds3231_read(0x05);
	tm->tm_mon = BcdToUint8(val & 0x1f);
	//tm->tm_year = (((val & 0x80) > 0) * 100) + BcdToUint8(ds3231_read(0x06));
	tm->tm_year =  100 + BcdToUint8(ds3231_read(0x06));
	tm->tm_isdst = 0;


	fprintf(&display_lcd, "%02d:%02d %02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
*/
}
