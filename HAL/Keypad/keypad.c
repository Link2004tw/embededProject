/*****************************************************************************
 * File: keypad.c
 * Description: 4x4 Keypad Driver for TM4C123GH6PM
 ******************************************************************************/

#include "keypad.h"
#include "../../Utils/dio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

/*
 * Keypad mapping array.
 */
const char keypad_codes[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

volatile char g_LastKeyPressed = 0;

/*
 * Keypad_Init
 * Initializes GPIOs and Timer0 for periodic scanning (20ms).
 */
void Keypad_Init(void) {
    uint8_t row_pins[4] = KEYPAD_ROW_PINS;
    uint8_t col_pins[4] = KEYPAD_COL_PINS;

    // --- GPIO Initialization ---
    // Configure rows (PortA) as input with pull-up
    for (uint8_t i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_ROW_PORT, row_pins[i], INPUT);
        DIO_SetPUR(KEYPAD_ROW_PORT, row_pins[i], ENABLE);
    }
    // Configure columns (PortB/C) as output and set HIGH
    for (uint8_t i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_COL_PORT, col_pins[i], OUTPUT);
        DIO_WritePin(KEYPAD_COL_PORT, col_pins[i], HIGH);
    }

    // --- Timer Initialization (Timer0A) ---
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    
    // Set Period to 20ms (50Hz)
    // Formula: LoadValue = (ClockFreq / TargetFreq) - 1
    // Assuming 16MHz clock: 16,000,000 / 50 = 320,000
    uint32_t period = SysCtlClockGet() / 50; 
    TimerLoadSet(TIMER0_BASE, TIMER_A, period - 1);

    // Enable Timer Interrupts
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();

    TimerEnable(TIMER0_BASE, TIMER_A);
}

/*
 * Internal helper to perform a single scan of the matrix.
 * Returns the character if pressed, or 0 if nothing.
 */
static char Keypad_Scan(void) {
    uint8_t row_pins[4] = KEYPAD_ROW_PINS;
    uint8_t col_pins[4] = KEYPAD_COL_PINS;

    for (uint8_t col = 0; col < 4; col++) {
        // Set all columns HIGH
        for (uint8_t c = 0; c < 4; c++) {
            DIO_WritePin(KEYPAD_COL_PORT, col_pins[c], HIGH);
        }
        // Set current column LOW
        DIO_WritePin(KEYPAD_COL_PORT, col_pins[col], LOW);

        // Small delay
        for (volatile int d = 0; d < 20; d++); 

        // Check rows
        for (uint8_t row = 0; row < 4; row++) {
            if (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW) {
                return keypad_codes[row][col];
            }
        }
    }
    return 0;
}

/*
 * Timer0A_Handler
 * Runs every 20ms. Handles debouncing and key registration.
 */
void Timer0A_Handler(void) {
    static char last_scan_state = 0;       // State from previous ISR call
    static uint8_t stable_count = 0;       // How many times we've seen the same key
    static bool key_registered = false;    // Have we already reported this press?

    // Clear Interrupt Flag
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    char current_scan = Keypad_Scan();

    if (current_scan == last_scan_state) {
        // Signal is stable
        if (current_scan != 0) {
            stable_count++;
            // If stable for > 2 checks (40ms) and not yet registered
            if (stable_count > 2 && !key_registered) {
                g_LastKeyPressed = current_scan; // Push to "buffer"
                key_registered = true;           // Mark as handled
            }
        }
    } else {
        // State changed (bounce or release)
        stable_count = 0;
        if (current_scan == 0) {
            key_registered = false; // Reset when key is released
        }
    }

    last_scan_state = current_scan;
}

/*
 * Keypad_GetKey
 * Returns the key from the "buffer" (g_LastKeyPressed).
 * Clears the buffer after reading.
 */
char Keypad_GetKey(void) {
    char key = g_LastKeyPressed;
    if (key != 0) {
        g_LastKeyPressed = 0; // Consume the key
    }
    return key;
}