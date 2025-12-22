/*****************************************************************************
 * File: potentiometer_manager.c
 * Description: Application-level Potentiometer Manager Implementation
 * 
 * Integrates the HAL potentiometer driver with the display system
 * for user-friendly timeout configuration.
 *****************************************************************************/

#include "potentiometer_manager.h"


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
    char message[20];
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
            /* Optional: Read raw ADC value for debugging 
             * Uncomment the lines below to see the actual ADC reading
             * This helps diagnose if the potentiometer is reaching full range
             */
            // uint16_t raw_adc = Potentiometer_GetRawDebug();
            
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
            
            /* Debug display (optional) - shows raw ADC value
             * Uncomment to diagnose potentiometer range issues:
             */
            // LCD_SetCursor(1, 11);
            // LCD_WriteChar('A');
            // LCD_WriteChar('D');
            // LCD_WriteChar('C');
            // LCD_WriteChar(':');
            // char digit4 = '0' + ((raw_adc / 1000) % 10);
            // char digit3 = '0' + ((raw_adc / 100) % 10);
            // char digit2 = '0' + ((raw_adc / 10) % 10);
            // char digit1 = '0' + (raw_adc % 10);
            // if (digit4 != '0') LCD_WriteChar(digit4);
            // if (digit4 != '0' || digit3 != '0') LCD_WriteChar(digit3);
            // LCD_WriteChar(digit2);
            // LCD_WriteChar(digit1);
            
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
            message[0] ='2'; //mode
            message[1]=',';
            message[2]= '0' + (stored_timeout / 10);
            message[3]= '0' + (stored_timeout % 10);
            message[4] =',';
            //message[5] = '\0';
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
              /* 200ms delay */
            break;
        }
            SysCtlDelay(266650);  /* ~50ms delay */
    }

        short pass_index = 5;
        key = 0;
        //1 , 1 2 3 4 5 #
        //0 1 2 3 4 5 6 7
        LCD_Clear();
        LCD_WriteString("Enter password");
        LCD_SetCursor(1, 0);
        //2 , 05 , 12345#
        //0 1 23 456789
        for(pass_index = 5; pass_index < 10; pass_index++){
            key = InputManager_GetKey();
            while(key == 0) {  
                key = InputManager_GetKey();
                if(key=='=') return;
                
                SysCtlDelay(10000);
            }
            message[pass_index] = key;
            LCD_WriteChar(key);
        }
        message[10] ='#';
        message[11] = '\0';
        
        UART5_SendString(message);
        char ack_buffer[20];
        LCD_Clear();
        DISPLAY_ShowMessage("Waiting for Ack...");
        UART5_ReceiveStringWithTimeout(ack_buffer, 20);
        if(ack_buffer != NULL){
            SHOW_BUFFER(ack_buffer);
        }
        /* Small delay to prevent busy-waiting */
    
}