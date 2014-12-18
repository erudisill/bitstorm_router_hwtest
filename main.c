/*
 * main.c
 *
 *  Created on: Nov 10, 2014
 *      Author: ericrudisill
 */

#define BAUD 38400

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/setbaud.h>
#include "clock.h"

/*
 * Simple data buffer. Not production ready!
 */
char buffer[1024];
uint16_t buffer_i = 0;



/*
 * WAN USART0
 *
 * Initialize for TX at 38400 8N1.
 * No flow control necessary.
 * RX via ISR.
 */
void usart0_init()
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	// Enble receiver and transmitter
	UCSR0B |= (1<<RXCIE0) | (1<<TXEN0);

	// Set rx and tx enable bits
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
	// Set databits to 8
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

void usart0_transmit(uint8_t data )
{
	while (!( UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

ISR(USART0_RX_vect)
{
	// Receive data and stuff magic string + char into databuffer.
	// NOTE: This will trash whatever is already in the databuffer.
	//       It is for testing RX capability only.

	char data = UDR0;
	buffer_i = 0;
	buffer[buffer_i++] = 'C';
	buffer[buffer_i++] = 'P';
	buffer[buffer_i++] = 'H';
	buffer[buffer_i++] = data;
	buffer[buffer_i++] = 0;
}




/*
 * BLE USART1
 *
 * Initialize for TX at 38400 8N1.
 * HW Flow Control implemented on PD4 and PD5.
 * RX via ISR.
 */
void usart1_init()
{
	UBRR1H = UBRRH_VALUE;
	UBRR1L = UBRRL_VALUE;

	// Enble receiver and transmitter
	UCSR1B |= (1<<RXCIE1) | (1<<TXEN1);

	// Set rx and tx enable bits
	UCSR1B |= (1 << RXEN1) | (1 << TXEN1);
	// Set databits to 8
	UCSR1C |= (1 << UCSZ01) | (1 << UCSZ00);
}

void usart1_transmit(uint8_t data )
{
	while (!( UCSR1A & (1<<UDRE1)));
	UDR1 = data;
}

void usart1_flow()
{
	// MCU_RTS .. BLE_CTS
	DDRD |= (1<<5);
	PORTD |= (0<<5);

	// MCU_CTS .. BLE_RTS
	DDRD &= ~(1<<4);
}

ISR(USART1_RX_vect)
{
	// Read the data, stuff it into the buffer, increment the index.
	// NOTE: If overflow occurs, it just continuously overwrites the last char.
	//       Do not use as reference code - for testing only.
	char data = UDR1;
	buffer[buffer_i++] = data;
	if (buffer_i >= sizeof(buffer))
		buffer_i = sizeof(buffer)-1;
}




/*
 * LEDs
 */
void leds_init()
{
	// LEDs - Set both to output and turn one on.
	DDRD |= (1<<6) | (1<<7);
	PORTD |= (1<<6);
}


void dumpHex_0(uint8_t * data, int len)
{
	char hex[] = "0123456789ABCDEF";
	uint8_t x;

	for (int i=0;i<len;i++)
	{
		x = (uint8_t)hex[(data[i] >> 4)];
		usart0_transmit(x);
		x = (uint8_t)hex[(data[i] & 0x0f)];
		usart0_transmit(x);
		usart0_transmit(' ');
	}
}


/*
 *			MAIN MAIN MAIN MAIN MAIN
 *			MAIN MAIN MAIN MAIN MAIN
 *			MAIN MAIN MAIN MAIN MAIN
 *
 */
int main()
{
	int i;
	uint8_t data[] = { 0x01, 0x02, 0x03, 0xab, 0xcd, 0xef };

	clock_init();

	leds_init();
	usart0_init();
	//usart1_init();
	//usart1_flow();

	sei();

	while (1)
	{
		// Toggle LEDs
		PORTD ^= (1<<6) | (1<<7);

		// Spit out regular data across the USARTs
		//usart0_transmit('0');
		dumpHex_0(data, sizeof(data));
		usart0_transmit('\r');
		usart0_transmit('\n');
		//usart1_transmit('1');

		// Exercise clock subsystem
		delay_millis(1000);

		// If databuffer has characters, dump to WAN USART and clear
		i = 0;
		while (buffer[i])
		{
			usart0_transmit(buffer[i]);
			buffer[i] = 0;
			i++;
			if (i >= sizeof(buffer))
				break;
		}
		buffer_i = 0;
	}
}
