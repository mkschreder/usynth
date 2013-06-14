#ifndef _UART_H_
#define _UART_H_

#include <inttypes.h>

// after calling this function you can use stdio for reading/writing uart
void UART_Init(uint16_t prescale);
void uart_putchar(char c);
char uart_getchar(void);
#endif
