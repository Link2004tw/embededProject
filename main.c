#include "UART/HMI_Comm.h"
#include "TIMER/TIMER.h"
#include "BACK/EEPROM/eepromDriver.h"
#include "HAL/password/password.h"
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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // PF3 is Green LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); // PF2 is Red LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // PF2 is Red LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Start with LED OFF
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); // Start with LED OFF
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2); // PF2 is Red LED
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0); // Start with LED OFF
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_6); // PF2 is Red LED
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0); // Start with LED OFF
    
}


int main(void) {
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  LED_Init();
  UART1_Init();
  Timer0A_Init();
  Timer1A_Init();
  uint8_t e =EEPROM_Init();
  if(e != EEPROM_SUCCESS){
  } //wala3 el denia;
  Password_Init();
  SysCtlDelay(SysCtlClockGet() / 10);  // 100ms delay
  IntMasterEnable();
  
  /* Check if password is initialized on startup */
  if (Password_IsInitialized() == 0)
  {
      /* Password not initialized - notify user via UART */
      UART1_SendString("SETUP_REQUIRED#");
      /* Visual indicator - blink LED to show setup needed */
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); // Red LED on
  }
  else
  {
      /* Password is initialized - system ready */
      UART1_SendString("READY#");
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Green LED on
      SysCtlDelay(SysCtlClockGet() / 3);  // Short blink
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Green LED off
  }
  
  //UART1_SendString("Ready#");
    
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); // Start with LED OFF
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); // Start with LED OFF
//    
    //short c =0;
    
    //short mode =0;
    while(1) {
      if(messageReady){
        messageReady = false;
        PROCESS_MESSAGE();
      }
    }
}