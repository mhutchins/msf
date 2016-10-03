#ifndef UNIXTIME_H
#define UNIXTIME_H

typedef long time_t;
typedef struct tm {
    int tm_sec;                 /* seconds after the minute -- [0,  59] */
    int tm_min;                 /* minutes after the hour   -- [0,  59] */
    int tm_hour;                /* hours since midnight     -- [0,  23] */
    int tm_mday;                /* day of the month         -- [1,  31] */
    int tm_mon;                 /* months since January     -- [0,  11] */
    int tm_year;                /* years since 1900         -- [      ] */
    int tm_wday;                /* day since Sunday         -- [0,   6] */
    int tm_yday;                /* day since January 1      -- [0, 365] */
    int tm_isdst;               /* daylight saving time flag            */
} tm_t;

uint8_t BcdToUint8(uint8_t val);
uint8_t BcdToBin24Hour(uint8_t bcdHour);

time_t mktime(struct tm *tp);

struct tm *gmtime(const time_t * timep);

#endif
