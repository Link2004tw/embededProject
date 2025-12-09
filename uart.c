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
   UART0 INITIALIZATION
   - Enables UART0
   - Enables GPIO Port A
   - Maps PA0/PA1 to UART
   - Sets baud rate and frame format
   - Enables UART interrupts
   ===================================================================== */
void UART0_Init(void)
{
    // -----------------------------------------------------------------
    // 1) Enable the peripheral clocks for UART0 and GPIOA
    //    (The hardware is OFF by default to save power)
    // -----------------------------------------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Wait until the peripherals are ready for use
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)) {}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {}

    // -----------------------------------------------------------------
    // 2) Configure PA0 and PA1 to function as UART0 pins
    //    PA0 → U0RX   (input)
    //    PA1 → U0TX   (output)
    // -----------------------------------------------------------------
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    // Apply UART mode to pins PA0 + PA1
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // -----------------------------------------------------------------
    // 3) Configure UART0 for:
    //      - 9600 baud
    //      - 8 data bits
    //      - No parity
    //      - One stop bit
    // -----------------------------------------------------------------
    UARTConfigSetExpClk(
        UART0_BASE,
        SysCtlClockGet(),  // Get current system clock
        9600,              // Baud rate
        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE
    );

    // -----------------------------------------------------------------
    // 4) Enable interrupts:
    //      - RX interrupt only (fires when a byte arrives)
    //      - Enable UART0 inside NVIC
    // -----------------------------------------------------------------
    UARTIntEnable(UART0_BASE, UART_INT_RX);
    IntEnable(INT_UART0);   // Enable interrupt vector in NVIC

    // -----------------------------------------------------------------
    // 5) Enable UART0 module (TX + RX)
    // -----------------------------------------------------------------
    UARTEnable(UART0_BASE);
}


/* =====================================================================
   UART0 INTERRUPT HANDLER
   Runs automatically whenever UART receives a byte.
   ===================================================================== */
void UART0_Handler(void)
{
    // Read interrupt status (which event occurred?)
    uint32_t status = UARTIntStatus(UART0_BASE, true);

    // Clear all UART interrupt flags we just read
    UARTIntClear(UART0_BASE, status);

    // If the interrupt came from "RX data available"
    if (status & UART_INT_RX)
    {
        // Read received byte without blocking the CPU
        rxData = UARTCharGetNonBlocking(UART0_BASE);

        // Tell main loop that new data is available
        rxFlag = 1;
    }
}
