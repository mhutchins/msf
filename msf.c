#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "time.h"
#include "msf.h"
#include "lcd.h"
#include "util.h"
#include <string.h>
#include "led.h"
#include "ds3231.h"

#define DEBUG 1

//                        2222222211111111
//                        4444333322221111
//                        8421842184218421
//#define MIN_MASK	__extension__ 0b0001111101111100
//#define MIN_PATT	__extension__ 0b0001111100000100
#define MIN_MASK        __extension__ 0b0001111110111111
#define MIN_PATT        __extension__ 0b0000000000111110


#define SEC_MASK	__extension__ 0b0001000111111100
#define SEC_PATT	__extension__ 0b0001000000000100
                          //___1_________1
                          //0001..000000k
                          //00000000001111111100
                          //00000000000000000100

#define MSF_SIZE	(64/8)

//#define TRACK	1

volatile uint16_t offset = 0x9e58;
//volatile uint16_t offset = 0xb1cf;
//volatile uint16_t offset = 0x63a0;
//volatile uint16_t offset = 0x6400;
volatile uint16_t tick = 0;
volatile uint8_t sync_flag = 0;
volatile uint8_t sec = 0;
volatile int hour, min, mon, dom, dow, year;

uint8_t msf_bit_a[MSF_SIZE];
uint8_t msf_bit_b[MSF_SIZE];
packed_time msf_time[2];

void timer_init(void)
{
    TCCR1A = 0;			// set entire TCCR1A register to 0
    TCCR1B = 0;			// set entire TCCR1B register to 0 
    // (as we do not know the initial  values) 
    //
    // // enable Timer1 overflow interrupt:
    // TIMSK1 | = (1 << TOIE1); //Atmega8 has no TIMSK1 but a TIMSK register
    //
    TIMSK1 |= (1 << TOIE1);
    TCCR1B |= (1 << CS11);
    TCNT1 = offset;

    //TCCR0B |= (1 << CS02) | (1 << CS01 ) | (1 << CS00);
}

#define SAMPLES	8

ISR(TIMER1_OVF_vect, ISR_BLOCK) {
    static uint8_t avg = 0;
    static uint8_t avg_shift = 0;
    static uint8_t last_high_avg_shift = 0;
    static uint8_t sample_idx = 0;
    static uint16_t history = 0;
    static uint8_t e_hi=0;
    static uint8_t e_lo=0;
    static uint8_t decode_idx=0;
    uint8_t state = 0;
    static uint8_t led_tick=0;

    // Set the timer overflow for the next ineterrupt
    TCNT1 = offset;

    // Grab the current state of the MSF input
    state = ((PIND & (1 << PD2)) > 0);

    PORTD = PORTD ^ (1 << PD3);        // LO
    tick++;

    if(led_tick++ % 5 == 0)
	update_led();


    avg = avg + state;
    avg_shift = avg_shift << 1 | state ;

    sample_idx = (sample_idx + 1) & 0x07;
    if (sample_idx == 0) {

	PORTD = PORTD ^ (1 << PD4);	// Toggle PD4

	history = history << 1;

	if (avg >= (SAMPLES / 2))
	{
	    last_high_avg_shift = avg_shift;
	    history |= 0x01;
	    e_hi = SAMPLES-avg;
	}
	else
	{
	    e_lo = avg;
	}

#ifdef TRACK
	binprint(history, 16);
#endif

	if ((history & MIN_MASK) == MIN_PATT) {
		PORTD = PORTD ^ (1 << PD5);        // LO
		fprintf(stderr, "\tSSSSSSS");
		fprintf(stderr, "A: ");
		binprint(avg_shift, 8);
		fprintf(stderr, "LA: ");
		binprint(last_high_avg_shift, 8);
		decode(&msf_time[decode_idx]);
		if (decode(&msf_time[decode_idx]) == 0)
		{
			if (
				msf_time[0].ten_minute == msf_time[1].ten_minute && \
				msf_time[0].one_minute == msf_time[1].one_minute && \
				msf_time[0].ten_hour == msf_time[1].ten_hour && \
				msf_time[0].one_hour == msf_time[1].one_hour && \
				msf_time[0].one_dom == msf_time[1].one_dom && \
				msf_time[0].ten_dom == msf_time[1].ten_dom && \
				msf_time[0].one_month == msf_time[1].one_month && \
				msf_time[0].ten_month == msf_time[1].ten_month && \
				msf_time[0].one_year == msf_time[1].one_year && \
				msf_time[0].ten_year == msf_time[1].ten_year )
			fprintf(stderr, "Storing MSF time!\n");
			ds3231_writetime(&msf_time[decode_idx]);
		}

		decode_idx=1 - decode_idx;

		tick=24;
#ifndef TRACK
		fprintf(stderr, "\n");
#endif
	} else {

	    //if ((history & SEC_MASK) == SEC_PATT && (tick%80 != 0))
	//	fprintf(stderr, "Sync miss: %d\n", tick%80);
	    if ((history & SEC_MASK) == SEC_PATT && (tick%80 == 0)) {
		sec = (tick/80)-1;
		sec = sec % 60;
		set_msf_bit(msf_bit_a, sec,
			    ((history & __extension__ 0b0000100000000000) > 0));
		set_msf_bit(msf_bit_b, sec,
			    ((history & __extension__ 0b0000010000000000) > 0));
		fprintf(stderr, "\t%02d ", sec);
		fprintf(stderr, " %d %d %d %d", e_hi, e_lo, e_hi + e_lo, tick);
#ifndef TRACK
		fprintf(stderr, "\n");
#endif
	    }
	}
	avg = 0;
	avg_shift=0;
#ifdef TRACK
	fprintf(stderr, "\n");
#endif
    }

}

uint8_t decode(packed_time *time)
{
    uint8_t idx;
    uint8_t parity_error_count = 0;

    memset(time, 0, sizeof(packed_time));

#ifdef DEBUG
   	fprintf(stderr, "\n");
   	fprintf(stderr, "          1         2         3         4         5         \n");
   	fprintf(stderr, "012345678901234567890123456789012345678901234567890123456789\n");

    for (idx = 0; idx < 60; idx++)
	fprintf(stderr, "%d", get_msf_bit(msf_bit_a, idx));

   	fprintf(stderr, "\n");

    for (idx = 0; idx < 60; idx++)
	fprintf(stderr, "%d", get_msf_bit(msf_bit_b, idx));

   	fprintf(stderr, "\n");
#endif

    idx = 17;

    time->ten_year += (get_msf_bit(msf_bit_a, idx++) * 8);
    time->ten_year += (get_msf_bit(msf_bit_a, idx++) * 4);
    time->ten_year += (get_msf_bit(msf_bit_a, idx++) * 2);
    time->ten_year += (get_msf_bit(msf_bit_a, idx++) * 1);

    time->one_year += (get_msf_bit(msf_bit_a, idx++) * 8);
    time->one_year += (get_msf_bit(msf_bit_a, idx++) * 4);
    time->one_year += (get_msf_bit(msf_bit_a, idx++) * 2);
    time->one_year += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nYear: 20%02d ", (time->ten_year * 10) + time->one_year);
#endif
    if (((getparity(17, 24) + get_msf_bit(msf_bit_b, 54)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time->ten_year = 0;
        time->one_year = 0;
    }


    time->ten_month += (get_msf_bit(msf_bit_a, idx++) * 1);

    time->one_month += (get_msf_bit(msf_bit_a, idx++) * 8);
    time->one_month += (get_msf_bit(msf_bit_a, idx++) * 4);
    time->one_month += (get_msf_bit(msf_bit_a, idx++) * 2);
    time->one_month += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nMonth: %02d" ,(time->ten_month * 10) + time->one_month);
#endif
    if (((getparity(25, 35) + get_msf_bit(msf_bit_b, 55)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time->ten_month = 0;
        time->one_month = 0;
    }

    time->ten_dom += (get_msf_bit(msf_bit_a, idx++) * 2);
    time->ten_dom += (get_msf_bit(msf_bit_a, idx++) * 1);

    time->one_dom += (get_msf_bit(msf_bit_a, idx++) * 8);
    time->one_dom += (get_msf_bit(msf_bit_a, idx++) * 4);
    time->one_dom += (get_msf_bit(msf_bit_a, idx++) * 2);
    time->one_dom += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nDOM: %02d", (time->ten_dom * 10) + time->one_dom);
#endif
    if (((getparity(25, 35) + get_msf_bit(msf_bit_b, 55)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time->ten_dom = 0;
        time->one_dom = 0;
    }


    time->dow += (get_msf_bit(msf_bit_a, idx++) * 4);
    time->dow += (get_msf_bit(msf_bit_a, idx++) * 2);
    time->dow += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nDOW: %d ", time->dow);
#endif
    if (((getparity(36, 38) + get_msf_bit(msf_bit_b, 56)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time->dow = 0;
    }

    time->ten_hour += (get_msf_bit(msf_bit_a, idx++) * 2);
    time->ten_hour += (get_msf_bit(msf_bit_a, idx++) * 1);

    time->one_hour += (get_msf_bit(msf_bit_a, idx++) * 8);
    time->one_hour += (get_msf_bit(msf_bit_a, idx++) * 4);
    time->one_hour += (get_msf_bit(msf_bit_a, idx++) * 2);
    time->one_hour += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nHOUR: %02d:", (time->ten_hour * 10 ) + time->one_hour);
#endif
    if (((getparity(39, 51) + get_msf_bit(msf_bit_b, 57)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time->ten_hour = 0;
        time->one_hour = 0;
    }


    time->ten_minute += (get_msf_bit(msf_bit_a, idx++) * 4);
    time->ten_minute += (get_msf_bit(msf_bit_a, idx++) * 2);
    time->ten_minute += (get_msf_bit(msf_bit_a, idx++) * 1);

    time->one_minute += (get_msf_bit(msf_bit_a, idx++) * 8);
    time->one_minute += (get_msf_bit(msf_bit_a, idx++) * 4);
    time->one_minute += (get_msf_bit(msf_bit_a, idx++) * 2);
    time->one_minute += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nMIN: %02d ", (time->ten_minute * 10) + time->one_minute);
#endif
    if (((getparity(39, 51) + get_msf_bit(msf_bit_b, 57)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time->ten_month = 0;
        time->one_month = 0;
    }

#ifdef DEBUG
   	fprintf(stderr, "\nParity errors: %d\n", parity_error_count);
#endif

    if (
		(time->ten_hour * 10) + time->one_hour > 23 || \
		(time->ten_minute * 10) + time->one_minute > 59 || \
		(time->ten_month * 10) + time->one_month > 11 || \
		(time->ten_dom * 10) + time->one_dom  > 31 || \
		time->dow > 6  || \
		parity_error_count > 0) {
	memset(time, 0, sizeof(packed_time));
	return 1;
    }
   return 0;
}

void clear_msf(uint8_t msf[])
{
    uint8_t i;
    for (i = 0; i < MSF_SIZE; i++)
	msf[i] = 0;
}

uint8_t get_msf_bit(uint8_t msf[], uint8_t period)
{
    uint8_t msf_idx = period / 8;
    uint8_t msf_bit_idx = period % 8;

    return ((msf[msf_idx] & (1 << msf_bit_idx)) > 0);
}

void set_msf_bit(uint8_t msf[], uint8_t period, uint8_t val)
{
    uint8_t msf_idx = period / 8;
    uint8_t msf_bit_idx = period % 8;

    msf[msf_idx] =
	(msf[msf_idx] & ~(1 << msf_bit_idx)) | (val << msf_bit_idx);
}

uint8_t getparity(uint8_t start, uint8_t end)
{
    uint8_t p = 0;
    uint8_t idx = 0;

    for (idx = start; idx <= end; idx++)
	p = p + (get_msf_bit(msf_bit_a, idx));

    return (p);

}
