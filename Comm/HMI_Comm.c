#include "HMI_Comm.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

// Ring buffer variables
volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_head = 0;
volatile uint16_t rx_tail = 0;

// Internal function to check if buffer has data
bool UART5_IsDataAvailable(void) {
    return (rx_head != rx_tail);
}

// Internal function to read from buffer
char UART5_ReadFromBuffer(void) {
    if (rx_head == rx_tail) return 0; // Empty
    char c = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
    return c;
}

// UART5 Interrupt Service Routine
void UART5_Handler(void) {
    uint32_t status = UARTIntStatus(UART5_BASE, true);
    UARTIntClear(UART5_BASE, status);

    while (UARTCharsAvail(UART5_BASE)) {
        char c = UARTCharGetNonBlocking(UART5_BASE);
        // Store in Ring Buffer
        uint16_t next_head = (rx_head + 1) % RX_BUFFER_SIZE;
        if (next_head != rx_tail) { // Check for overflow
            rx_buffer[rx_head] = c;
            rx_head = next_head;
        }
        // Else: Drop character (Buffer Full)
    }
}

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
    
    // Enable UART FIFO
    UARTFIFOEnable(UART5_BASE);
    
    // Configure Interrupts (Receive and Receive Timeout)
    IntEnable(INT_UART5);
    UARTIntEnable(UART5_BASE, UART_INT_RX | UART_INT_RT);
    IntMasterEnable(); // Enable global interrupts

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // PF3 is Green LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Start with LED OFF
     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); // PF3 is Green LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Start with LED OFF
     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // PF3 is Green LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); // Start with LED OFF

    
    UARTEnable(UART5_BASE);
    SysCtlDelay(1000);  // Add delay to let UART settle
    
    // Clear hardware FIFO
    while (UARTCharsAvail(UART5_BASE)) {
        int x = UARTCharGet(UART5_BASE);
    }
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

#define ACK_TIMEOUT_MS 1000000  // Increased software loop counter for timeout (since we don't have accurate timer yet)

void UART5_ReceiveStringWithTimeout(char* buffer, uint16_t max_length) {
    uint16_t index = 0;
    char c;
    uint32_t timeout_counter = 0;

    while(index < max_length - 1) {
        if(UART5_IsDataAvailable()) { // Check SW Buffer instead of HW Polling
            c = UART5_ReadFromBuffer();
            if(c == '#') break;  // Changed from % to #
            buffer[index++] = c;
            timeout_counter = 0; // reset on receive
        } else {
            timeout_counter++;
            if(timeout_counter > ACK_TIMEOUT_MS) break; // timeout reached
            SysCtlDelay(100); // Small delay to prevent CPU spin
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


