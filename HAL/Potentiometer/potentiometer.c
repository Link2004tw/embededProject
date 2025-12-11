/*****************************************************************************
 * File: potentiometer.c
 * Description: Potentiometer ADC Driver Implementation for TM4C123GH6PM
 * 
 * Hardware Setup:
 *   - ADC Module: ADC0
 *   - Input Pin: PE0 (ADC Channel 3)
 *   - Sequencer: Sequencer 3 (single-sample, lowest priority)
 *   - Resolution: 12-bit (0-4095)
 * 
 * EEPROM Configuration:
 *   The timeout value is saved to EEPROM by the Control_ECU
 *   via UART communication from the HMI_ECU.
 *****************************************************************************/

#include "potentiometer.h"



/*****************************************************************************
 * Function: Potentiometer_Init
 * 
 * Description:
 *   Initializes the ADC0 module for reading the potentiometer on PE0.
 *   Configures the GPIO pin as analog input and sets up ADC sequencer.
 *****************************************************************************/
void Potentiometer_Init(void)
{
    /* Enable ADC0 peripheral clock */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)); //make sure ADC0 is ready
    
    /* Enable Port E peripheral clock */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
    
    /* Configure PE0 as analog input (disable digital I/O) */
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);
    
    /* Guarantees that the analog mode is enabled on PE0 */
    GPIO_PORTE_AMSEL_R |= 0x01;  /* Enable analog function on PE0 */
    GPIO_PORTE_DEN_R &= ~0x01;   /* Disable digital I/O on PE0 */
    
    /* Configure ADC0 Sequencer 3
     * Sequencer 3 is used for single-sample conversions
     * Priority: lowest (3), so it runs after other sequencers
     */
    
    /* Disable sequencer 3 before configuration */
    ADCSequenceDisable(ADC0_BASE, ADC_SEQUENCER);
    
    /* Configure the ADC sequencer
     * - Sample from PE0 (channel 3)
     * - Trigger on processor write to ADCPSSI
     */
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQUENCER, 
                         ADC_TRIGGER_PROCESSOR, 3);
    
    /* Configure the step in the sequencer
     * - Channel 3 (PE0)
     * - Mark as end of sequence (ADC_CTL_END)
     * - Enable interrupt on this step
     */
    ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQUENCER, 0,
                             ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);
    
    /* Enable sequencer 3 */
    ADCSequenceEnable(ADC0_BASE, ADC_SEQUENCER);
    
    /* Clear any pending interrupt flag */
    ADCIntClear(ADC0_BASE, ADC_SEQUENCER);
}

/*****************************************************************************
 * Function: Potentiometer_ReadRaw
 * 
 * Description:
 *   Reads the raw 12-bit ADC value from the potentiometer.
 *   Performs a single conversion and returns the result.
 * 
 * Returns:
 *   uint16_t - Raw ADC value (0-4095)
 *****************************************************************************/
uint16_t Potentiometer_ReadRaw(void)
{
    uint32_t adc_value = 0;
    
    /* Clear any pending interrupt flag */
    ADCIntClear(ADC0_BASE, ADC_SEQUENCER);
    
    /* Trigger ADC conversion on sequencer 3 */
    ADCProcessorTrigger(ADC0_BASE, ADC_SEQUENCER);
    
    /* Wait for conversion to complete
     * Poll the ADC interrupt status register
     */
    while (!ADCIntStatus(ADC0_BASE, ADC_SEQUENCER, false));
    
    /* Read the conversion result
     * ADC stores result in the SSFIFO (Sequencer Sample FIFO)
     */
    ADCSequenceDataGet(ADC0_BASE, ADC_SEQUENCER, &adc_value);
    
    /* Return the 12-bit value */
    return (uint16_t)(adc_value & 0xFFF);
}

/*****************************************************************************
 * Function: Potentiometer_ReadTimeout
 * 
 * Description:
 *   Reads the potentiometer and maps the analog value to timeout seconds.
 *   Uses linear mapping: Timeout = ((ADC_Value / 4095) * 25) + 5
 * 
 *   This allows users to adjust timeout from 5 to 30 seconds via
 *   the potentiometer, displayed live on the LCD.
 * 
 * Returns:
 *   uint8_t - Timeout value in seconds (5-30)
 *****************************************************************************/
uint8_t Potentiometer_ReadTimeout(void)
{
    uint16_t adc_raw;
    uint32_t timeout_value;
    
    /* Read raw ADC value */
    adc_raw = Potentiometer_ReadRaw();
    
    /* Map ADC value (0-4095) to timeout range (5-30 seconds)
     * Formula: Timeout = ((ADC / 4095) * (MAX - MIN)) + MIN
     */
    timeout_value = ((adc_raw * TIMEOUT_RANGE) / ADC_MAX_VALUE) + TIMEOUT_MIN_SECONDS;
    
    /* Ensure value is within bounds (safety check) */
    if (timeout_value > TIMEOUT_MAX_SECONDS)
    {
        timeout_value = TIMEOUT_MAX_SECONDS;
    }
    else if (timeout_value < TIMEOUT_MIN_SECONDS)
    {
        timeout_value = TIMEOUT_MIN_SECONDS;
    }
    
    return (uint8_t)timeout_value;
}