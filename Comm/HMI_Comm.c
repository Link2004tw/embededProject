#include "HMI_Comm.h"

// Initialize UART5 on PE4 (Rx) and PE5 (Tx)
void UART5_Init_front(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART5));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // PF3 is Green LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF
    

    
    UARTEnable(UART5_BASE);
    SysCtlDelay(1000);  // Add delay to let UART settle
    
    while (UARTCharsAvail(UART5_BASE)) {
        UARTCharGet(UART5_BASE);
    }
    //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Start with LED OFF
    
    
}


void UART5_SendString(char* str) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    
    while(*str) {
        while(UARTBusy(UART5_BASE));  // Wait until TX ready
        UARTCharPut(UART5_BASE, *str);
        str++;
    }
    
    while(UARTBusy(UART5_BASE));  // Wait for last char to finish
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
}

// uint8_t UART_ReceiveByte(void) {
//     while(UARTBusy(UART5_BASE));
//     return UARTCharGet(UART5_BASE);
// }

void UART5_ReceiveString(char* buffer, uint16_t max_length) {
    uint16_t index = 0;
    char received_char;
    
    while(index < max_length - 1) {  // Leave room for null terminator
        // Wait for data to be available
        while(!UARTCharsAvail(UART5_BASE));
        
        received_char = UARTCharGet(UART5_BASE);
        
        // Check for end-of-string conditions (newline or carriage return)
        if(received_char == '#') {
            break;
        }
        
        buffer[index++] = received_char;
    }
    
    buffer[index] = '\0';  // Null terminate the string
}
