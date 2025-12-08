#ifndef INPUT_MANAGER_H_
#define INPUT_MANAGER_H_

#include <stdint.h>

/*
 * Returns the mapped key after processing keypad output.
 * - Returns 0 if no key pressed
 * - Returns '+', '-', '*', digits, '#', 'D', etc.
 */
char InputManager_GetKey(void);

/*
 * Handles the key by:
 * 1. Getting mapped key
 * 2. Sending it to display manager
 * 3. (Optional) sending via UART
 */
void InputManager_HandleKey(void);

#endif /* INPUT_MANAGER_H_ */