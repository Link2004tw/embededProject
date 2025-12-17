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

// void UART1_SendString(char* str) {
//     //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    
//     while(*str) {
//         while(UARTBusy(UART1_BASE));  // Wait until TX ready
//         UARTCharPut(UART1_BASE, *str);
//         str++;
//     }
//     while(UARTBusy(UART1_BASE));  // Wait for last char to finish
//     //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
// }


void UART1_SendString(char* str) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    
    while(*str) {
        while(UARTBusy(UART1_BASE));  // Wait until TX ready
        UARTCharPut(UART1_BASE, *str);
        str++;
    }
    
    while(UARTBusy(UART1_BASE));  // Wait for last char to finish
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

            // Parse manually to avoid strtok issues
            // Expected formats: "0,12345#", "1,54321#", "1,54321,67890#"
            
            if(rxBuffer[0] == '\0') {
                memset(rxBuffer, 0, sizeof(rxBuffer));
                return;  // Empty message
            }
            
            char mode = rxBuffer[0];
            
            // Find first comma
            char *comma1 = strchr(rxBuffer, ',');
            if(comma1 == NULL) {
                memset(rxBuffer, 0, sizeof(rxBuffer));
                return;  // No comma found
            }
            
            // Extract password between first and second comma (or end)
            char *comma2 = strchr(comma1 + 1, ',');
            int pass1Len = (comma2 != NULL) ? (comma2 - comma1 - 1) : (rxIndex - (comma1 - rxBuffer) - 1);
            
            char pass1[20] = "";
            strncpy(pass1, comma1 + 1, pass1Len);
            pass1[pass1Len] = '\0';
            
            char pass2[20] = "";
            if(comma2 != NULL) {
                int pass2Len = rxIndex - (comma2 - rxBuffer) - 1;
                strncpy(pass2, comma2 + 1, pass2Len);
                pass2[pass2Len] = '\0';
            }

            // ------------------------------------------------------------------
            // Mode 0: Normal unlock with user password
            // ------------------------------------------------------------------
            if (mode == '0')
            {
                if (strcmp(pass1, PASSWORD) == 0)
                {
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
                    SysCtlDelay(16000000);
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
                        BUZZ();
                        failedAttempts = 0;
                    }
                }
            }
            // ------------------------------------------------------------------
            // Mode 1: Change password
            // ------------------------------------------------------------------
            else if (mode == '1')
            {
                if (strcmp(pass1, PASSWORD) == 0)
                {
                    if (pass2[0] != '\0')
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
            else if (mode == '2')
            {
                if(strcmp(pass1, "26") == 0) {
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

