#include <stdbool.h>
#include "display_manager.h"
#include "lcd.h"

static uint8_t password_mode = 0;

void DISPLAY_Init(void)
{
    LCD_Init();
}

void DISPLAY_HandleKey(char key)
{
    if (password_mode)
        LCD_WriteChar('*');
    else
        LCD_WriteChar(key);
}

void DISPLAY_EnablePasswordMode(void)
{
    password_mode = 1;
}

void DISPLAY_DisablePasswordMode(void)
{
    password_mode = 0;
}

void DISPLAY_ShowMainMenu(void)
{
    LCD_Clear();
    LCD_WriteString("+ Open");
    LCD_SetCursor(1, 0);
    LCD_WriteString("- Change   * Timeout");
}

void DISPLAY_ShowMessage(const char *msg)
{
    LCD_Clear();
    LCD_WriteString(msg);
}

void DISPLAY_ShowTimeoutValue(uint8_t value)
{
    LCD_Clear();
    LCD_WriteString("Timeout: ");

    LCD_WriteChar((value / 10) + '0');
    LCD_WriteChar((value % 10) + '0');

    LCD_WriteString(" sec");
}

void DISPLAY_ClearScreen(void)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
}
//short haspass = true;//dummy for the check that it is the first password will be take from the back
 void DISPLAY_OPEN(void)
{
    /* Verify old password first */
    //if(haspass){
        DISPLAY_OLD_PASSWORD();
    //}//else{
      //  DISPLAY_NEW_PASSWORD();
    //}
    
    /* Get and confirm new password */
}
void DISPLAY_NEW_PASSWORD(void)
{
    char newPassword[7] = "";
    char confirmPassword[7] = "";
    short pass_index = 0;
    char key = 0;
    /* Get new password */
    //while(1){
    LCD_Clear();
    LCD_WriteString("Enter new pass");
    LCD_SetCursor(1, 0);
    for(pass_index = 0; pass_index < 4; pass_index++){
        key = InputManager_GetKey();
        while(key == 0) {  
            key = InputManager_GetKey();
            if(key=='=') return;
            SysCtlDelay(10000);
        }
        newPassword[pass_index] = key;
        LCD_WriteChar('*');
    }
    
        
    //}
    newPassword[6] = '\0';
    UART5_SendString(newPassword);
    
    /* Get confirmation password */
    LCD_Clear();
    LCD_WriteString("Confirm new pass");
    LCD_SetCursor(1, 0);
    
    for(pass_index = 0; pass_index < 4; pass_index++){
        key = InputManager_GetKey();
        while(key == 0) {
            key = InputManager_GetKey();
            SysCtlDelay(10000);
        }
        confirmPassword[pass_index] = key;
        LCD_WriteChar('*');
    }
    confirmPassword[4] = '\0';
    
    /* Verify passwords match */
    if(strcmp(newPassword, confirmPassword) != 0){
        LCD_Clear();
        LCD_WriteString("Passwords don't");
        LCD_SetCursor(1, 0);
        LCD_WriteString("match");
        SysCtlDelay(10000000);
        LCD_Clear();
        //return false;
    }else{
        return;
    }

    
    
    /* Success message */
    LCD_Clear();
    LCD_WriteString("Password Changed");
    return;
    //SysCtlDelay(10000000);
    //return true;
}

/*****************************************************************************
 * Function: DISPLAY_OPEN (refactored)
 * 
 * Description:
 *   Handles the door open flow by verifying old password,
 *   then allowing user to set a new password.
 * 
 * Parameters: None
 * Returns: None
 *****************************************************************************/
//short haspass = true;//dummy for the check that it is the first password will be take from the back
 void DISPLAY_OPEN(void)
{
    /* Verify old password first */
    //if(haspass){
        DISPLAY_OLD_PASSWORD();
    //}//else{
      //  DISPLAY_NEW_PASSWORD();
    //}
    
    /* Get and confirm new password */
}

void DISPLAY_ERROR(void){
    LCD_Clear();
    LCD_WriteString("Invalid input");
    SysCtlDelay(10000000);
    
}
