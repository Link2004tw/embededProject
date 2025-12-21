#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "tm4c123gh6pm.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "../UART/HMI_Comm.h"
#include "../BACK/EEPROM/eepromDriver.h"

void Timer0A_Init();
void Timer0A_Handler(void);
void TimerStart(uint32_t timerBase);