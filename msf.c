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

#define DEBUG 1

//                        2222222211111111
//                        4444333322221111
//                        8421842184218421
//#define MIN_MASK	__extension__ 0b0001111101111100
//#define MIN_PATT	__extension__ 0b0001111100000100
#define MIN_MASK        __extension__ 0b0001111110111111
#define MIN_PATT        __extension__ 0b0000000000111110


#define SEC_MASK	__extension__ 0b0001001111111100
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
time_t msf_time[2];

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
		msf_time[decode_idx] = decode();
		decode_idx=1 - decode_idx;
		tick=24;
#ifndef TRACK
		fprintf(stderr, "\n");
#endif
	} else {
	    if ((history & SEC_MASK) == SEC_PATT && (tick%80 == 0)) {
		sec = (tick/80)-1;
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

time_t decode(void)
{
    uint8_t idx;
    uint8_t parity_error_count = 0;
    struct tm time;

    memset(&time, 0, sizeof(struct tm));

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

    time.tm_year += (get_msf_bit(msf_bit_a, idx++) * 80);
    time.tm_year += (get_msf_bit(msf_bit_a, idx++) * 40);
    time.tm_year += (get_msf_bit(msf_bit_a, idx++) * 20);
    time.tm_year += (get_msf_bit(msf_bit_a, idx++) * 10);

    time.tm_year += (get_msf_bit(msf_bit_a, idx++) * 8);
    time.tm_year += (get_msf_bit(msf_bit_a, idx++) * 4);
    time.tm_year += (get_msf_bit(msf_bit_a, idx++) * 2);
    time.tm_year += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nYear: 20%02d ", time.tm_year);
#endif
    if (((getparity(17, 24) + get_msf_bit(msf_bit_b, 54)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time.tm_year = 0;
    }


    time.tm_mon += (get_msf_bit(msf_bit_a, idx++) * 10);

    time.tm_mon += (get_msf_bit(msf_bit_a, idx++) * 8);
    time.tm_mon += (get_msf_bit(msf_bit_a, idx++) * 4);
    time.tm_mon += (get_msf_bit(msf_bit_a, idx++) * 2);
    time.tm_mon += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nMonth: %02d" ,time.tm_mon);
#endif
    if (((getparity(25, 35) + get_msf_bit(msf_bit_b, 55)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time.tm_mon = 0;
    }

    time.tm_mday += (get_msf_bit(msf_bit_a, idx++) * 20);
    time.tm_mday += (get_msf_bit(msf_bit_a, idx++) * 10);

    time.tm_mday += (get_msf_bit(msf_bit_a, idx++) * 8);
    time.tm_mday += (get_msf_bit(msf_bit_a, idx++) * 4);
    time.tm_mday += (get_msf_bit(msf_bit_a, idx++) * 2);
    time.tm_mday += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nDOM: %02d", time.tm_mday);
#endif
    if (((getparity(25, 35) + get_msf_bit(msf_bit_b, 55)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time.tm_mday = 0;
    }


    time.tm_wday += (get_msf_bit(msf_bit_a, idx++) * 4);
    time.tm_wday += (get_msf_bit(msf_bit_a, idx++) * 2);
    time.tm_wday += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nDOW: %d ", time.tm_wday);
#endif
    if (((getparity(36, 38) + get_msf_bit(msf_bit_b, 56)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time.tm_wday = 0;
    }

    time.tm_hour += (get_msf_bit(msf_bit_a, idx++) * 20);
    time.tm_hour += (get_msf_bit(msf_bit_a, idx++) * 10);

    time.tm_hour += (get_msf_bit(msf_bit_a, idx++) * 8);
    time.tm_hour += (get_msf_bit(msf_bit_a, idx++) * 4);
    time.tm_hour += (get_msf_bit(msf_bit_a, idx++) * 2);
    time.tm_hour += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nHOUR: %02d:", time.tm_hour);
#endif
    if (((getparity(39, 51) + get_msf_bit(msf_bit_b, 57)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time.tm_hour = 0;
    }


    time.tm_min += (get_msf_bit(msf_bit_a, idx++) * 40);
    time.tm_min += (get_msf_bit(msf_bit_a, idx++) * 20);
    time.tm_min += (get_msf_bit(msf_bit_a, idx++) * 10);

    time.tm_min += (get_msf_bit(msf_bit_a, idx++) * 8);
    time.tm_min += (get_msf_bit(msf_bit_a, idx++) * 4);
    time.tm_min += (get_msf_bit(msf_bit_a, idx++) * 2);
    time.tm_min += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
   	fprintf(stderr, "\nMIN: %02d ", time.tm_min);
#endif
    if (((getparity(39, 51) + get_msf_bit(msf_bit_b, 57)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	fprintf(stderr, "!");
#endif
        time.tm_min = 0;
    }

#ifdef DEBUG
   	fprintf(stderr, "\nParity errors: %d\n", parity_error_count);
#endif

    if (time.tm_hour <= 23 || time.tm_min <= 59 || time.tm_mon <= 11 || time.tm_mday <= 31 || time.tm_wday <= 6  || parity_error_count > 0) {
	return mktime(&time);
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
