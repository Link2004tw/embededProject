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
#define RX_BUFFER_SIZE 8


char rxBuffer[RX_BUFFER_SIZE]="";
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
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Start with LED OFF
    
    while (UARTCharsAvail(UART1_BASE)) {
        UARTCharGet(UART1_BASE);
      }
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF
    
}

void LED_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // PF3 is Green LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); // PF2 is Red LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // PF2 is Red LED
}

int main(void) {
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    LED_Init();

    UART1_Init();
    
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Start with LED OFF
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); // Start with LED OFF
    
    short c =0;
    
    //short mode =0;
    while(1) {
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); // Start with LED OFF
        
        if (UARTCharsAvail(UART1_BASE)) {
          GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); // Start with LED OFF
    
            char receivedChar = UARTCharGet(UART1_BASE);
            //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); // Start with LED OFF
    
            if (receivedChar == '#') {
                rxBuffer[rxIndex] = '\0';   // terminate string

                // Expected format: mode,password
                char *modeStr = strtok(rxBuffer, ",");
                char *passStr = strtok(NULL, ",");

                if (modeStr != NULL && passStr != NULL) {
                    //uint8_t mode = atoi(modeStr);

                    if (*modeStr == '0' && strcmp(passStr, "12345") == 0) {
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Green ON
                        SysCtlDelay(16000000);
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
                        c = 0; // reset failed attempts
                    } else {
                        c++;
                        if (c == 3) {
                            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // Red ON
                            SysCtlDelay(16000000);
                            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
                            c = 0;
                        }
                        
                    }
                }

                // Reset buffer
                rxIndex = 0;
                short t = sizeof(rxBuffer);
                memset(rxBuffer, 0, sizeof(rxBuffer));
            }
            else {
                if (rxIndex < RX_BUFFER_SIZE - 1) {
                    rxBuffer[rxIndex++] = receivedChar;
                }
            }
            //SysCtlDelay(16000000);
                            
        }
    }
} 