#include "HMI_Comm.h"
#define PASSWORD "12345" // will be changed to eeprom later
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
    
    UARTEnable(UART1_BASE);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Start with LED OFF
    
    while (UARTCharsAvail(UART1_BASE)) {
        UARTCharGet(UART1_BASE);
      }
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF
    
}

void BUZZ(void){
GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // Red ON
                            SysCtlDelay(16000000);
                            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

}

void UART1_SendString(char* str) {
    //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    
    while(*str) {
        while(UARTBusy(UART1_BASE));  // Wait until TX ready
        UARTCharPut(UART1_BASE, *str);
        str++;
    }
    while(UARTBusy(UART1_BASE));  // Wait for last char to finish
    //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
}

void WAIT_FOR_MESSAGE(void)
{
    if (UARTCharsAvail(UART1_BASE))
    {
        char receivedChar = UARTCharGet(UART1_BASE);

        if (receivedChar == '#')  // End of message marker
        {
            rxBuffer[rxIndex] = '\0';  // Null-terminate the string
            rxIndex = 0;              // Prepare for next message

            // Expected formats:
            //   0,12345#              → normal unlock
            //   1,54321#              → enter admin mode
            //   1,54321,67890#        → admin: change user password to 67890

            char *modeStr;
            char *pass1Str;
            char *pass2Str;

            modeStr = strtok(rxBuffer, ",");
            pass1Str = strtok(NULL, ",");
            pass2Str = strtok(NULL, ",");  // May be NULL

            if (modeStr == NULL || pass1Str == NULL)
            {
                // Invalid format
                memset(rxBuffer, 0, sizeof(rxBuffer));
                return;
            }

            // ------------------------------------------------------------------
            // Mode 0: Normal unlock with user password
            // ------------------------------------------------------------------
            if (modeStr[0] == '0' && modeStr[1] == '\0')
            {
                if (strcmp(pass1Str, PASSWORD) == 0)  // Use stored password
                {
                    
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Green ON
                    SysCtlDelay(16000000);  // ~500ms at 16MHz
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
                    UART1_SendString("Correct Password#");
                    failedAttempts = 0;
                }
                else
                {
                    failedAttempts++;
                    UART1_SendString("Wrong Password#");
                        
                    if (failedAttempts >= 3)
                    {
                        BUZZ();  // Alarm for 3 wrong attempts
                        failedAttempts = 0;
                    }
                }
            }
            // ba8ayar el password
            else if (modeStr[0] == '1' && modeStr[1] == '\0')
            {
                if (strcmp(pass1Str, PASSWORD) == 0)  // Master password correct
                {
                    if (pass2Str != NULL && pass2Str[0] != '\0')
                    {
                        //save new user password
                        UART1_SendString("Password Changed#");
                          
                        // Confirm success
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Green
                        SysCtlDelay(16000000);  // 1 second
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
                        // Optional: save to Flash here for persistence
                        // SavePasswordToFlash(currentUserPassword);
                    }
                    else
                    {
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); // Blue LED?
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
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // Red ON
                        SysCtlDelay(16000000);
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
                        failedAttempts = 0;
                    }
                }
            }else if (modeStr[0] == '2' && modeStr[1] == '\0')
            {
                if(strcmp(pass1Str, "26") == 0){
                    UART1_SendString("Timeout saved#");
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // Blue ON
                    SysCtlDelay(16000000);
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
                }
            }

            // Clear buffer for next message
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
                // Buffer overflow → reset
                rxIndex = 0;
                memset(rxBuffer, 0, sizeof(rxBuffer));
            }
        }
    }
}

