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

#define PASSWORD_LEN 5

char rxBuffer[10];
uint8_t rxIndex = 0;

void UART1_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART1));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    
    UARTEnable(UART1_BASE);
}

void LED_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // PF3 is Green LED
}

int main(void) {
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    UART1_Init();
    LED_Init();

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF

    while(1) {
        if (UARTCharsAvail(UART1_BASE)) {
            char receivedChar = UARTCharGet(UART1_BASE);

            if (receivedChar == '#') {
                rxBuffer[rxIndex] = '\0'; 
                
                if (strcmp(rxBuffer, "12345") == 0) {
                    // 1. Turn ON Green LED
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
                    
                    // 2. Wait 3 Seconds
                    // 3 * 16,000,000 / 3 = 16,000,000
                    SysCtlDelay(16000000);
                    
                    // 3. Turn OFF Green LED
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
                } 
                
                // Clear buffer for next attempt
                rxIndex = 0;
                memset(rxBuffer, 0, sizeof(rxBuffer));
            } 
            else {
                if (rxIndex < PASSWORD_LEN) {
                    rxBuffer[rxIndex++] = receivedChar;
                }
            }
        }
    }
}