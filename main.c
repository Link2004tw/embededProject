//#include "UART/HMI_Comm.h"
//#include <stdint.h>
//#include <stdbool.h>
//#include <string.h> 
//#include "tm4c123gh6pm.h"
//#include "inc/hw_memmap.h"
//#include "inc/hw_types.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/gpio.h"
//#include "driverlib/uart.h"
//#include "driverlib/pin_map.h"

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

#define PASSWORD "12345" // will be changed to eeprom later

#define RX_BUFFER_SIZE 20

short c =0;
void LED_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // PF3 is Green LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); // PF2 is Red LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // PF2 is Red LED
}

char rxBuffer[RX_BUFFER_SIZE]="";
uint8_t rxIndex = 0;

short failedAttempts =0;
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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // PF3 is Green LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF
    

    
    UARTEnable(UART1_BASE);
    //SysCtlDelay(1000);  // Add delay to let UART settle
    SysCtlDelay(SysCtlClockGet() / 100);  // Add delay to let UART settle
    
    while (UARTCharsAvail(UART1_BASE)) {
        UARTCharGet(UART1_BASE);
    }
    //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Start with LED OFF
    
    
}

void BUZZ(void){
GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // Red ON
                            SysCtlDelay(16000000);
                            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

}


void UART1_SendString2(char* str) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    
    while(*str) {
        while(UARTBusy(UART1_BASE));  // Wait until TX ready
        UARTCharPut(UART1_BASE, *str);
        str++;
    }

    while(UARTBusy(UART1_BASE));  // Wait for last char to finish
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
}

void UART1_SendString(char* str) {
    if (str == NULL || *str == '\0') return;  // Guard against empty strings
    
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    
    while(*str) {
        while(UARTBusy(UART1_BASE));  // Wait until TX ready
        UARTCharPut(UART1_BASE, *str);
        str++;
    }
    
    while(UARTBusy(UART1_BASE));  // Wait for last char to finish
    SysCtlDelay(160);  // Small delay to ensure transmission completes
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
}


void WAIT_FOR_MESSAGE(void)
{
    if (UARTCharsAvail(UART1_BASE))
    {
        char receivedChar = UARTCharGet(UART1_BASE);
        
        // TEST LED: Flash blue when character received
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        SysCtlDelay(800000);  // ~50ms
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

        if (receivedChar == '#')  // End of message marker
        {
            // TEST LED: Long flash red when message complete
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
            SysCtlDelay(4000000);  // ~250ms
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
            
            rxBuffer[rxIndex] = '\0';  // Null-terminate the string
            rxIndex = 0;              // Prepare for next message

            // Make a copy for parsing (strtok modifies the buffer)
            char rxBufferCopy[RX_BUFFER_SIZE];
            strcpy(rxBufferCopy, rxBuffer);

            // Expected formats:
            //   0,12345#              ? normal unlock
            //   1,54321#              ? enter admin mode
            //   1,54321,67890#        ? admin: change user password to 67890

            char *modeStr;
            char *pass1Str;
            char *pass2Str;

            modeStr = strtok(rxBufferCopy, ",");
            pass1Str = strtok(NULL, ",");
            pass2Str = strtok(NULL, ",");  // May be NULL

            if (modeStr == NULL || pass1Str == NULL)
            {
                // Invalid format - send error
                UART1_SendString("Invalid Format#");
                memset(rxBuffer, 0, sizeof(rxBuffer));
                return;
            }

            // ------------------------------------------------------------------
            // Mode 0: Normal unlock with user password
            // ------------------------------------------------------------------
            if (modeStr[0] == '0' && modeStr[1] == '\0')
            {
                if (strcmp(pass1Str, PASSWORD) == 0)
                {
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
                    SysCtlDelay(16000000);
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
                    if(c==0){
                      UART1_SendString("1#");
                      c++;
                    }else {
                      UART1_SendString("2#");
                    }
                    failedAttempts = 0;
                }
                else
                {
                    failedAttempts++;
                    UART1_SendString("Wrong Password#");
                    
                    if (failedAttempts >= 3)
                    {
                        BUZZ();
                        failedAttempts = 0;
                    }
                }
            }
            // ------------------------------------------------------------------
            // Mode 1: Change password
            // ------------------------------------------------------------------
            else if (modeStr[0] == '1' && modeStr[1] == '\0')
            {
                if (strcmp(pass1Str, PASSWORD) == 0)
                {
                    if (pass2Str != NULL && pass2Str[0] != '\0')
                    {
                        UART1_SendString("Password Changed#");
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
                        SysCtlDelay(16000000);
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
                    }
                    else
                    {
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
                        SysCtlDelay(8000000);
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
                    }
                    failedAttempts = 0;
                }
                else
                {
                    failedAttempts++;
                    UART1_SendString("Wrong Password#");
                    
                    if (failedAttempts >= 3)
                    {
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
                        SysCtlDelay(16000000);
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
                        failedAttempts = 0;
                    }
                }
            }
            // ------------------------------------------------------------------
            // Mode 2: Timeout setting
            // ------------------------------------------------------------------
            else if (modeStr[0] == '2' && modeStr[1] == '\0')
            {
                if(strcmp(pass1Str, "26") == 0) {
                    UART1_SendString("Timeout saved#");
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
                    SysCtlDelay(16000000);
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
                }
            }

            memset(rxBuffer, 0, sizeof(rxBuffer));
        }
        else  // Normal character received
        {
            if (rxIndex < RX_BUFFER_SIZE - 1)
            {
                rxBuffer[rxIndex++] = receivedChar;
            }
            else
            {
                rxIndex = 0;
                memset(rxBuffer, 0, sizeof(rxBuffer));
            }
        }
    }
}


int main(void) {
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  LED_Init();
  UART1_Init();
  SysCtlDelay(SysCtlClockGet() / 10);  // 100ms delay
  UART1_SendString("Ready#");
    
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Start with LED OFF
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); // Start with LED OFF
    
    //short c =0;
    
    //short mode =0;
    while(1) {
      WAIT_FOR_MESSAGE();
    }
}