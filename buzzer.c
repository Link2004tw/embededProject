

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
