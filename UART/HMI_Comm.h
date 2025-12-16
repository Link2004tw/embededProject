#include <stdint.h>
#include <stdbool.h>
#include <string.h> 
#include "tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"


#define RX_BUFFER_SIZE 20
void UART1_Init(void);
void WAIT_FOR_MESSAGE(void);
void UART1_SendString(char* str);