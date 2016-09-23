#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include "timer.h"

#define DEBUG 1

//                        2222222211111111
//                        4444333322221111
//                        8421842184218421
#define MIN_MASK	0b0000000111111110
#define MIN_PATT	0b0000000000011110

#define SEC_MASK	0b0000001111111100
#define SEC_PATT	0b0000000000000100

#define MSF_SIZE	(64/8)

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
}

void binprint(uint16_t data, uint8_t bits)
{
    int i;

    for (i = bits; i >= 0; i--) {
	printf("%d", ((data & (1 << i)) > 0));
    }
}

#define SAMPLES	8

ISR(TIMER1_OVF_vect)
{
    static uint8_t avg = 0;
    static uint8_t avg_shift = 0;
    static uint8_t last_high_avg_shift = 0;
    static uint8_t sample_idx = 0;
    static uint16_t history = 0;
    static uint8_t e_hi=0;
    static uint8_t e_lo=0;
    uint8_t state = 0;


    TCNT1 = offset;
    //PIND |= (1 << PD3);        // LO

    PORTD = PORTD ^ (1 << PD3);        // LO

    state = ((PIND & (1 << PD2)) > 0);
    avg = avg + state;
    avg_shift = avg_shift << 1 | state ;

		printf("\t\t\tS [");
		binprint(avg_shift, 8);
		printf("]\n");
    sample_idx++;
    if (sample_idx >= SAMPLES) {
	sample_idx = 0;

	    PORTD = PORTD ^ (1 << PD4);        // LO
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
/*
	    uint8_t i;
	    i = 0;
	    while ((last_high_avg_shift & 0x80) > 0 )
	    {
		i++;
		last_high_avg_shift = last_high_avg_shift << 1;
	    }
	    if (last_high_avg_shift == 0 && i >= (SAMPLES / 2))
	    {
		    PORTD = PORTD ^ (1 << PD5);        // LO
		avg=0;
		sample_idx=(8-i);
	    }
*/
	}

	avg = 0;
	avg_shift=0;
	binprint(history, 16);
	printf("\n");

	if ((history & MIN_MASK) == MIN_PATT) {
	    sync_flag = 1;
	    sec = 0;
	} else {
	    if ((history & SEC_MASK) == SEC_PATT) {
		//xxxxxx-----11111
		set_msf_bit(msf_bit_a, sec,
			    ((history & 0b0000000001000000) > 0));
		set_msf_bit(msf_bit_b, sec,
			    ((history & 0b0000000010000000) > 0));
		if (sec++ > 59)
		    sec = 0;
		printf("\t%02d ", sec);
		printf(" %d %d %d", e_hi, e_lo, e_hi + e_lo);
		printf("\n");
	    }
	}
    }

}

void decode(void)
{
    uint8_t val;
    uint8_t idx;
    uint8_t parity_error_count = 0;
    //static RtcDateTime this_tm;
    //static RtcDateTime prev_tm;

    hour = min = mon = dom = dow = year = 0;

#ifdef DEBUG
    printf("\n");
    printf("          1         2         3         4         5         \n");
    printf("012345678901234567890123456789012345678901234567890123456789\n");

    for (idx = 0; idx < 60; idx++)
	printf("%d", get_msf_bit(msf_bit_a, idx));

    printf("\n");

    for (idx = 0; idx < 60; idx++)
	printf("%d", get_msf_bit(msf_bit_b, idx));

    printf("\n");
#endif

    idx = 17;

    val = 0;
    val += (get_msf_bit(msf_bit_a, idx++) * 80);
    val += (get_msf_bit(msf_bit_a, idx++) * 40);
    val += (get_msf_bit(msf_bit_a, idx++) * 20);
    val += (get_msf_bit(msf_bit_a, idx++) * 10);

    val += (get_msf_bit(msf_bit_a, idx++) * 8);
    val += (get_msf_bit(msf_bit_a, idx++) * 4);
    val += (get_msf_bit(msf_bit_a, idx++) * 2);
    val += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
    printf("\nYear: 20%02d ", val);
#endif
    if (((getparity(17, 24) + get_msf_bit(msf_bit_b, 54)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	printf("!");
#endif
    } else
	year = val;

    val = 0;
    val += (get_msf_bit(msf_bit_a, idx++) * 10);

    val += (get_msf_bit(msf_bit_a, idx++) * 8);
    val += (get_msf_bit(msf_bit_a, idx++) * 4);
    val += (get_msf_bit(msf_bit_a, idx++) * 2);
    val += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
    printf("\nMonth: %02d" ,val);
#endif
    if (((getparity(25, 35) + get_msf_bit(msf_bit_b, 55)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	printf("!");
#endif
    } else
	mon = val;

    val = 0;
    val += (get_msf_bit(msf_bit_a, idx++) * 20);
    val += (get_msf_bit(msf_bit_a, idx++) * 10);

    val += (get_msf_bit(msf_bit_a, idx++) * 8);
    val += (get_msf_bit(msf_bit_a, idx++) * 4);
    val += (get_msf_bit(msf_bit_a, idx++) * 2);
    val += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
    printf("\nDOM: %02d", val);
#endif
    if (((getparity(25, 35) + get_msf_bit(msf_bit_b, 55)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	printf("!");
#endif
    } else
	dom = val;

    val = 0;
    val += (get_msf_bit(msf_bit_a, idx++) * 4);
    val += (get_msf_bit(msf_bit_a, idx++) * 2);
    val += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
    printf("\nDOW: %d ", val);
#endif
    if (((getparity(36, 38) + get_msf_bit(msf_bit_b, 56)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	printf("!");
#endif
    } else
	dow = val;

    val = 0;
    val += (get_msf_bit(msf_bit_a, idx++) * 20);
    val += (get_msf_bit(msf_bit_a, idx++) * 10);

    val += (get_msf_bit(msf_bit_a, idx++) * 8);
    val += (get_msf_bit(msf_bit_a, idx++) * 4);
    val += (get_msf_bit(msf_bit_a, idx++) * 2);
    val += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
    printf("\nHOUR: %02d:", val);
#endif
    if (((getparity(39, 51) + get_msf_bit(msf_bit_b, 57)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	printf("!");
#endif
    } else
	hour = val;

    val = 0;
    val += (get_msf_bit(msf_bit_a, idx++) * 40);
    val += (get_msf_bit(msf_bit_a, idx++) * 20);
    val += (get_msf_bit(msf_bit_a, idx++) * 10);

    val += (get_msf_bit(msf_bit_a, idx++) * 8);
    val += (get_msf_bit(msf_bit_a, idx++) * 4);
    val += (get_msf_bit(msf_bit_a, idx++) * 2);
    val += (get_msf_bit(msf_bit_a, idx++) * 1);
#ifdef DEBUG
    printf("\nMIN: %02d ", val);
#endif
    if (((getparity(39, 51) + get_msf_bit(msf_bit_b, 57)) & 0x01) == 0) {
	parity_error_count++;
#ifdef DEBUG
	printf("!");
#endif
    } else
	min = val;

#ifdef DEBUG
    printf("\nParity errors: %d\n", parity_error_count);
#endif

    if (parity_error_count > 0) {
	hour = min = mon = dom = dow = year = 0;
    }

    /*
       this_tm = RtcDateTime(year, mon, dom, hour, min, 0);
       if (this_tm.TotalSeconds() - 60 == prev_tm.TotalSeconds())
       {
       Rtc.SetDateTime(this_tm);
       sync_tm = this_tm;
       }
       else
       {
       printf("SYNC FAIL: ");
       printDateTime(prev_tm);
       printf(" is not 60 earlier than ");
       printDateTime(this_tm);
       printf('\n');

       }
       prev_tm = this_tm;
     */
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
