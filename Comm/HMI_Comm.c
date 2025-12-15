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
    
    UARTEnable(UART5_BASE);
}

void UART5_SendString(char* str) {
    while(*str) {
        UARTCharPut(UART5_BASE, *str);
        str++;
    }
}
//example how to use the uart
// int main(void) {
    // Set clock to 16MHz
//     SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

//     UART1_Init_front();

//     char dummyPassword[] = "12345#"; 

//     while(1) {
//         // Send the dummy password
//         UART1_SendString(dummyPassword);
        
//         // Wait 10 seconds
//         // SysCtlDelay count = (Seconds * Clock) / 3
//         // 10 * 16,000,000 / 3 = ~53,333,333
//         SysCtlDelay(53333333); 
//     }
// }