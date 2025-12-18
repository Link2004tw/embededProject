#ifndef HMI_COMM_H
#define HMI_COMM_H

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
#include "driverlib/timer.h"
#include "driverlib/flash.h"

#define RX_BUFFER_SIZE    64
#define PASSWORD_LENGTH   5
#define PASSWORD          "12345"
#define LOCKOUT_DURATION  5000
#define TIMEOUT_ADDRESS   0x10
#define TIMEOUT_VALUE     3000

#define LOCK_PORT GPIO_PORTB_BASE
#define LOCK_PIN  GPIO_PIN_6
#define LED_PORT  GPIO_PORTF_BASE
#define LED_PIN   GPIO_PIN_1

// ------------------------------
// UART globals (shared)
extern char rxBuffer[RX_BUFFER_SIZE];
extern volatile uint8_t rxIndex;
extern volatile bool messageReady;
extern volatile short failedAttempts;

// Function prototypes
void UART1_Init(void);
void PROCESS_MESSAGE(void);
void UART1_SendString(char* str);
void Door_Unlock(void);
void UART1_Handler(void);
void Start_AutoLock_Timer(void);
void Activate_Lockout(void);
void SavePasswordToEEPROM(char *newPass);
void SaveTimeoutToEEPROM(uint8_t timeout);

#endif
