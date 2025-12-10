#include <stdint.h>
//#include "consts.h"
#include <stdbool.h> // four macros for boolean
#include "tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h" // for clock gating API
#include "driverlib/gpio.h" // for gpio APIs & GPIO_PINx definition
#include "inc/hw_memmap.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "UART.h"

void UART0_Init(void) { 
  // Enable UART0 and Port A
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  
  // Wait for peripherals to be ready
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
  
  // Configure pins for UART0 (PA0=RX, PA1=TX)
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  
  // Configure UART clock and baud rate
  UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
  UARTConfigSetExpClk(UART0_BASE, 16000000, 115200, 
                      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE)); // Enable UART UARTEnable(UART0_BASE); 
}

void UART_SendByte(uint8_t data) { 
    UARTCharPut(UART0_BASE, data); // driverlib function
}

void HMI_SendPassword(uint8_t* pass, uint8_t length) {
  for(uint8_t i = 0; i < length; i++) {
    UART_SendByte(pass[i]); } UART_SendByte('#'); // end marker
}

// --- New UART1 Helper Send Function ---
/**
 * @brief Sends a single byte of data over UART1.
 *
 * This function waits until the UART1 transmit buffer is ready and
 * then sends the byte.
 *
 * @param data The 8-bit data to send.
 */
void UART1_SendByte(uint8_t data) {
    // UARTCharPut is a DriverLib function. It targets UART1_BASE,
    // which is defined by the TivaWare library.
    UARTCharPut(UART1_BASE, data);
}

// --- Password Sending Function (via UART1) ---
/**
 * @brief Sends a password string over UART1, followed by an end marker.
 *
 * @param pass Pointer to the array of password bytes (uint8_t).
 * @param length The length of the password array.
 */
void HMI_SendPassword_UART1(uint8_t* pass, uint8_t length) {
    // Check for valid pointer and length
    if (pass == NULL || length == 0) {
        return;
    }

    // Loop through and send each byte of the password
    for(uint8_t i = 0; i < length; i++) {
        // Use the UART1-specific send function
        UART1_SendByte(pass[i]);
    }

    // Send the end marker after the password
    UART1_SendByte('#');
}
