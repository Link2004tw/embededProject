#include "HMI_Comm.h"
#define PASSWORD "12345" // will be changed to eeprom later
char rxBuffer1[RX_BUFFER_SIZE]="";
uint8_t rxIndex1 = 0;

short failedAttempts1 =0;
void UART1_Init1(void) {
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
    SysCtlDelay(1000);  // Add delay to let UART settle
    
    while (UARTCharsAvail(UART1_BASE)) {
        UARTCharGet(UART1_BASE);
    }
    //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Start with LED OFF
    
    
}

void BUZZ1(void){
GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // Red ON
                            SysCtlDelay(16000000);
                            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

}


void UART1_SendString1(char* str) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    
    while(*str) {
        while(UARTBusy(UART1_BASE));  // Wait until TX ready
        UARTCharPut(UART1_BASE, *str);
        str++;
    }

    while(UARTBusy(UART1_BASE));  // Wait for last char to finish
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
}


void WAIT_FOR_MESSAGE1(void)
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
            
            rxBuffer1[rxIndex1] = '\0';  // Null-terminate the string
            rxIndex1 = 0;              // Prepare for next message

            // Make a copy for parsing (strtok modifies the buffer)
            char rxBuffer1Copy[RX_BUFFER_SIZE];
            strcpy(rxBuffer1Copy, rxBuffer1);

            // Expected formats:
            //   0,12345#              → normal unlock
            //   1,54321#              → enter admin mode
            //   1,54321,67890#        → admin: change user password to 67890

            char *modeStr;
            char *pass1Str;
            char *pass2Str;

            modeStr = strtok(rxBuffer1Copy, ",");
            pass1Str = strtok(NULL, ",");
            pass2Str = strtok(NULL, ",");  // May be NULL

            if (modeStr == NULL || pass1Str == NULL)
            {
                // Invalid format - send error
                UART1_SendString("Invalid Format#");
                memset(rxBuffer1, 0, sizeof(rxBuffer1));
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
                    UART1_SendString("Correct Password#");
                    failedAttempts1 = 0;
                }
                else
                {
                    failedAttempts1++;
                    UART1_SendString("Wrong Password#");
                    
                    if (failedAttempts1 >= 3)
                    {
                        BUZZ();
                        failedAttempts1 = 0;
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
                    failedAttempts1 = 0;
                }
                else
                {
                    failedAttempts1++;
                    UART1_SendString("Wrong Password#");
                    
                    if (failedAttempts1 >= 3)
                    {
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
                        SysCtlDelay(16000000);
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
                        failedAttempts1 = 0;
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

            memset(rxBuffer1, 0, sizeof(rxBuffer1));
        }
        else  // Normal character received
        {
            if (rxIndex1 < RX_BUFFER_SIZE - 1)
            {
                rxBuffer1[rxIndex1++] = receivedChar;
            }
            else
            {
                rxIndex1 = 0;
                memset(rxBuffer1, 0, sizeof(rxBuffer1));
            }
        }
    }
}

