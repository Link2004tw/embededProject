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
    LCD_WriteString("- Change * Time");
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


void DISPLAY_CHANGEPASSWORD(void){
    LCD_Clear();
    char password[5] = "";
    char savedPassword[5] = "1234";  // Load from storage
    char newPassword[5] = "";
    short pass_index = 0;
    
    
    LCD_WriteString("Enter old password");
    LCD_SetCursor(1, 0);
    
    for(pass_index = 0; pass_index < 4; pass_index++){
        char key = InputManager_GetKey();
        while(key == 0) {
            key = InputManager_GetKey();
            if(key=='=') return;
            
            SysCtlDelay(10000);
        }
        password[pass_index] = key;
        LCD_WriteChar('*');
    }
    password[4] = '\0';
    
   
    if(strcmp(password, savedPassword) != 0){
        LCD_Clear();
        LCD_WriteString("Incorrect password");
        SysCtlDelay(10000000);
        return;
    }
    
    
    LCD_Clear();
    LCD_WriteString("Enter new password");
    LCD_SetCursor(1, 0);
    
    for(pass_index = 0; pass_index < 4; pass_index++){
        char key = InputManager_GetKey();
        while(key == 0) {
            key = InputManager_GetKey();
            if(key=='=') return;
            SysCtlDelay(10000);
        }
        newPassword[pass_index] = key;
        LCD_WriteChar('*');
    }
    newPassword[4] = '\0';
    
    
    LCD_Clear();
    LCD_WriteString("Confirm new pass");
    LCD_SetCursor(1, 0);
    
    char confirmPassword[5] = "";
    for(pass_index = 0; pass_index < 4; pass_index++){
        char key = InputManager_GetKey();
        while(key == 0) {
            key = InputManager_GetKey();
            if(key=='=') return;
            
            SysCtlDelay(10000);
        }
        confirmPassword[pass_index] = key;
        LCD_WriteChar('*');
    }
    confirmPassword[4] = '\0';
    
    
    if(strcmp(newPassword, confirmPassword) != 0){
        LCD_Clear();
        LCD_WriteString("Passwords don't match");
        SysCtlDelay(10000000);
        return;
    }
    
    
    LCD_Clear();
    LCD_WriteString("Password Updated!");
    SysCtlDelay(10000000);
}

void DISPLAY_CHANGETIMEOUT(void){
   /*LCD_Clear();
   LCD_WriteString("Timeout =")
   LCD_SetCursor(1, 0);
   uint8_t value = PotentiometerManager_GetTimeout();
   char buffer[3];
   sprintf(buffer, "%u",value);
   LCD_WriteString(buffer)*/
   PotentiometerManager_HandleTimeoutConfig();
}

/*****************************************************************************
 * Function: DISPLAY_OLD_PASSWORD
 * 
 * Description:
 *   Prompts user to enter their old/current password for verification.
 *   Displays asterisks on LCD for each character entered.
 *   Compares with saved password.
 * 
 * Parameters: None
 * 
 * Returns:
 *   true  - Password is correct
 *   false - Password is incorrect
 *****************************************************************************/
void DISPLAY_OLD_PASSWORD(void)
{
    char password[5] = "";
    char savedPassword[5] = "1234";  // Load from storage (replace with actual storage read)
    short pass_index = 0;
    char key = 0;
    
    LCD_Clear();
    LCD_WriteString("Enter old password");
    LCD_SetCursor(1, 0);
    
    for(pass_index = 0; pass_index < 4; pass_index++){
        key = InputManager_GetKey();
        while(key == 0) {  
            key = InputManager_GetKey();
            if(key=='=') return;
            
            SysCtlDelay(10000);
        }
        password[pass_index] = key;
        LCD_WriteChar('*');
    }
    password[4] = '\0';
    //send the password;
    if(strcmp(password, savedPassword) != 0){
        LCD_Clear();
        LCD_WriteString("Incorrect password");
        SysCtlDelay(10000000);
        //return false;
    }else {
      LCD_Clear();
        LCD_WriteString("Correct password");
        SysCtlDelay(10000000);
    }
    
    
    //return true;
}

/*****************************************************************************
 * Function: DISPLAY_NEW_PASSWORD
 * 
 * Description:
 *   Prompts user to enter a new password twice for confirmation.
 *   Verifies both entries match.
 *   Displays asterisks on LCD for each character entered.
 * 
 * Parameters: None
 * 
 * Returns:
 *   true  - New password successfully entered and confirmed
 *   false - Passwords don't match
 *****************************************************************************/
void DISPLAY_NEW_PASSWORD(void)
{
    char newPassword[5] = "";
    char confirmPassword[5] = "";
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
    newPassword[4] = '\0';
    
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
short haspass = true;//dummy for the check that it is the first password will be take from the back
 void DISPLAY_OPEN(void)
{
    /* Verify old password first */
    if(haspass){
        DISPLAY_OLD_PASSWORD();
    }else{
        DISPLAY_NEW_PASSWORD();
    }
    
    /* Get and confirm new password */
}
// void DISPLAY_OPEN(void){
//     LCD_Clear();
//     char password[5] = "";
//     char savedPassword[5] = "1234";  // Load from storage (replace with actual storage read)
//     short pass_index = 0;
    
//     LCD_WriteString("Enter old password");
//     LCD_SetCursor(1, 0);
    
//     for(pass_index = 0; pass_index < 4; pass_index++){
//         char key = InputManager_GetKey();
//         while(key == 0) {  
//             key = InputManager_GetKey();
//             SysCtlDelay(10000);
//         }
//         password[pass_index] = key;
//         LCD_WriteChar('*');
//     }
//     password[4] = '\0';
    
    
//     if(strcmp(password, savedPassword) != 0){
//         LCD_Clear();
//         LCD_WriteString("Incorrect password");
//         SysCtlDelay(10000000);
//         return;
//     }
    
    
//     LCD_Clear();
//     LCD_WriteString("Enter new pass");
//     LCD_SetCursor(1, 0);
    
//     for(pass_index = 0; pass_index < 4; pass_index++){
//         char key = InputManager_GetKey();
//         while(key == 0) {
//             key = InputManager_GetKey();
//             SysCtlDelay(10000);
//         }
//         password[pass_index] = key;
//         LCD_WriteChar('*');
//     }
//     password[4] = '\0';
    
//     LCD_Clear();
//     LCD_WriteString("Password Changed");
//     SysCtlDelay(10000000);  
// }

void DISPLAY_ERROR(void){
    LCD_Clear();
    LCD_WriteString("Invalid input");
}