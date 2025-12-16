/*****************************************************************************
 * File: potentiometer_manager.c
 * Description: Application-level Potentiometer Manager Implementation
 * 
 * Integrates the HAL potentiometer driver with the display system
 * for user-friendly timeout configuration.
 *****************************************************************************/

#include "potentiometer_manager.h"
#include <stdbool.h>
#include "display_manager.h"
#include "input_manager.h"
#include "../HAL/Potentiometer/potentiometer.h"
#include "../HAL/LCD/lcd.h"
#include "driverlib/sysctl.h"

/*****************************************************************************
 * Macros and Constants
 *****************************************************************************/

#define TIMEOUT_DISPLAY_UPDATE_INTERVAL  100  /* Update LCD every 100ms */
#define TIMEOUT_CONFIRM_KEY              '=' /* Key to save timeout */
#define DEFAULT_TIMEOUT                  10   /* Default timeout in seconds */

/*****************************************************************************
 * Static Variables
 *****************************************************************************/

/* Stores the confirmed timeout value (5-30 seconds) */
static uint8_t stored_timeout = DEFAULT_TIMEOUT;



/*****************************************************************************
 * Function: PotentiometerManager_GetTimeout
 * 
 * Description:
 *   Returns the stored (confirmed) timeout value.
 *   This is the timeout that was saved by the user.
 * 
 * Parameters: None
 * Returns:
 *   uint8_t - Stored timeout value in seconds (5-30)
 *****************************************************************************/
uint8_t PotentiometerManager_GetTimeout(void)
{
    return stored_timeout;
}

/*****************************************************************************
 * Function: PotentiometerManager_HandleTimeoutConfig
 * 
 * Description:
 *   Handles the "Set Auto-Lock Timeout" menu flow.
 *   
 *   User Flow:
 *     1. Display "Adjust Timeout" on LCD
 *     2. Show live timeout value (5-30 seconds) from potentiometer
 *     3. Update LCD as user adjusts potentiometer
 *     4. User presses '#' to confirm and save
 *     5. Application should prompt for password verification
 *        before saving to EEPROM
 * 
 *   LCD Display:
 *     Line 1: "Adjust Timeout"
 *     Line 2: "Time: XX sec"
 * 
 * Parameters: None
 * Returns: None
 *****************************************************************************/
void PotentiometerManager_HandleTimeoutConfig(void)
{
    uint8_t current_timeout;
    uint8_t last_displayed_timeout = 0;
    uint32_t update_counter = 0;
    char key;
    
    /* Display menu title */
    DISPLAY_ClearScreen();
    DISPLAY_ShowMessage("Adjust Timeout");
    SysCtlDelay(533333);  /* 100ms delay */
    
    /* Main loop for timeout configuration */
    current_timeout = PotentiometerManager_GetTimeout();
    while (1)
    {
        /* Increment counter to control update rate */
        update_counter++;
        
        /* Get current timeout value from potentiometer */
         
        current_timeout = Potentiometer_ReadTimeout();
        /* Update display only when value changes or after timeout interval
         * This reduces LCD writes for smoother operation
         */
        if ((current_timeout != last_displayed_timeout) || 
            (update_counter >= TIMEOUT_DISPLAY_UPDATE_INTERVAL))
        {
            /* Position cursor on second line */
            DISPLAY_ClearScreen();
            DISPLAY_ShowMessage("Adjust Timeout");
            LCD_SetCursor(1, 0);
            
            /* Display timeout value */
            LCD_WriteString("Time: ");
            if (current_timeout < 10)
            {
                LCD_WriteChar('0');
            }
            LCD_WriteChar('0' + (current_timeout / 10));
            LCD_WriteChar('0' + (current_timeout % 10));
            LCD_WriteString(" sec");
            
            /* Update state */
            last_displayed_timeout = current_timeout;
            update_counter = 0;
        }
        
        /* Check for user input */
        key = InputManager_GetKey();
        
        if (key == TIMEOUT_CONFIRM_KEY)
        {
            /* User pressed '#' to confirm timeout value
             * Save the timeout and return to caller
             */
            stored_timeout = current_timeout;  /* Store confirmed value */
            char message[6];
            message[0] ='2'; //mode
            message[1]=',';
            message[2]= '0' + (stored_timeout / 10);
            message[3]= '0' + (stored_timeout % 10);
            message[4] ='#';
            message[5] = '\0';
            DISPLAY_ClearScreen();
            DISPLAY_ShowMessage("Timeout Saved!");
            LCD_SetCursor(1, 0);
            LCD_WriteChar('0' + (stored_timeout / 10));
            LCD_WriteChar('0' + (stored_timeout % 10));
            //LCD_WriteChar(message[2]);
            //LCD_WriteChar(message[3]);
            LCD_WriteString(" seconds");
            //LCD_Clear();
            //LCD_WriteString(message);
            UART5_SendString(message);
            char *ack_buffer;
            LCD_Clear();
            DISPLAY_ShowMessage("Waiting for Ack...");
            UART5_ReceiveString(ack_buffer, 20);
            DISPLAY_ShowMessage(ack_buffer);
            SysCtlDelay(1066666);  /* 200ms delay */
            break;
        }
        
        /* Small delay to prevent busy-waiting */
        SysCtlDelay(266650);  /* ~50ms delay */
    }
}