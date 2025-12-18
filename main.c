#include "UART/HMI_Comm.h"
//#include <stdint.h>
//#include <stdbool.h>
//#include <string.h> 
//#include "tm4c123gh6pm.h"
//#include "inc/hw_memmap.h"
//#include "inc/hw_types.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/gpio.h"
//#include "driverlib/uart.h"
//#include "driverlib/pin_map.h"

void LED_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // PF3 is Green LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); // PF2 is Red LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // PF2 is Red LED
}

int main(void) {
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  LED_Init();
  UART1_Init();
  SysCtlDelay(SysCtlClockGet() / 10);  // 100ms delay
  //UART1_SendString("Ready#");
    
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Start with LED OFF
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); // Start with LED OFF
    
    //short c =0;
    
    //short mode =0;
    while(1) {
      WAIT_FOR_MESSAGE();
    }
}