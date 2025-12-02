#include "driverlib/eeprom.h"

void eepromInit(){

  uint32_t pui32Data[2];
  uint32_t pui32Read[2];
  pui32Data[0] = 0x00000001;
  pui32Data[1] = 0x00000002;
  
  SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
  SysCtlDelay(20000000);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);

}