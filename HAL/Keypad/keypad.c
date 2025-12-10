#include "keypad.h"
#include "dio.h"
#include "tm4c123gh6pm.h" // For direct register access

const char keypad_codes[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

#define KEYPAD_COL_PORT PORTC
#define KEYPAD_COL_PINS {PIN4, PIN5, PIN6, PIN7} // PC4-PC7

#define KEYPAD_ROW_PORT PORTA
#define KEYPAD_ROW_PINS {PIN2, PIN3, PIN4, PIN5} // PA2-PA5

static volatile char last_key_pressed = 0;

void Keypad_Init(void) {
    uint8_t row_pins[4] = KEYPAD_ROW_PINS;
    uint8_t col_pins[4] = KEYPAD_COL_PINS;

    // Configure rows (PortA) as input with pull-up and enable interrupt
    for (uint8_t i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_ROW_PORT, row_pins[i], INPUT);
        DIO_SetPUR(KEYPAD_ROW_PORT, row_pins[i], ENABLE);
        // Enable interrupt for falling edge (key press)
        GPIO_PORTA_IS_R &= ~(1 << row_pins[i]);   // Edge-sensitive
        GPIO_PORTA_IBE_R &= ~(1 << row_pins[i]);  // Single edge
        GPIO_PORTA_IEV_R &= ~(1 << row_pins[i]);  // Falling edge
        GPIO_PORTA_ICR_R = (1 << row_pins[i]);    // Clear any prior interrupt
        GPIO_PORTA_IM_R |= (1 << row_pins[i]);    // Unmask interrupt
    }
    NVIC_EN0_R |= (1 << 0); // Enable IRQ for PortA (IRQ#0)

    // Configure columns (PortC) as output and set HIGH
    for (uint8_t i = 0; i < 4; i++) {
        DIO_Init(KEYPAD_COL_PORT, col_pins[i], OUTPUT);
        DIO_WritePin(KEYPAD_COL_PORT, col_pins[i], HIGH);
    }
}

char Keypad_GetKey(void) {
    char key = last_key_pressed;
    last_key_pressed = 0; // Clear after read
    return key;
}

// This should be called from the GPIO PortA interrupt handler
void Keypad_RowISR(void) {
    uint8_t row_pins[4] = KEYPAD_ROW_PINS;
    uint8_t col_pins[4] = KEYPAD_COL_PINS;

    // Find which row triggered the interrupt
    uint32_t status = GPIO_PORTA_MIS_R;
    for (uint8_t row = 0; row < 4; row++) {
        if (status & (1 << row_pins[row])) {
            // Scan columns to find which key is pressed
            for (uint8_t col = 0; col < 4; col++) {
                // Set all columns HIGH
                for (uint8_t c = 0; c < 4; c++) {
                    DIO_WritePin(KEYPAD_COL_PORT, col_pins[c], HIGH);
                }
                // Set current column LOW
                DIO_WritePin(KEYPAD_COL_PORT, col_pins[col], LOW);
                for (volatile int d = 0; d < 100; d++); // Small delay
                if (DIO_ReadPin(KEYPAD_ROW_PORT, row_pins[row]) == LOW) {
                    last_key_pressed = keypad_codes[row][col];
                    break;
                }
            }
            GPIO_PORTA_ICR_R = (1 << row_pins[row]); // Clear interrupt
        }
    }
}

// In your main interrupt vector table, call Keypad_RowISR() from PortA ISR
// Example:
// void GPIOPortA_Handler(void) {
//     Keypad_RowISR();
// }