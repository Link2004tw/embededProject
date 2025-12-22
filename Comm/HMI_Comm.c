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
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Start with LED OFF
     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); // PF3 is Green LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Start with LED OFF
     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // PF3 is Green LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); // Start with LED OFF

    
    UARTEnable(UART5_BASE);
    SysCtlDelay(1000);  // Add delay to let UART settle
    
    while (UARTCharsAvail(UART5_BASE)) {
        int x = UARTCharGet(UART5_BASE);
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

#define ACK_TIMEOUT_MS 1000  // 1 second timeout

void UART5_ReceiveStringWithTimeout(char* buffer, uint16_t max_length) {
    uint16_t index = 0;
    char c;
    uint32_t timeout_counter = 0;

    while(index < max_length - 1) {
        if(UARTCharsAvail(UART5_BASE)) {
            c = UARTCharGet(UART5_BASE);
            if(c == '#') break;
            buffer[index++] = c;
            timeout_counter = 0; // reset on receive
        // } else {
        //     timeout_counter++;
        //     if(timeout_counter > ACK_TIMEOUT_MS) break; // timeout reached
        //     SysCtlDelay(1000); // small delay
        }
    }

    buffer[index] = '\0';
}

// void UART5_ReceiveString(char* buffer, uint16_t max_length) {
//    uint16_t index = 0;
//    char received_char;
//    //uint32_t start_tick = SysTick->VAL; // or any timer tick for timeout
//    const uint32_t TIMEOUT_MS = 1000;   // 1 second

//    while(index < max_length - 1) {
//        if(UARTCharsAvail(UART5_BASE)) {
//            received_char = UARTCharGet(UART5_BASE);

//            if(received_char == '#') break;

//            buffer[index++] = received_char;

//            // Reset timeout on successful receive
//            //start_tick = SysTick->VAL;
//        }

//        // Check timeout (polling)
//     //    if((SysTick->VAL - start_tick) > (SysCtlClockGet() / 1000 * TIMEOUT_MS)) {
//     //        break;  // Timeout
//     //    }
//    }

//    buffer[index] = '\0'; // Null terminate string
// }


