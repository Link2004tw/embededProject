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
    while (UARTCharsAvail(UART5_BASE)) {
        UARTCharGet(UART5_BASE);
    }
    //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Start with LED OFF
    
    
}

void UART5_SendString(char* str) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Start with LED OFF
  
    while(*str) {
        UARTCharPut(UART5_BASE, *str);
        str++;
    }
    //SysCtlDelay(53333333);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF
    
}
