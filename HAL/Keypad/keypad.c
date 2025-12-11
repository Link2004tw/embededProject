/*****************************************************************************
 * File: keypad.c
 * Description: 4x4 Keypad Driver for TM4C123GH6PM
 ******************************************************************************/

#include "keypad.h"
#include "../../Utils/dio.h"

/*
 * Keypad mapping array.
 * Each element represents the character returned for a specific key press.
 * The array is organized as [row][column], matching the physical keypad layout.
 */
const char keypad_codes[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

/*
 * Pin configuration for keypad interface.
 * Columns are connected to PortC pins PC4-PC7 (outputs).
 * Rows are connected to PortA pins PA2-PA5 (inputs with pull-up).
 */



/*
 * Keypad_Init
 * Initializes the GPIO pins for keypad operation.
 * - Rows are set as inputs with internal pull-up resistors (PortA).
 * - Columns are set as outputs and driven HIGH (PortB).
 * This function must be called before using Keypad_GetKey.
 */
void Keypad_Init(void) {
    uint8_t row_pins[4] = KEYPAD_ROW_PINS;
    uint8_t col_pins[4] = KEYPAD_COL_PINS;
    // Configure rows (PortA) as input with pull-up
    for (uint8_t i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_ROW_PORT, row_pins[i], INPUT);
        DIO_SetPUR(KEYPAD_ROW_PORT, row_pins[i], ENABLE);
    }
    // Configure columns (PortB) as output and set HIGH
    for (uint8_t i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_COL_PORT, col_pins[i], OUTPUT);
        DIO_WritePin(KEYPAD_COL_PORT, col_pins[i], HIGH);
    }
}


/*
 * Keypad_GetKey
 * Scans the keypad and returns the character of the pressed key.
 * Returns 0 if no key is pressed.
 *
 * Scanning logic:
 *   1. Set each column LOW one at a time, others HIGH.
 *   2. Read all row inputs; if any row reads LOW, a key is pressed.
 *   3. Wait for key release (debounce).
 *   4. Return the mapped character from keypad_codes.
 */
//char Keypad_GetKey(void) {
 //   uint8_t row_pins[4] = KEYPAD_ROW_PINS;
 //   uint8_t col_pins[4] = KEYPAD_COL_PINS;
 //   for (uint8_t col = 0; col < 4; col++) {
 //       // Set all columns HIGH (inactive)
 //       for (uint8_t c = 0; c < 4; c++) {
 //           DIO_WritePin(KEYPAD_COL_PORT, col_pins[c], HIGH);
 //       }
 //       // Set current column LOW (active)
 //       DIO_WritePin(KEYPAD_COL_PORT, col_pins[col], LOW);
        // Small delay for signal to settle
 //       for (volatile int d = 0; d < 100; d++);
        // Scan rows for key press
 //       for (uint8_t row = 0; row < 4; row++) {
 //           uint8_t pin_val = DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]);
 //           if (pin_val == LOW) {
                // Key detected at (col, row)
                // Wait for key release (debounce)
  //              while (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW);
                // Return the mapped character from keypad_codes
  //              return keypad_codes[row][col];
//            }
//        }
//    }
//    return 0; // No key pressed
//}
char Keypad_GetKey(void) {
    uint8_t row_pins[4] = KEYPAD_ROW_PINS;
    uint8_t col_pins[4] = KEYPAD_COL_PINS;
    char pressed_key = 0; // Variable to hold the detected key

    for (uint8_t col = 0; col < 4; col++) {
        // Step 1: Drive one column LOW
        // Set all columns HIGH (inactive)
        for (uint8_t c = 0; c < 4; c++) {
            DIO_WritePin(KEYPAD_COL_PORT, col_pins[c], HIGH);
        }
        // Set current column LOW (active)
        DIO_WritePin(KEYPAD_COL_PORT, col_pins[col], LOW);

        // Small delay for signal to settle and pre-debounce
        for (volatile int d = 0; d < 100; d++); // ~100 loop delay (depends on clock)

        // Step 2: Scan rows for key press
        for (uint8_t row = 0; row < 4; row++) {
            uint8_t pin_val = DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]);

            if (pin_val == LOW) {
                // Key detected at (row, col) - CONFIRM PRESS
                pressed_key = keypad_codes[row][col];
                
                // *** CRITICAL CHANGE: Handle Key Release Blocking ***

                // Wait until the key is released (pin goes back to HIGH)
                // This is a blocking loop that ensures the same key is not 
                // re-registered until the finger is lifted.
                while (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW) {
                    // Optional: Add a small delay here to prevent unnecessary fast polling
                    for (volatile int d_rel = 0; d_rel < 50; d_rel++);
                }
                
                // Now that the key has been pressed AND released, 
                // we return the key value. The function will not find this 
                // key pressed again on the next call because the loop is done.
                return pressed_key; 
            }
        }
    }
    
    // Step 3: No key pressed, return 0
    return 0; 
}