#ifndef KEYPAD_H
#define KEYPAD_H
#include "../../Utils/dio.h"
#include "tm4c123gh6pm.h" // For direct register access
#include <stdint.h>
#include <stdbool.h>

/*
 * Keypad mapping array declaration.
 * Used to translate row/column indices to key values.
 */
extern const char keypad_codes[4][4];

/* Keypad dimensions */
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
#define KEYPAD_COL_PORT PORTC
#define KEYPAD_COL_PINS {PIN4, PIN5, PIN6, PIN7} // PC4-PC7

#define KEYPAD_ROW_PORT PORTA
#define KEYPAD_ROW_PINS {PIN2, PIN3, PIN4, PIN5} // PA2-PA5
/*
 * Initializes the keypad GPIO pins and configures interrupts for row pins.
 * Must be called before using Keypad_GetKey.
 */
void Keypad_Init(void);

/*
 * Returns the last key detected by the interrupt handler.
 * Returns 0 if no key is pressed.
 */
char Keypad_GetKey(void);

/*
 * Interrupt handler for keypad row pins.
 * Call this from the GPIO PortA ISR.
 */
void Keypad_RowISR(void);
void GPIOPortA_Handler(void);
#endif // KEYPAD_H