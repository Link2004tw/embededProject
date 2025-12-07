

/**
 * main.c
 */
/*changed file path of tm4c123gh6pm.h header file*/
#include "/Users/admin/Downloads/TivaWare_C_Series-2.2.0.295/inc/tm4c123gh6pm.h"
#include <stdint.h>
void buzzerInit(){
    SYSCTL_RCGCGPIO_R|=(1<<1);
    SYSCTL_PRGPIO_R&=((1<<1)==0);
    GPIO_PORTB_DIR_R|=(1<<2);
    GPIO_PORTB_DEN_R|=(1<<2);
}
void buzzerON(){
    GPIO_PORTB_DATA_R |=(1<<2);
}
void buzzerOFF(){
    GPIO_PORTB_DATA_R |=0;

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

int main(void)
{
    while(1){
    buzzerInit();}

    /**if(!password for 3 times){
     * buzzerOn();
     *
     * }
     * else
     * buzzerOFF();
    **/

	//return 0;

}
