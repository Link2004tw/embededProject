#include <stdint.h>
//#include "consts.h"
#include <stdbool.h> // four macros for boolean
#include "tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h" // for clock gating API
#include "driverlib/gpio.h" // for gpio APIs & GPIO_PINx definition
#include "inc/hw_memmap.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "HMI_Comm.h"

int main()
{ 
  UART0_Init();
  // LCD_Init();
  // Keypad_Init();
  //uint8_t testPass[5] = {'1','2','3','4','5'};
  
   while(1) {
     //GetUserPassword(PasswordBuffer); // keypad module fills the array
     HMI_SendPassword(PasswordBuffer, 5);
   }
}