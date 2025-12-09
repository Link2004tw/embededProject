#ifndef UART_H
#define UART_H

#include <stdint.h>

extern volatile uint8_t rxData;   // Last received byte
extern volatile uint8_t rxFlag;   // Data received flag

void UART0_Init(void);
void UART0_Handler(void);

#endif
