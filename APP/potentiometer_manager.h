/*****************************************************************************
 * File: potentiometer_manager.h
 * Description: Application-level Potentiometer Manager
 * 
 * Purpose:
 *   Provides a high-level interface for the potentiometer functionality.
 *   Handles user interaction when setting the auto-lock timeout value.
 *   Displays live timeout values on the LCD as the user adjusts the potentiometer.
 *****************************************************************************/

#ifndef POTENTIOMETER_MANAGER_H
#define POTENTIOMETER_MANAGER_H
#include "../Comm/HMI_Comm.h"
#include <stdbool.h>
#include "display_manager.h"
#include "input_manager.h"
#include "../HAL/Potentiometer/potentiometer.h"
#include "../HAL/LCD/lcd.h"
#include "driverlib/sysctl.h"
#include <stdint.h>

/*****************************************************************************
 * Function Prototypes
 *****************************************************************************/

/*
 * PotentiometerManager_Init
 * 
 * Description:
 *   Initializes the potentiometer manager.
 *   This is a placeholder for future enhancements (e.g., calibration, history).
 * 
 * Parameters: None
 * Returns: None
 */
void PotentiometerManager_Init(void);

/*
 * PotentiometerManager_HandleTimeoutConfig
 * 
 * Description:
 *   Main handler for the "Set Auto-Lock Timeout" menu option.
 *   Displays live timeout value on LCD as user adjusts the potentiometer.
 *   User presses '#' to save the selected timeout value.
 * 
 *   LCD Display Format:
 *     Line 1: "Adjust Timeout"
 *     Line 2: "Time: XX sec"  (where XX is 5-30)
 * 
 * Parameters: None
 * Returns: None
 * 
 * Note:
 *   - Blocks in a loop until user presses '#' to confirm
 *   - The timeout value should be saved to EEPROM via Control_ECU
 *     after the user enters their password
 */
void PotentiometerManager_HandleTimeoutConfig(void);

/*
 * PotentiometerManager_GetTimeout
 * 
 * Description:
 *   Returns the current timeout value from the potentiometer.
 *   Used for displaying live feedback during configuration.
 * 
 * Parameters: None
 * Returns:
 *   uint8_t - Current timeout value in seconds (5-30)
 */
uint8_t PotentiometerManager_GetTimeout(void);

#endif /* POTENTIOMETER_MANAGER_H */