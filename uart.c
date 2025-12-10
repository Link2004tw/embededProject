#include <stdint.h>
#include <stdbool.h>

// This tells DriverLib which exact microcontroller you are using
#define PART_TM4C123GH6PM 

/* Low-level hardware register definitions */
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/tm4c123gh6pm.h"

/* TivaWare DriverLib (high-level API) */
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

#include "uart.h"   // Your UART header (function prototypes)

/* Global variables updated inside the interrupt */
volatile uint8_t rxData = 0;   // Stores the last received byte
volatile uint8_t rxFlag = 0;   // Tells main() that new data arrived


/* =====================================================================
   UART1 INITIALIZATION
   - Enables UART1
   - Enables GPIO Port B
   - Maps PB0/PB1 to UART
   - Sets baud rate and frame format
   - Enables UART interrupts
   ===================================================================== */
void UART1_Init(void)
{
    // -----------------------------------------------------------------
    // 1) Enable the peripheral clocks for UART1 and GPIOB
    //    (The hardware is OFF by default to save power)
    // -----------------------------------------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Wait until the peripherals are ready for use
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART1)) {}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)) {}

    // -----------------------------------------------------------------
    // 2) Configure PB0 and PB1 to function as UART1 pins
    //    PB0 → U1RX   (input)
    //    PB1 → U1TX   (output)
    // -----------------------------------------------------------------
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);

    // Apply UART mode to pins PB0 + PB1
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // -----------------------------------------------------------------
    // 3) Configure UART1 for:
    //      - 9600 baud
    //      - 8 data bits
    //      - No parity
    //      - One stop bit
    // -----------------------------------------------------------------
    UARTConfigSetExpClk(
        UART1_BASE,
        SysCtlClockGet(),  // Get current system clock
        9600,              // Baud rate
        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE
    );

    // -----------------------------------------------------------------
    // 4) Enable interrupts:
    //      - RX interrupt only (fires when a byte arrives)
    //      - Enable UART1 inside NVIC
    // -----------------------------------------------------------------
    UARTIntEnable(UART1_BASE, UART_INT_RX);
    IntEnable(INT_UART1);   // Enable interrupt vector in NVIC

    // -----------------------------------------------------------------
    // 5) Enable UART1 module (TX + RX)
    // -----------------------------------------------------------------
    UARTEnable(UART1_BASE);
}


/* =====================================================================
   UART1 INTERRUPT HANDLER
   Runs automatically whenever UART receives a byte.
   ===================================================================== */
void UART1_Handler(void)
{
    // Read interrupt status (which event occurred?)
    uint32_t status = UARTIntStatus(UART1_BASE, true);

    // Clear all UART interrupt flags we just read
    UARTIntClear(UART1_BASE, status);

    // If the interrupt came from "RX data available"
    if (status & UART_INT_RX)
    {
        // Read received byte without blocking the CPU
        rxData = UARTCharGetNonBlocking(UART1_BASE);

        // Tell main loop that new data is available
        rxFlag = 1;
    }
}
