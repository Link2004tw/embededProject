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
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

#define PASSWORD "12345" // will be changed to eeprom later
#define LOCK_PORT GPIO_PORTB_BASE
#define LOCK_PIN  GPIO_PIN_6
#define RX_BUFFER_SIZE 20


void Door_Unlock(void);
void Start_AutoLock_Timer(void);
void Activate_Lockout(void);
void SavePasswordToEEPROM(char *newPass);
void SaveTimeoutToEEPROM(uint8_t timeout);

void UART1_Init(void);
void PROCESS_MESSAGE(void);
void UART1_SendString(char* str);