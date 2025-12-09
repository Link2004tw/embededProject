#include "feedback_manager.h"
#include "lcd.h"


void DISPLAY_AskEnterPassword(void)
{
    LCD_Clear();
    LCD_WriteString("Enter Password:");
    LCD_SetCursor(1, 0);
}

void DISPLAY_AskConfirmPassword(void)
{
    LCD_Clear();
    LCD_WriteString("Confirm Pass:");
    LCD_SetCursor(1, 0);
}

void DISPLAY_AskPassword(void)
{
    LCD_Clear();
    LCD_WriteString("Enter Password:");
    LCD_SetCursor(1, 0);
}

void DISPLAY_PasswordMismatch(void)
{
    LCD_Clear();
    LCD_WriteString("Mismatch!");
}

void DISPLAY_WrongPassword(uint8_t attempt)
{
    LCD_Clear();
    LCD_WriteString("Wrong Password");
    LCD_SetCursor(1, 0);
    LCD_WriteString("Attempt ");
    LCD_WriteChar(attempt + '0');
    LCD_WriteString("/3");
}

void DISPLAY_Lockout(void)
{
    LCD_Clear();
    LCD_WriteString("LOCKOUT ACTIVE");
}

void DISPLAY_AccessGranted(void)
{
    LCD_Clear();
    LCD_WriteString("Access Granted");
}

void DISPLAY_AccessDenied(void)
{
    LCD_Clear();
    LCD_WriteString("Access Denied");
}

void DISPLAY_DoorOpening(void)
{
    LCD_Clear();
    LCD_WriteString("Door Opening...");
}

void DISPLAY_DoorOpenWait(uint8_t sec)
{
    LCD_Clear();
    LCD_WriteString("Open Wait:");
    LCD_SetCursor(1, 0);
    LCD_WriteChar((sec / 10) + '0');
    LCD_WriteChar((sec % 10) + '0');
    LCD_WriteString(" sec");
}

void DISPLAY_DoorClosing(void)
{
    LCD_Clear();
    LCD_WriteString("Door Closing...");
}

void DISPLAY_ShowTimeoutMenu(void)
{
    LCD_Clear();
    LCD_WriteString("Adjust Timeout");
    LCD_SetCursor(1, 0);
    LCD_WriteString("(5-30 sec)");
}

void DISPLAY_SaveTimeoutPrompt(void)
{
    LCD_Clear();
    LCD_WriteString("Save Timeout?");
    LCD_SetCursor(1, 0);
    LCD_WriteString("Enter Password");
}

void DISPLAY_TimeoutSaved(void)
{
    LCD_Clear();
    LCD_WriteString("Timeout Saved!");
}

void DISPLAY_Processing(void)
{
    LCD_Clear();
    LCD_WriteString("Processing...");
}

void DISPLAY_Error(const char *msg)
{
    LCD_Clear();
    LCD_WriteString("Error:");
    LCD_SetCursor(1, 0);
    LCD_WriteString(msg);
}
