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

typedef union {
    struct {
//Byte 00
        uint8_t one_second:4;
        uint8_t ten_second:3;
         uint8_t:1;
//Byte 01
        uint8_t one_minute:4;
        uint8_t ten_minute:3;
         uint8_t:1;
//Byte 02
        uint8_t one_hour:4;
        uint8_t ten_hour:2;
        uint8_t twentyfourhour:1;
         uint8_t:1;
//Byte 03
        uint8_t dow:3;
         uint8_t:5;
//Byte 04
        uint8_t one_dom:4;
        uint8_t ten_dom:2;
         uint8_t:2;
//Byte 05
        uint8_t one_month:4;
        uint8_t ten_month:1;
         uint8_t:2;
        uint8_t century:1;
//Byte 06
        uint8_t one_year:4;
        uint8_t ten_year:4;
//Byte 07
        uint8_t spare_1:8;
        uint8_t spare_2:8;
    };
    uint8_t raw[10];
} __attribute__ ((__packed__)) packed_time;


typedef struct s_tm {
    union {
        struct {
            uint8_t stm_sec;
            uint8_t stm_min;
            uint8_t stm_hour;
            union {
                uint8_t stm_day;        // For 'time'/'alarm' 1(mon)-7 -- For 'alarm' 8=6&7, 9=1-5
                struct {
                    uint8_t stm_bf_day_mon:1;
                    uint8_t stm_bf_day_tue:1;
                    uint8_t stm_bf_day_wed:1;
                    uint8_t stm_bf_day_thu:1;
                    uint8_t stm_bf_day_fri:1;
                    uint8_t stm_bf_day_sat:1;
                    uint8_t stm_bf_day_sun:1;
                     uint8_t:1;
                };
            };
        };
        uint8_t raw[4];
    };
} s_tm_t;


uint8_t BcdToUint8(uint8_t val);
uint8_t BcdToBin24Hour(uint8_t bcdHour);

time_t mktime(struct tm *tp);

struct tm *gmtime(const time_t * timep);

#endif
