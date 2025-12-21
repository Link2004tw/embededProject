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
    LCD_WriteString("- Change  * Time");
}
void DISPLAY_CHANGEPASSWORD(void){
    LCD_Clear();
    //0: Mode
    //1: seperator
    //2->6: password
    //7: seperator
    //8->12 new password
    //13:#
    //14: terminating 
    char message[15]="";
    message[0]= ADDMODE;
    message[1]=',';
    //char password[5] = "";
    //char savedPassword[5] = "1234";  // Load from storage
    char newPassword[5] = "";
    short pass_index = 0;
    
     //1 , 1  2 3 4 5 , 1 2 3 4 5 #
    //1 2  3  4 5 6 7 8 9 0 1 2 3 4

    LCD_WriteString("Old password:");
    LCD_SetCursor(1, 0);
    //2 3 4 5 6
    for(pass_index = 2; pass_index < 7; pass_index++){
        char key = InputManager_GetKey();
        while(key == 0) {
            key = InputManager_GetKey();
            if(key=='=') return;
            
            SysCtlDelay(10000);
        }
        message[pass_index] = key;
        LCD_WriteChar('*');
    }
    message[7] = ',';
    
    
    LCD_Clear();
    LCD_WriteString("New password:");
    LCD_SetCursor(1, 0);
    
    for(pass_index = 8; pass_index < 13; pass_index++){
        char key = InputManager_GetKey();
        while(key == 0) {
            key = InputManager_GetKey();
            if(key=='=') return;
            SysCtlDelay(10000);
        }
        message[pass_index] = key;
        newPassword[pass_index-8] =key;
        LCD_WriteChar('*');
    }
    message[13] = '#';
    newPassword[4]='\0';
    
    LCD_Clear();
    LCD_WriteString("Confirm new pass");
    LCD_SetCursor(1, 0);
    
    char confirmPassword[5] = "";
    while(1){
        LCD_Clear();
        LCD_WriteString("Confirm new pass");
        LCD_SetCursor(1, 0);
        
        for(pass_index = 0; pass_index < 5; pass_index++){
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
            LCD_WriteString("They don't match");
            SysCtlDelay(10000000);
            continue;
        }else { break;}
    }                  
    LCD_Clear();
    //LCD_WriteString("Congrats");
    //LCD_WriteString("Password Updated!");
    
    // TEST LED: Green flash before sending
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    SysCtlDelay(2000000);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    
    UART5_SendString(message);
    char ack_buffer[20];
    LCD_Clear();
    DISPLAY_ShowMessage("Waiting for Ack...");
    
    // TEST LED: Blue flash while waiting
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
    SysCtlDelay(1000000);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
    
    //UART5_ReceiveString(ack_buffer, 20);
    UART5_ReceiveStringWithTimeout(ack_buffer, 20);
    LCD_Clear();
    if(ack_buffer[0] != '\0'){

        // TEST LED: Red flash on successful receive
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        SysCtlDelay(2000000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        
        SHOW_BUFFER(ack_buffer);   
        SysCtlDelay(16000000);  

    }else {
        ack_buffer[0]='\0';
        LCD_Clear();
        UART5_ReceiveStringWithTimeout(ack_buffer, 20);
        //UART5_ReceiveString(ack_buffer, 20);

        // TEST LED: Long red on timeout
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        SysCtlDelay(8000000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        
        DISPLAY_ShowMessage(ack_buffer);
    }
    SysCtlDelay(10000000);
    
    //LCD_Clear();
    //SysCtlDelay(10000000);
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
    char password[9];
    memset(password, 0, sizeof(password));
    password[0]=PASSMODE;
    password[1]=',';
    //char savedPassword[5] = "1234";  // Load from storage (replace with actual storage read)
    short pass_index = 2;
    char key = 0;
    //1 , 1 2 3 4 5 #
    //0 1 2 3 4 5 6 7
    LCD_Clear();
    LCD_WriteString("Enter password");
    LCD_SetCursor(1, 0);
    
    for(pass_index = 2; pass_index < 7; pass_index++){
        key = InputManager_GetKey();
        while(key == 0) {  
            key = InputManager_GetKey();
            if(key=='=') return;
            
            SysCtlDelay(10000);
        }
        password[pass_index] = key;
        LCD_WriteChar(key);
    }
    password[7] ='#';
    password[8] = '\0';
    
    
    // TEST LED: Green flash before sending
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    SysCtlDelay(20000000);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    
    UART5_SendString(password);
    SysCtlDelay(10000000);
    char ack_buffer[20];
    LCD_Clear();
    DISPLAY_ShowMessage("Waiting for Ack...");
    
    // TEST LED: Blue flash while waiting
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
    //SysCtlDelay(1000000);
    UART5_ReceiveStringWithTimeout(ack_buffer, 20);
   // UART5_ReceiveString(ack_buffer, 20);
    SysCtlDelay(SysCtlClockGet());  // 100ms
    
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

   // LCD_Clear();
    if(ack_buffer[0] != '\0'){
        // TEST LED: Red flash on successful receive
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        SysCtlDelay(2000000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        
        SHOW_BUFFER(ack_buffer);   
        SysCtlDelay(16000000);  

    }else {
        // TEST LED: Long red on timeout
        //LCD_Clear();
        // TEST LED: Long red on timeout
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        SysCtlDelay(8000000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        
        DISPLAY_ShowMessage("No response");
    }
    //SysCtlDelay(16000000);  
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

//void DISPLAY_NEW_PASSWORD(void)
//{
//    char newPassword[7] = "";
//    char confirmPassword[7] = "";
//    short pass_index = 0;
//    char key = 0;
//    /* Get new password */
//    //while(1){
//    LCD_Clear();
//    LCD_WriteString("Enter new pass");
//    LCD_SetCursor(1, 0);
//    for(pass_index = 0; pass_index < 4; pass_index++){
//        key = InputManager_GetKey();
//        while(key == 0) {  
//            key = InputManager_GetKey();
//            if(key=='=') return;
//            SysCtlDelay(10000);
//        }
//        newPassword[pass_index] = key;
//        LCD_WriteChar('*');
//    }
//    
//        
//    //}
//    newPassword[6] = '\0';
//    UART5_SendString(newPassword);
//    
//    /* Get confirmation password */
//    LCD_Clear();
//    LCD_WriteString("Confirm new pass");
//    LCD_SetCursor(1, 0);
//    
//    for(pass_index = 0; pass_index < 4; pass_index++){
//        key = InputManager_GetKey();
//        while(key == 0) {
//            key = InputManager_GetKey();
//            SysCtlDelay(10000);
//        }
//        confirmPassword[pass_index] = key;
//        LCD_WriteChar('*');
//    }
//    confirmPassword[4] = '\0';
//    
//    /* Verify passwords match */
//    if(strcmp(newPassword, confirmPassword) != 0){
//        LCD_Clear();
//        LCD_WriteString("Passwords don't");
//        LCD_SetCursor(1, 0);
//        LCD_WriteString("match");
//        SysCtlDelay(10000000);
//        LCD_Clear();
//        //return false;
//    }else{
//        return;
//    }
//
//    
//    
//    /* Success message */
//    LCD_Clear();
//    LCD_WriteString("Password Changed");
//    return;
//    //SysCtlDelay(10000000);
//    //return true;
//}

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
// void DISPLAY_OPEN(void)
//{
//    /* Verify old password first */
//    //if(haspass){
//        DISPLAY_OLD_PASSWORD();
//    //}//else{
//      //  DISPLAY_NEW_PASSWORD();
//    //}
//    
//    /* Get and confirm new password */
//}

//void DISPLAY_ERROR(void){
//    LCD_Clear();
//    LCD_WriteString("Invalid input");
//    SysCtlDelay(10000000);
//    
//}

void SHOW_BUFFER(char* buffer){
    LCD_Clear();
    char* dollar_pos = strchr(buffer, '$');
    
    if(dollar_pos != NULL) {
        // Find length of first part (before $)
        int first_len = dollar_pos - buffer;
        
        // Write first part
        for(int i = 0; i < first_len; i++) {
            LCD_WriteChar(buffer[i]);
        }
        
        // Set cursor to second line
        LCD_SetCursor(1, 0);
        
        // Write second part (skip the $)
        LCD_WriteString(dollar_pos + 1);
    } else {
        // No $ found, just write the whole buffer
        LCD_WriteString(buffer);
    }
}