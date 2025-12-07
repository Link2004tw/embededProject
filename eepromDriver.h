#include "driverlib/eeprom.h"

void eepromInit(){

  // 1. Enable the EEPROM Peripheral (moved from last line to first one)
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);

  // 2. Wait for the EEPROM peripheral to be ready (Crucial Step)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0))
  {
  }

  // 3. Initialize the EEPROM. This handles recovery if a power failure occurred during a write.
  uint32_t eepromStatus = EEPROMInit();


  // Ana mesh 3aref eh el 4 lines dol
  uint32_t pui32Data[2];
  uint32_t pui32Read[2];
  pui32Data[0] = 0x00000001;
  pui32Data[1] = 0x00000002;


  SysCtlDelay(20000000);

}

bool VerifyPassword(uint32_t *pui32UserPassword, uint32_t ui32Count) {
  // Buffer to hold data read from EEPROM (Adjust size as needed)
  uint32_t pui32ReadBuffer[16];

  // Safety check: Ensure we don't overflow our stack buffer
  if(ui32Count > 16) {
    return false; // Error: Password too long for buffer
  }

  // Read the stored password from EEPROM
  // Parameters: Destination pointer, EEPROM Address (offset), Byte Count
  EEPROMRead(pui32ReadBuffer, PASSWORD_START_ADDR, (ui32Count * sizeof(uint32_t)));

  // Compare user input with stored data
  for (int i = 0; i < ui32Count; i++) {
    if (pui32UserPassword[i] != pui32ReadBuffer[i]) {
      return false; // Mismatch found
    }
  }

  return true; // Passwords are identical
}