#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

#include "HAL/Keypad/keypad.h"              // Frontend 1 (TA driver)      // Potentiometer ADC
#include "APP/input_manager.h"       // Your mapped key function
#include "APP/display_manager.h"     // Frontend 2
#include "APP/potentiometer_manager.h" // Potentiometer manager
#include "driverlib/sysctl.h"
#include "HAL/Potentiometer/potentiometer.h"



int main(void)
{
    /* ---------- Initialize all frontend modules ---------- */
    Keypad_Init();           // From TA keypad driver
    Potentiometer_Init();    // Initialize ADC for potentiometer
    DISPLAY_Init();          // Initializes LCD
    DISPLAY_ClearScreen();   // Clear display

    /* ---------- Optional: show a startup message ---------- */
    DISPLAY_ShowMessage("Frontend Ready");
    SysCtlDelay(5333300);  // 1s delay

    DISPLAY_ClearScreen();
    DISPLAY_ShowMainMenu();  // Show menu while testing

    

    /* ---------- Main Loop ---------- */
    while (1)
    {
        char key = InputManager_GetKey();  // Frontend 1 returns mapped char

        if (key != 0)
        {
            /* Handle menu selections */
            if (key == '*')
            {
                /* User pressed '*' - Set Auto-Lock Timeout */
                PotentiometerManager_HandleTimeoutConfig();
                
                /* Return to main menu after timeout config */
                DISPLAY_ClearScreen();
                DISPLAY_ShowMainMenu();
            }
            else if (key == '+')
            {
                /* User pressed '+' - Open Door (to be implemented) */
                DISPLAY_ShowMessage("Open Door");
                SysCtlDelay(1066666);  // 200ms
                DISPLAY_ClearScreen();
                DISPLAY_ShowMainMenu();
            }
            else if (key == '-')
            {
                /* User pressed '-' - Change Password (to be implemented) */
                DISPLAY_ShowMessage("Change Password");
                SysCtlDelay(1066666);  // 200ms
                DISPLAY_ClearScreen();
                DISPLAY_ShowMainMenu();
            }
            else
            {
                /* Display other characters on LCD */
                DISPLAY_HandleKey(key);
            }

            /* Optional: Wait a bit to avoid flickering */
            SysCtlDelay(266650);   // 50ms delay
        }
    }
}
