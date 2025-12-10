#include <stdint.h>
#include "tm4c123gh6pm.h"

#include "./HAL/keypad/keypad.h"              // Frontend 1 (TA driver)
#include "./APP/input_manager.h"       // Your mapped key function
#include "./APP/display_manager.h"     // Frontend 2
#include "driverlib/sysctl.h"
int main(void)
{
    /* ---------- Initialize all frontend modules ---------- */
    Keypad_Init();           // From TA keypad driver
    DISPLAY_Init();          // Initializes LCD
    DISPLAY_ClearScreen();   // Clear display

    /* ---------- Optional: show a startup message ---------- */
    DISPLAY_ShowMessage("Frontend Ready");
    SysCtlDelay(1000);   // 1 second (if SysTick exists)

    DISPLAY_ClearScreen();
    DISPLAY_ShowMainMenu();  // Show menu while testing

    /* ---------- Main Loop ---------- */
    while (1)
    {
        char key = InputManager_GetKey();  // Frontend 1 returns mapped char

        if (key != 0)
        {
            /* Display the character on LCD */
            DISPLAY_HandleKey(key);

            /* Debug: If needed, print raw or mapped value on second line */
            // LCD_SetCursor(1, 0);
            // LCD_WriteChar(key);

            /* Optional: Wait a bit to avoid flickering */
            SysCtlDelay(50);
        }
    }
}
