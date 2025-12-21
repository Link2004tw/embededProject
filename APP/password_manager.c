#include "password_manager.h"
#include "../HAL/password/password.h"
#include "../APP/display_manager.h"
#include "../APP/input_manager.h"
#include "../HAL/BUZZER/buzzer.h"

/******************************************************************************
 *                          STATIC VARIABLES                                   *
 ******************************************************************************/

static uint8_t g_failedAttempts = 0U;

/******************************************************************************
 *                          FUNCTION DEFINITIONS                                *
 ******************************************************************************/

uint8_t PasswordMgr_Verify(void)
{
    uint8_t enteredPassword[PASSWORD_LENGTH];

    //Display_ShowMessage("Enter Password");
    //Input_GetPassword(enteredPassword);

    if (Password_Compare(enteredPassword) == PASSWORD_OK)
    {
        PasswordMgr_ResetAttempts();
        return 1U; /* Correct */
    }

    g_failedAttempts++;

    if (g_failedAttempts >= MAX_PASSWORD_ATTEMPTS)
    {
        //Display_ShowMessage("LOCKED!");
        buzzerON();
        PasswordMgr_ResetAttempts();
        return 0U;
    }

    //Display_ShowMessage("Wrong Password");
    return 0U;
}

uint8_t PasswordMgr_Change(void)
{
    uint8_t oldPass[PASSWORD_LENGTH];
    uint8_t newPass[PASSWORD_LENGTH];

    //Display_ShowMessage("Old Password");
    //Input_GetPassword(oldPass);

    if (Password_Compare(oldPass) != PASSWORD_OK)
    {
        g_failedAttempts++;

        if (g_failedAttempts >= MAX_PASSWORD_ATTEMPTS)
        {
            buzzerON();
            PasswordMgr_ResetAttempts();
        }
        else
        {
            /* User feedback: notify wrong password entered */
            buzzerON();
        }

        return 0U;
    }

    //Display_ShowMessage("New Password");
    //Input_GetPassword(newPass);

    Password_Save(newPass);
    PasswordMgr_ResetAttempts();

    //Display_ShowMessage("Password Saved");
    return 1U;
}

void PasswordMgr_ResetAttempts(void)
{
    g_failedAttempts = 0U;
}
