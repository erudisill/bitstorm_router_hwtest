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

char buffer[1024];
uint16_t buffer_i = 0;

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
	char data = UDR0;
	buffer_i = 0;
	buffer[buffer_i++] = 'C';
	buffer[buffer_i++] = 'P';
	buffer[buffer_i++] = 'H';
	buffer[buffer_i++] = data;
	buffer[buffer_i++] = 0;
}





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
	char data = UDR1;
	buffer[buffer_i++] = data;
	if (buffer_i >= sizeof(buffer))
		buffer_i = sizeof(buffer)-1;
}





void leds_init()
{
	// LEDs
	DDRD |= (1<<6) | (1<<7);
	PORTD |= (1<<6);
}






int main()
{
	int i;

	clock_init();

	leds_init();
	usart0_init();
	usart1_init();
	usart1_flow();

	sei();

	while (1)
	{
		PORTD ^= (1<<6) | (1<<7);
		usart0_transmit('0');
		usart1_transmit('1');
		delay_millis(1000);

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
