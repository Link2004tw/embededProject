/*
 * buzzer.c
 *
 *  Created on: Dec 21, 2025
 */

#include "buzzer.h"
/*changed file path of tm4c123gh6pm.h header file*/
#include "tm4c123gh6pm.h"

void buzzerInit(void){
    SYSCTL_RCGCGPIO_R|=(1<<1);
    while((SYSCTL_PRGPIO_R & (1<<1)) == 0);
    GPIO_PORTB_DIR_R|=(1<<2);
    GPIO_PORTB_DEN_R|=(1<<2);
}

void buzzerON(void){
    GPIO_PORTB_DATA_R |=(1<<2);
}

void buzzerOFF(void){
    GPIO_PORTB_DATA_R &= ~(1<<2);
}

// this assumes that the interrupt is caused by PF4
void GPIOF_InterruptInit(void){
    SYSCTL_RCGCGPIO_R |= (1<<5);      
    while((SYSCTL_PRGPIO_R & (1<<5)) == 0);

    GPIO_PORTF_LOCK_R = 0x4C4F434B;   
    GPIO_PORTF_CR_R |= 0x10;         

    GPIO_PORTF_DIR_R &= ~(1<<4);      
    GPIO_PORTF_DEN_R |= (1<<4);      
    GPIO_PORTF_PUR_R |= (1<<4);      

    GPIO_PORTF_IS_R &= ~(1<<4);      
    GPIO_PORTF_IBE_R &= ~(1<<4);     
    GPIO_PORTF_IEV_R &= ~(1<<4);      
    GPIO_PORTF_ICR_R |= (1<<4);       
    GPIO_PORTF_IM_R |= (1<<4);       

    NVIC_EN0_R |= (1<<30);            
}

void GPIOF_Handler(void){
    GPIO_PORTF_ICR_R |= (1<<4);       
    if (GPIO_PORTB_DATA_R & (1<<2))
        buzzerOFF();
    else
        buzzerON();
}
