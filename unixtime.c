#include <stdint.h>
#include <stdbool.h>
#include "unixtime.h"

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

time_t mktime(struct tm * tp)
{
    register long days;
    tp->tm_yday = ((tp->tm_mon * 367) + 5) / 12 + (tp->tm_mday)
        - 1;
    tp->tm_yday -= (tp->tm_mon > 1) ? ((tp->tm_year % 4) ? 2 : 1) : 0;
    days = (((long) (tp->tm_year) * 1461) + 3) / 4 + tp->tm_yday;
    tp->tm_wday = days % 7;
    return (time_t) (((days - 25568) * (24L * 60 * 60))
                     + ((long) tp->tm_hour * (3600))
                     + (tp->tm_min * 60)
                     + (tp->tm_sec));
}

struct tm *gmtime(const time_t * timep)
{
    static struct tm tmbuf;
    register struct tm *tp = &tmbuf;
    time_t time = *timep;
    register long day, mins, secs, year, leap;
    day = time / (24L * 60 * 60);
    secs = time % (24L * 60 * 60);
    tp->tm_sec = secs % 60;
    mins = secs / 60;
    tp->tm_hour = mins / 60;
    tp->tm_min = mins % 60;
    tp->tm_wday = (day + 4) % 7;
    year = (((day * 4) + 2) / 1461);
    tp->tm_year = year + 70;
    leap = !(tp->tm_year & 3);
    day -= ((year * 1461L) + 1) / 4;
    tp->tm_yday = day;
    day += (day > 58 + leap) ? ((leap) ? 1 : 2) : 0;
    tp->tm_mon = ((day * 12) + 6) / 367;
    tp->tm_mday = day + 1 - ((tp->tm_mon * 367) + 5) / 12;
    tp->tm_isdst = 0;
    return (tp);
}
