#include "TIMER.h"

// Example: Initialize one-shot timer with interrupt
void Timer0A_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);
   // IntMasterEnable();
}

void Timer0A_Handler(void) {
    // Clear the interrupt flag
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Action to perform when timer expires
    Door_Lock(); // example: relock the door

    // Optional: turn off LED or buzzer if this was lockout
     GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Start with LED OFF
 
}


void TimerStart(uint32_t timeoutTicks) {
    TimerLoadSet(TIMER0_BASE, TIMER_A, timeoutTicks);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);
}

