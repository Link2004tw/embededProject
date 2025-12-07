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