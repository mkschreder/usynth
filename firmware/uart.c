#include "uart.h"
#include <stdio.h>
#include <avr/io.h>

void UART_PutChar(char c) {
    loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
    UDR0 = c;
}

char UART_GetChar(void) {
    loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
}

void UART_Init(uint16_t bp){
	
	UBRR0L = (uint8_t)bp;
	UBRR0H = bp >> 8; 

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */ 
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
	
	//stdout = &uart_output;
	//stdin  = &uart_input;
}
