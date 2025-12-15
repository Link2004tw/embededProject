#include <stdint.h>
#include "tm4c123gh6pm.h"

#include "./HAL/keypad/keypad.h"              // Frontend 1 (TA driver)
#include "./APP/input_manager.h"       // Your mapped key function
#include "./APP/display_manager.h"     // Frontend 2
#include "driverlib/sysctl.h"
#include "./HAL/Potentiometer/potentiometer.h"
#include "Comm/HMI_Comm.h"


void DELAY(void){
  SysCtlDelay(SysCtlClockGet());            
}

int main(void)
{
      
    /* ---------- Initialize all frontend modules ---------- */
    Keypad_Init();           // From TA keypad driver
    DISPLAY_Init();          // Initializes LCD
    Potentiometer_Init(); //initialize potentiometer
    UART5_Init_front();
    DISPLAY_ClearScreen();   // Clear display
    
    /* ---------- Optional: show a startup message ---------- */
    DISPLAY_ShowMessage("Frontend Ready");
    SysCtlDelay(1000);   // 1 second (if SysTick exists)

    DISPLAY_ClearScreen();
    DISPLAY_ShowMainMenu();  // Show menu while testing
    /* ---------- Main Loop ---------- */
    short mode = 0;
    while (1)
    {
        char key = InputManager_GetKey();  // Frontend 1 returns mapped char
        //printf(key);
        if (key != 0)
        {
            /* Display the character on LCD */
            
            //DISPLAY_HandleKey(key);
            if(key=='+'){
              mode =1;
            }else if(key=='-'){
              mode = 2;
            
            }else if(key=='*'){
              mode =3;
            }else {
              DISPLAY_ERROR();
              //SysCtlDelay(5000000);
              DISPLAY_ClearScreen();
              DISPLAY_ShowMainMenu();  // Show menu while testing
            }
            if(mode == 0){
              DISPLAY_ClearScreen();
              DISPLAY_ShowMainMenu();  // Show menu while testing
        }
        else if(mode == 1){
           DISPLAY_OPEN();
           DISPLAY_ClearScreen();
            //DISPLAY_ShowMessage("Hello i am out");
            DELAY();
            DISPLAY_ClearScreen();
            DISPLAY_ShowMainMenu();
            
            //DISPLAY_ShowMessage("hiiiii");
            mode = 0;
        }else if(mode == 2){
          DISPLAY_CHANGEPASSWORD();
          DELAY();
            DISPLAY_ClearScreen();
            DISPLAY_ShowMainMenu();
            
            //DISPLAY_ShowMessage("hiiiii");
            mode = 0;
        }else if(mode == 3){
          DISPLAY_CHANGETIMEOUT();
          DELAY();
            
          DISPLAY_ClearScreen();
            DISPLAY_ShowMainMenu();
            
            //DISPLAY_ShowMessage("hiiiii");
            mode = 0;
        }
    
            /* Debug: If needed, print raw or mapped value on second line */
            // LCD_SetCursor(1, 0);
            // LCD_WriteChar(key);

            /* Optional: Wait a bit to avoid flickering */
            SysCtlDelay(50);
        }
        
    }
}