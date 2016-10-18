#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include "util.h"
#include "time.h"

#include "i2cmaster.h"
#include "spi.h"
#include "led.h"
#include "pcf8574.h"
#include "ds3231.h"
#include "max7219.h"
#include <util/delay.h>
#include "tm1637.h"

//#include "tea5767.h"

#define BAUD 115200
#define BAUD_TOL 5

//#define BAUD 57600
#include <util/setbaud.h>

#define BVV(bit, val) ((val)?_BV(bit):0)

#include "lcd.h"
#include "msf.h"
#include "at24c32.h"
#include "keypad.h"
#include "main.h"


packed_time set_time;		// Working copy of time - used during 'set' operations
packed_time rtc_time;
packed_time msf_time[2];
s_tm_t alarm_time[2];

static void
usart_init (void)
{
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
#if USE_2X
  UCSR0A |= _BV (U2X0);
#else
  UCSR0A &= ~_BV (U2X0);
#endif
  UCSR0B = BVV (TXEN0, 1) | BVV (RXEN0, 0);	/* Only TX */
}

static void
usart_putchar (char c)
{
  while (!(UCSR0A & _BV (UDRE0)));
  UDR0 = c;
}

/*
static void usart_puts(const char *s)
{
    while(*s != '\0')
    {
        usart_putchar(*s++);
    }
}
*/

int
usart_putchar_printf (char var, FILE * stream)
{
  if (var == '\n')
    usart_putchar ('\r');
  usart_putchar (var);
  return 0;
}

int
lcd_printf_char (char var, FILE * stream)
{
  LCD_Write_Char (var);
  return 0;
}

void
dumptime (char *prefix, packed_time * tm)
{
  fprintf (stderr, "%s: %02d:%02d:%02d\n", prefix,
	   (tm->ten_hour * 10) + tm->one_hour,
	   (tm->ten_minute * 10) + tm->one_minute,
	   (tm->ten_second * 10) + tm->one_second);
  fprintf (stderr, "%s: %s %02d/%02d/%04d\n", prefix, dayname[tm->dow],
	   (tm->ten_dom * 10) + tm->one_dom,
	   (tm->ten_month * 10) + tm->one_month,
	   (tm->ten_year * 10) + tm->one_year + 1900);
}

int
led_printf_char (char var, FILE * stream)
{
  static uint8_t idx = 0;
  uint8_t bitval;

  if (var == '\n')
    {
      idx = 0;
      return 0;
    }
  bitval = getled (var);
  //max7219(MAX7219_DIGIT7 - idx, bitval);
  //fprintf(stderr, "Sending %d to pos %d\n", bitval, idx);
  led_framebuf[idx][0] = bitval;
  led_framebuf[idx][1] = bitval;
  led_framebuf[idx][2] = bitval;
  led_framebuf[idx][3] = bitval;

  idx = ((idx + 1) & 0x07);

  return 0;
}

FILE display_serial =
FDEV_SETUP_STREAM (usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);
FILE display_lcd =
FDEV_SETUP_STREAM (lcd_printf_char, NULL, _FDEV_SETUP_WRITE);
//FILE display_led = FDEV_SETUP_STREAM(led_printf_char, NULL, _FDEV_SETUP_WRITE);

int
main (void)
{
  uint8_t count = 0;
  //uint8_t lcd_bl=0;
  uint8_t decode_idx = 0;

  stderr = &display_serial;
  stdout = &display_lcd;


  usart_init ();
  i2c_init ();
  spi_init ();
  timer_init ();
  tm_init ();
  //struct tm temp_tm;

/*
	memset(&temp_tm, 0, sizeof(temp_tm));
	dumptime("INIT:", &temp_tm);
	temp_tm.tm_hour=12;
	temp_tm.tm_min=34;
	temp_tm.tm_mday=4;
	temp_tm.tm_mon=5;
	temp_tm.tm_year=115;
	//temp_tm.tm_wday=1;
	//temp_tm.tm_yday=123;

	dumptime("BEFORE:", &temp_tm);
	rtc_time=mktime(&temp_tm);

	memset(&temp_tm, 0, sizeof(temp_tm));
	dumptime("PRE:", &temp_tm);

	gmtime_r(&rtc_time, &temp_tm);

	dumptime("AFTER:", &temp_tm);

	while(1);
*/

  max7219 (MAX7219_SHUTDOWN, 0x01);
  max7219 (MAX7219_SCANLIMIT, 0x07);
  max7219 (MAX7219_INTENSITY, 0x02);
  max7219 (MAX7219_TEST, 0x00);
  max7219 (MAX7219_DECODE, 0x00);

/*    DDRD = 0x00;	// ALL INPUT
    PORTD = 0x00;	// NO PULLUP
    PIND = 0x00;	// NO PULLUP
*/
  DDRD = 0xff;			// ALL OUTPUT

  //DDRD | (1 << PD3);     // Set the PD3 pin (OUTPUT)
  DDRD &= ~(1 << PD2);		// Clear the PD2 pin (INPUT) 
  //PORTD |= ~(1 << PORTD2);    // turn On the Pull-up
  // PD2 is now an input with pull-up enabled

//PORTD &= ~(1 << PD3);        // LO
  DDRC |= (1 << PD7);		// Clear the PD2 pin (OUTPUT) 
  DDRC |= (1 << PD6);		// Clear the PD2 pin (OUTPUT) 

/*
    EICRA |= (1 << ISC00);    // set INT0 to trigger on ANY logic change
    EIMSK |= (1 << INT0);     // Turns on INT0
*/


  sei ();			// turn on interrupts


/*
	uint8_t i;
	for(i=0;i<10;i++)
	{
		fprintf(stderr, "Sleep %d\n", i);
		_delay_ms(1000);
	}
*/


  at24c32_write (7, 1, 0x23);
  at24c32_write (7, 2, 0x34);
  // Turn on 32khz output
/*
	uint8_t c=1;
	c = ds3231_read(0x0F);
	c |= (1 << 3);
	ds3231_write(0x0F, c);
*/

  LCD_Init ();

  fprintf (stderr, "Libc version: %s\n", __AVR_LIBC_VERSION_STRING__);


  readalarm (0);
  readalarm (1);
  LCD_BL (1);

/*
    TEA5767 Radio;
    Radio.init();
    Radio.set_frequency(105.4);
*/


  while (1)
    {

/*
	unsigned char buf[5];
	int stereo;
	int signal_level;
	double current_freq;
	unsigned long current_millis = millis();

	if (Radio.read_status(buf) == 1) {
	    current_freq =
		floor(Radio.frequency_available(buf) / 100000 + .5) / 10;
	    stereo = Radio.stereo(buf);
	    signal_level = Radio.signal_level(buf);
	    lcd.setCursor(0, 0);
	    fprintf(&display_lcd, "FM: ");
	    fprintf(&display_lcd, current_freq);
	    lcd.setCursor(0, 1);
	    if (stereo)
		fprintf(&display_lcd, "STEREO ");
	    else
		fprintf(&display_lcd, "MONO ");
	    //lcd.print(signal_level);
	}

	if (search_mode == 1) {
	    if (Radio.process_search(buf, search_direction) == 1) {
		search_mode = 0;
	    }
	}
*/

      count++;

      ds3231_readtime (&rtc_time);

      if (sync_flag == 1)
	{
	  sync_flag = 0;
	  fprintf (stderr, "SYNC\n");
	  if (decode (&msf_time[decode_idx]) == 0)
	    {
	      if (msf_time[0].ten_minute == msf_time[1].ten_minute &&
		  msf_time[0].one_minute == msf_time[1].one_minute &&
		  msf_time[0].ten_hour == msf_time[1].ten_hour &&
		  msf_time[0].one_hour == msf_time[1].one_hour &&
		  msf_time[0].one_dom == msf_time[1].one_dom &&
		  msf_time[0].ten_dom == msf_time[1].ten_dom &&
		  msf_time[0].one_month == msf_time[1].one_month &&
		  msf_time[0].ten_month == msf_time[1].ten_month &&
		  msf_time[0].one_year == msf_time[1].one_year &&
		  msf_time[0].ten_year == msf_time[1].ten_year)
		{
		  fprintf (stderr, "Storing MSF time!\n");
		  ds3231_writetime (&msf_time[decode_idx]);
		}
	    }

	  decode_idx = 1 - decode_idx;
	}

      _delay_ms (80);

      keypad ();

      fprintf (&display_lcd, "LOOP [%02x]\n", count);
    }
}



/*ISR (INT0_vect)
{
	fprintf(stderr, "!!!!!\n");
}
*/
