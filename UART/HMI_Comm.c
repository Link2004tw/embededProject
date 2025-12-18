#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "HMI_Comm.h"

#define PASSWORD "12345" 
char rxBuffer[RX_BUFFER_SIZE] = "";
uint8_t rxIndex = 0;
short failedAttempts = 0;

void UART1_Init(void) {
    // 1. Enable Peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART1));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    // 2. Configure LED Pins (F1=Red, F2=Blue, F3=Green)
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0); // Start all OFF

    // 3. Configure UART Pins on Port B
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    // --- FIX: Prevent "Unknown Character" defect ---
    // Add internal pull-up to RX pin to keep it high when idle
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // 4. Configure UART parameters (9600, 8-N-1)
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    UARTEnable(UART1_BASE);
    
    // --- FIX: Clear Startup Junk ---
    // Wait for the line to settle and flush any garbage characters
    SysCtlDelay(SysCtlClockGet() / 20); 
    while (UARTCharsAvail(UART1_BASE)) { UARTCharGet(UART1_BASE); }
}

void BUZZ(void) {
    // Visual "Buzz" using Red LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); 
    SysCtlDelay(SysCtlClockGet() / 3); // Approx 1 second
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
}

void UART1_SendString(char* str) {
    // Signal transmission start with Green LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    
    while(*str) {
        UARTCharPut(UART1_BASE, *str);
        str++;
    }

    // Wait for transmit to finish then turn LED off
    while(UARTBusy(UART1_BASE));  
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
}

void WAIT_FOR_MESSAGE(void) {
    if (UARTCharsAvail(UART1_BASE)) {
        char receivedChar = UARTCharGet(UART1_BASE);
        
        // Visual feedback for character reception (Blue LED)
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        SysCtlDelay(SysCtlClockGet() / 300); // Short flash
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

        if (receivedChar == '#') {
            // Null-terminate the string, effectively removing the '#' for strcmp
            rxBuffer[rxIndex] = '\0'; 
            rxIndex = 0; 

            char rxBufferCopy[RX_BUFFER_SIZE];
            strcpy(rxBufferCopy, rxBuffer);

            char *modeStr = strtok(rxBufferCopy, ",");
            char *pass1Str = strtok(NULL, ",");
            char *pass2Str = strtok(NULL, ",");

            if (modeStr == NULL || pass1Str == NULL) {
                UART1_SendString("Invalid Format#");
                memset(rxBuffer, 0, sizeof(rxBuffer));
                return;
            }

            // Mode 0: Normal Unlock
            if (strcmp(modeStr, "0") == 0) {
                if (strcmp(pass1Str, PASSWORD) == 0) {
                    UART1_SendString("Correct Password#");
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Green LED Success
                    SysCtlDelay(SysCtlClockGet() / 3);
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
                    failedAttempts = 0;
                } else {
                    failedAttempts++;
                    UART1_SendString("Wrong Password#");
                    if (failedAttempts >= 3) {
                        BUZZ();
                        failedAttempts = 0;
                    }
                }
            }
            // Mode 1: Change Password
            else if (strcmp(modeStr, "1") == 0) {
                if (strcmp(pass1Str, PASSWORD) == 0) {
                    if (pass2Str != NULL) {
                        UART1_SendString("Password Changed#");
                        // Logic to update password would go here
                    }
                    failedAttempts = 0;
                } else {
                    failedAttempts++;
                    UART1_SendString("Wrong Password#");
                }
            }
            // Mode 2: Settings
            else if (strcmp(modeStr, "2") == 0) {
                if(strcmp(pass1Str, "26") == 0) {
                    UART1_SendString("Timeout saved#");
                }
            }

            memset(rxBuffer, 0, sizeof(rxBuffer));
        } else {
            // Add char to buffer if there is space
            if (rxIndex < RX_BUFFER_SIZE - 1) {
                rxBuffer[rxIndex++] = receivedChar;
            } else {
                rxIndex = 0;
                memset(rxBuffer, 0, sizeof(rxBuffer));
            }
        }
    }
}
