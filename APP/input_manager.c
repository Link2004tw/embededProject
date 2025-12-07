#include "keypad.h"        
#include "display_manager.h"
// Add your UART include if needed
// #include "uart.h"

/*
 * Map keypad returned characters to project logic:
 * A -> +
 * B -> -
 * C -> *
 * digits, *, #, 0 remain unchanged
 */
static char MapKey(char k)
{
    switch (k)
    {
        case 'A': return '+';
        case 'B': return '-';
        case 'C': return '*';
        case 'D': return 'D';  // optional: treat as cancel or unused
        default:  return k;
    }
}

char InputManager_GetKey(void)
{
    char raw = Keypad_GetKey();    // directly from keypad.c
    if (raw == 0)                  
        return 0;                  // no key pressed

    return MapKey(raw);
}

/*
 * Main function that other modules call.
 * Handles what to show on screen immediately after a press.
 */
void InputManager_HandleKey(void)
{
    char key = InputManager_GetKey();
    if (key == 0)
        return;

    DISPLAY_HandleKey(key);

    // Uncomment this if you want to send to UART here:
    // UART_SendByte(key);
}
