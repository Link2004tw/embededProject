#include "password_manager.h"
#include "display_manager.h"
#include "input_manager.h"
#include "../HAL/eeprom/eeprom.h"
#include <string.h>

static char stored_password[PASSWORD_LENGTH + 1];
static uint8_t attempts;

static void GetPasswordFromUser(char *buffer, uint8_t length)
{
    uint8_t index = 0;
    memset(buffer, 0, length + 1);
    DISPLAY_EnablePasswordMode();

    while (index < length)
    {
        char key = 0;
        while ((key = InputManager_GetKey()) == 0); 

        if (key == '#') break; 
        if (key == 'D')       
        {
            if (index > 0) index--;
            continue;
        }
        if (key >= '0' && key <= '9')
        {
            buffer[index++] = key;
            DISPLAY_HandleKey(key);
        }
    }

    DISPLAY_DisablePasswordMode();
}

void PASSWORD_Init(void)
{
    attempts = 0;
    EEPROM_Read(0x00, (uint8_t*)stored_password, PASSWORD_LENGTH);
    stored_password[PASSWORD_LENGTH] = '\0';
}

void PASSWORD_SetNew(void)
{
    char new_pass[PASSWORD_LENGTH + 1];
    char confirm[PASSWORD_LENGTH + 1];

    
        //DISPLAY_AskEnterPassword();
        GetPasswordFromUser(new_pass, PASSWORD_LENGTH);

        DISPLAY_AskConfirmPassword();
        GetPasswordFromUser(confirm, PASSWORD_LENGTH);

        if (strcmp(new_pass, confirm) == 0)
        {
            strncpy(stored_password, new_pass, PASSWORD_LENGTH);
            stored_password[PASSWORD_LENGTH] = '\0';
            EEPROM_Write(0x00, (uint8_t*)stored_password, PASSWORD_LENGTH);
            //DISPLAY_ShowMessage("Password Saved!");
            break;
        }
        else
        {
            DISPLAY_PasswordMismatch();
        }
    
}

bool PASSWORD_Check(void)
{
    char entered[PASSWORD_LENGTH + 1];
    DISPLAY_AskPassword();
    GetPasswordFromUser(entered, PASSWORD_LENGTH);

    if (strcmp(entered, stored_password) == 0)
    {
        attempts = 0;
        DISPLAY_AccessGranted();
        return true;
    }
    else
    {
        attempts++;
        DISPLAY_WrongPassword(attempts);

        if (attempts >= MAX_ATTEMPTS)
        {
            DISPLAY_Lockout();
            attempts = 0;
        }

        return false;
    }
}

uint8_t PASSWORD_GetAttempts(void)
{
    return attempts;
}
