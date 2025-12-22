#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "inc/hw_memmap.h"

/*****************************************************************************
 * Macros and Constants
 *****************************************************************************/

/* ADC Configuration */
#define ADC_MODULE              0           /* ADC0 */
#define ADC_SEQUENCER           3           /* Sequencer 3 */
#define ADC_CHANNEL             3           /* PE0 = Channel 3 */
#define ADC_MAX_VALUE           4095        /* 12-bit resolution */

/* Timeout Configuration */
#define TIMEOUT_MIN_SECONDS     5           /* Minimum timeout */
#define TIMEOUT_MAX_SECONDS     30          /* Maximum timeout */
#define TIMEOUT_RANGE           (TIMEOUT_MAX_SECONDS - TIMEOUT_MIN_SECONDS)

/* GPIO Configuration for Potentiometer */
#define POT_PORT                PORTE       /* Port E */
#define POT_PIN                 0           /* Pin 0 (PE0) */

/*****************************************************************************
 * Function Prototypes
 *****************************************************************************/

/*
 * Potentiometer_Init
 * 
 * Description:
 *   Initializes the ADC module for potentiometer reading.
 *   - Enables ADC0 peripheral
 *   - Configures PE0 as analog input
 *   - Sets up ADC sequencer for single-ended sampling
 *   - Configures 12-bit resolution
 * 
 * Parameters: None
 * Returns: None
 * 
 * Must be called before using Potentiometer_ReadTimeout().
 */
void Potentiometer_Init(void);

/*
 * Potentiometer_ReadRaw
 * 
 * Description:
 *   Reads the raw ADC value from the potentiometer.
 *   Returns the 12-bit digitized analog input.
 * 
 * Parameters: None
 * Returns: 
 *   uint16_t - Raw ADC value (0-4095)
 *   - 0: Minimum voltage (0V)
 *   - 4095: Maximum voltage (3.3V)
 */
uint16_t Potentiometer_ReadRaw(void);

/*
 * Potentiometer_ReadTimeout
 * 
 * Description:
 *   Reads the potentiometer and maps the analog value to timeout seconds.
 *   Linear mapping from 0-4095 ADC range to 5-30 seconds with calibration.
 *   
 *   Includes hardware calibration: If ADC reads above 95% of maximum (3890),
 *   it returns the full 30 seconds timeout. This compensates for potentiometers
 *   that don't reach the full voltage range.
 * 
 *   Formula (with calibration):
 *     if (ADC >= 3890) return 30;
 *     else Timeout(seconds) = ((ADC_Value / 3890) * 25) + 5
 *     Where 25 = (30 - 5) and 5 is the minimum timeout
 * 
 * Returns:
 *   uint8_t - Timeout value in seconds (5-30)
 */
uint8_t Potentiometer_ReadTimeout(void);

/*
 * Potentiometer_GetRawDebug
 * 
 * Description:
 *   Returns the current raw ADC reading for debugging purposes.
 *   Use this to check if your potentiometer is reaching the expected
 *   voltage range.
 *   
 *   Expected values:
 *   - Minimum position: 0-200
 *   - Maximum position: 3800-4095 (ideally, but often 3000-3900 in practice)
 *   
 * Returns:
 *   uint16_t - Current raw ADC value (0-4095)
 */
#define Potentiometer_GetRawDebug() Potentiometer_ReadRaw()

#endif 