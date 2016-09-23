#include <stdio.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include "spi.h"


volatile uint8_t SPI_buf[SPI_BUF_LEN];
volatile uint8_t spi_busy = 0;

volatile uint8_t SPI_buf_w_idx=0;
volatile uint8_t SPI_buf_r_idx=0;

void spi_init(void)
{
	uint8_t i;

	DDRB |= (1 << SPI_SS);	// SS out
	DDRB |= (1 << SPI_MOSI);	// MOSI out
	DDRB |= (1 << SPI_SCK);	// SCK out

	PORTB &= ~(1 << SPI_SS);	// SS LOW

	SPCR |= (1 << MSTR);
	SPCR |= (1 << SPE);
	//SPCR |= (1 << SPIE);

	SPCR |= (1 << SPR0);
	SPCR |= (1 << SPR1);
	
	for (i=0; i< SPI_BUF_LEN; i++)
		SPI_buf[i]=i+1;

}

uint8_t spi_enqueue(uint8_t value)
{
	uint8_t next;


	next = SPI_buf_w_idx + 1;
	if (next >= SPI_BUF_LEN)
		next = 0;

	if (next == SPI_buf_r_idx)
		return 1;
		
	SPI_buf[SPI_buf_w_idx]=value;

	//if (next != SPI_buf_w_idx + 1 )
		//printf("Wrapped SPI_buf_w_idx\n");

	SPI_buf_w_idx = next;

	return 0;
}

uint8_t spi_dequeue(uint8_t *qval)
{

	//printf("\t Read index %d => Write index %d\t", SPI_buf_r_idx, SPI_buf_w_idx);
	if (SPI_buf_r_idx != SPI_buf_w_idx)
	{
		//printf("De-Queued 0x%02x ", SPI_buf[SPI_buf_r_idx]);
		*qval=SPI_buf[SPI_buf_r_idx++];

		if (SPI_buf_r_idx == SPI_BUF_LEN)
		{
			//printf(" Wrapped SPI_buf_r_ptr ");
			SPI_buf_r_idx=0;
		}

		return 0;
	}
	else
	{
		return 1;
	}

}

void spi_dumpqueue(void)
{
	uint8_t i;

	for (i=0; i< SPI_BUF_LEN; i++)
	{
		printf("%d -> 0x%02x\n", i, SPI_buf[i]);
	}
}

void max7219(uint8_t addr, uint8_t data)
{
	PORTB &= ~(1 << SPI_SS);	// SS LOW
	SPDR = addr;
	while(!(SPSR & (1 << SPIF)));
	SPDR = data;
	while(!(SPSR & (1 << SPIF)));
	PORTB |= (1 << SPI_SS);		// SS HIGH
	_delay_us(10);
	PORTB &= ~(1 << SPI_SS);	// SS LOW
}

void spi_send(uint8_t data)
{
	while(spi_enqueue(data) == 1)
	{
		printf("Waiting for free SPI queue slot..\n");
		_delay_ms(100);
	}

	if (spi_busy == 0)
	{
		printf("SPI Idle ");
		if (spi_dequeue(&data) == 0)
		{
			printf(" - triggering with 0x%02x\n", data);
			SPDR = data;
		}
		else
			printf(" - but queue is empty\n");
		
	}
}

ISR (SPI_STC_vect)
{
/*
 * uint8_t data;
	static uint8_t byte_count=1;

	if (byte_count++ >= 2)
	{
		PORTB |= (1 << SPI_SS);		// SS HIGH
		_delay_ms(1);
		PORTB &= ~(1 << SPI_SS);	// SS LOW
		byte_count=0;
	}
		
	if (spi_dequeue(&data) == 0)
	{
		spi_busy = 1;
		SPDR = data;
		printf("SPI_ISR: 0x%02x\n", data);
	}
	else
	{
		printf("SPI_ISR: idle\n");
		spi_busy = 0;
	}

*/
}
