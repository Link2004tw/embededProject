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
#include "driverlib/gpio.h"

#define RED_LED     GPIO_PIN_1
#define BLUE_LED    GPIO_PIN_2
#define GREEN_LED   GPIO_PIN_3

// **********************
// This function Should be modified to maintain the style we learned using the .h files
// **********************
void SetupLEDs(void) {
  // Enable the GPIO port for LEDs.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

  // Check if the peripheral access is enabled.
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
  {
  }

  // Enable the GPIO pins for the LED (PF1, PF2, PF3).
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED | BLUE_LED | GREEN_LED);
}

int main() {
  // 1. Setup System Clock (as you had it) (moved from eepromDriver to main)
  SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

  // 2. Initialize EEPROM
  eepromInit();

  // 3. Example: Verify a password
  uint32_t userEnteredPass[2] = {0x00000001, 0x00000002}; // The input to check

  // 4. Verify the password
  if(VerifyPassword(userEnteredPass, 2)) {
    // Password Correct - Unlock System - Turn ON Green LED
    // Should be handled so to match the .h file
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED | BLUE_LED | GREEN_LED, GREEN_LED);
  } else {
    // Password Incorrect - Access Denied - Turn ON Red LED
    // Should be handled so to match the .h file
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED | BLUE_LED | GREEN_LED, RED_LED);
  }
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
  
  
  
  char key = KEYPAD_GetMappedKey();
if(key)
{
    // Send key to display module
    DISPLAY_HandleKey(key);

    // Send key to UART module
    UART_SendByte(key);
}

}
