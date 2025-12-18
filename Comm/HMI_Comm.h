#ifndef HMI_COMM_H
#define HMI_COMM_H
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#define PASSMODE '0'
#define ADDMODE '1'
#define TIMEMODE '2'

//void HMI_SendPassword(uint8_t* pass, uint8_t length);
void UART5_Init_front(void);
void UART5_SendString(char* str);
void UART5_ReceiveString(char* buffer, uint16_t max_length);
void UART5_ReceiveStringWithTimeout(char* buffer, uint16_t max_length);

#endif