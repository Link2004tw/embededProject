//this is done by polling as that ISR cant take inputs and return outputs and we need an input of seconds from the user
#include "tm4c123gh6pm.h"
#include "dio.h"
#include <stdint.h>
#include "gptm.h"

void timer_config(uint32_t interval){
    volatile uint32_t delay;
    //1. enable clock
    SYSCTL_RCGCTIMER_R |= 0x01;
    delay = SYSCTL_RCGCTIMER_R;
    //2. disable timer
    TIMER0_CTL_R &= ~0x01;
    //3. configure mode: 1. full size 2. one shot 3. count down
    TIMER0_CFG_R =0x00;//full size
    TIMER0_TAMR_R &= ~0x3;//clear the register
    TIMER0_TAMR_R |=0x1;//one shot & count down
    //TIMER0_TACDIR_R =0X00;//count down
    //4. load interval
    //assume a 16MHz clock
    TIMER0_TAILR_R= (interval*15999999);

    //5. clear timeout flag
    TIMER0_ICR_R = 0x01;
    TIMER0_IMR_R |= 0x01;
    NVIC_EN0_R |= (1 << 19);
   //6. start timer
    TIMER0_CTL_R|=0X01;
}

//void timer_wait(void)
//{
//    while (!(TIMER0_RIS_R & 0x01)) { }  // wait until timeout
//    TIMER0_ICR_R = 0x01;                // clear flag
//}

void Timer0A_Handler(void)
{
    GPIO_PORTF_DATA_R |= 0x02;
    TIMER0_ICR_R = 0x01;
}
/**
 * main.c
 */
int main(void)
{
    //load interval of 5 seconds
   timer_config(5);
   while(1);
	return 0;
}
