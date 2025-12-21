#include "TIMER.h"

// Example: Initialize one-shot timer with interrupt
void Timer0A_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);
   // IntMasterEnable();
}

void Timer0A_Handler(void)
{
    // Clear the interrupt flag
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Optional: turn off LED or buzzer if this was lockout
     GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0); // Start with LED OFF
 
}


void TimerStart()
{
    uint8_t timeoutSeconds;
    EEPROM_ReadTimeout(&timeoutSeconds);
    /* Convert seconds to ticks: 16MHz clock * seconds */
    /* 16,000,000 * timeoutSeconds */
    uint32_t loadVal = (uint32_t)timeoutSeconds * 16000000;
    
    // Safety check for 0 or overflow (though 30s * 16M fits in 32-bit)
    if(loadVal < 16000000) loadVal = 16000000; // Minimum 1 second safety? Or just trust logic
    
    TimerLoadSet(TIMER0_BASE, TIMER_A, loadVal);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void Timer1A_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));

    TimerDisable(TIMER1_BASE, TIMER_A);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER1A);
}

void Timer1A_Handler(void)
{
    // Clear the interrupt flag
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0); 
    // will be replaced with buzzer
}

void Timer1AStart(uint32_t loadVal)
{
    TimerLoadSet(TIMER1_BASE, TIMER_A, loadVal);
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER1_BASE, TIMER_A);
}

