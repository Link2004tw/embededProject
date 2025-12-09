#include <stdint.h>
#include "uart.h"   // Gives us UART0_Init(), rxData, rxFlag

int main(void)
{
    uint8_t password[5];   // Buffer to store 5-digit password
    uint8_t count = 0;     // Tracks how many characters were received so far

    UART0_Init();          // Initialize UART + interrupts

    while(1)
    {
        // ---------------------------------------------------------
        // If UART interrupt handler received a byte:
        //    - rxFlag = 1
        //    - rxData contains the new byte
        // ---------------------------------------------------------
        if(rxFlag)
        {
            rxFlag = 0;    // Clear flag so we wait for next character

            // Store received byte in the password buffer
            password[count++] = rxData;

            // -----------------------------------------------------
            // If we collected 5 bytes → password is fully received
            // -----------------------------------------------------
            if(count == 5)
            {
                count = 0; // Reset for next password

                // ============================================
                //  PASSWORD FULLY RECEIVED (5 BYTES)
                //
                //  Add your password check logic here:
                //      - Compare with predefined password
                //      - Call door unlock function
                //      - Send UART message back
                //
                //  Example:
                //      if(memcmp(password, correctPassword, 5) == 0)
                //          unlockDoor();
                //      else
                //          denyAccess();
                // ============================================
            }
        }
    }
}
