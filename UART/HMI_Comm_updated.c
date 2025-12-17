#include "HMI_Comm.h"

#define PASSWORD                 "12345"        // will be changed to eeprom later
#define EEPROM_MAGIC_NUMBER      0xA5A5A5A5
#define EEPROM_ADDR_MAGIC        0x0000         // Address 0: Magic Number
#define EEPROM_ADDR_TIMEOUT      0x0004         // Address 4: Timeout Value
#define EEPROM_ADDR_PASS         0x0010         // Address 16: Password (start)
#define PASSWORD_MAX_LEN         8              // Max password length (must be multiple of 4 for Tiva EEPROM)

char rxBuffer[RX_BUFFER_SIZE]="";
uint8_t rxIndex = 0;

char currentPassword[PASSWORD_MAX_LEN];
uint32_t currentTimeout = 30;                   // Default timeout (seconds)

short failedAttempts = 0;

// --- HELPER FUNCTION: SAVE TO EEPROM ---
// Tiva C EEPROMProgram takes a pointer to uint32_t, so we cast our data
void Save_Settings(void) {
    uint32_t magic = EEPROM_MAGIC_NUMBER;

    // Save Magic Number
    EEPROMProgram(&magic, EEPROM_ADDR_MAGIC, sizeof(magic));

    // Save Timeout
    EEPROMProgram(&currentTimeout, EEPROM_ADDR_TIMEOUT, sizeof(currentTimeout));

    // Save Password (must write multiples of 4 bytes)
    // We cast char* to uint32_t* for the Tiva API
    EEPROMProgram((uint32_t *)currentPassword, EEPROM_ADDR_PASS, PASSWORD_MAX_LEN);
}

void UART1_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);            // Enable EEPROM Module

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART1));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0));     // Wait for EEPROM

    // Configure LED Pins as Outputs
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // EEPROM System Init
    uint32_t eepromStatus = EEPROMInit();
    if(eepromStatus != EEPROM_INIT_OK) {
        // Handle error if EEPROM fails (optional: turn on Red LED forever)
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
    }

    // CHECK FOR FIRST TIME RUN
    uint32_t checkMagic;
    EEPROMRead(&checkMagic, EEPROM_ADDR_MAGIC, sizeof(checkMagic));

    if (checkMagic != EEPROM_MAGIC_NUMBER) {
        // FIRST TIME: Write Defaults
        strcpy(currentPassword, "12345");
        currentTimeout = 30;
        Save_Settings(); // Write these defaults to EEPROM
    }
    else {
        // SUBSEQUENT TIMES: Read from EEPROM
        EEPROMRead(&currentTimeout, EEPROM_ADDR_TIMEOUT, sizeof(currentTimeout));
        EEPROMRead((uint32_t *)currentPassword, EEPROM_ADDR_PASS, PASSWORD_MAX_LEN);
    }

    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    
    UARTEnable(UART1_BASE);

    // Flash Green to indicate startup success
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    SysCtlDelay(8000000); // Small delay to see the light
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    
    while (UARTCharsAvail(UART1_BASE)) {
        UARTCharGet(UART1_BASE);
    }
}

void BUZZ(void) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);     // Red ON
    SysCtlDelay(16000000);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
}

void UART1_SendString(char* str) {
    // Turn ON Green LED until the whole message is sent
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    
    while(*str) {
        while(UARTBusy(UART1_BASE));      // Wait until TX ready
        UARTCharPut(UART1_BASE, *str);
        str++;
    }
    while(UARTBusy(UART1_BASE));          // Wait for last char to finish
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
}

void WAIT_FOR_MESSAGE(void) {
    if (UARTCharsAvail(UART1_BASE)) {
        char receivedChar = UARTCharGet(UART1_BASE);

        if (receivedChar == '#') {    // End of message marker
            rxBuffer[rxIndex] = '\0';  // Null-terminate the string
            rxIndex = 0;               // Prepare for next message

            // Expected formats:
            //   0,12345#           →    normal unlock with 12345 (mode 0: user enters password to open door)
            //   1,12345,67890#     →    change password to 67890 (mode 1: user wants to change password)
            //   2,5->30#           →    change timeout           (mode 2: user wants to change timeout)

            char *modeStr;
            char *pass1Str;
            char *pass2Str;

            modeStr = strtok(rxBuffer, ",");
            pass1Str = strtok(NULL, ",");
            pass2Str = strtok(NULL, ",");  // May be NULL used only when changing password

            if (modeStr == NULL || pass1Str == NULL) {
                // Invalid format
                memset(rxBuffer, 0, sizeof(rxBuffer));
                return;
            }

            // ------------------------------------------------------------------
            // Mode 0: Normal unlock with user password
            // ------------------------------------------------------------------
            if (modeStr[0] == '0' && modeStr[1] == '\0') {
                // Compare received pass against the Global variable (loaded from EEPROM)
                if (strcmp(pass1Str, currentPassword) == 0) {                  // Use stored password [PASSWORD]
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);     // Green ON
                    SysCtlDelay(16000000);                                     // ~500ms at 16MHz
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
                    failedAttempts = 0;
                }
                else {
                    failedAttempts++;
                    UART1_SendString("Wrong$Password#");
                    if (failedAttempts >= 3) {
                        BUZZ();  // Alarm for 3 wrong attempts
                        failedAttempts = 0;
                    }
                }
            }
            // ------------------------------------------------------------------
            // Mode 1: Change Password
            // ------------------------------------------------------------------
            else if (modeStr[0] == '1' && modeStr[1] == '\0') {
                if (strcmp(pass1Str, currentPassword) == 0) {                 // Check if old password is correct or not
                    if (pass2Str != NULL && pass2Str[0] != '\0') {            // Check the new password not equal NULL
                        // 1. Update RAM Variable
                        memset(currentPassword, 0, PASSWORD_MAX_LEN);         // Clear old
                        strncpy(currentPassword, pass2Str, PASSWORD_MAX_LEN - 1);

                        // 2. Write New Password to EEPROM
                        EEPROMProgram((uint32_t *)currentPassword, EEPROM_ADDR_PASS, PASSWORD_MAX_LEN);

                        UART1_SendString("Password$Changed#");
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Green
                        SysCtlDelay(16000000);
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
                    }
                    else {
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); // Blue LED?
                        SysCtlDelay(8000000);
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
                    }
                    failedAttempts = 0;
                }
                else {
                    failedAttempts++;
                    UART1_SendString("Wrong$Password#");
                    
                    if (failedAttempts >= 3) {                                 // 3 incorrect entered values of password
                        BUZZ();
                        failedAttempts = 0;
                    }
                }
            }
            // ------------------------------------------------------------------
            // Mode 2: Change Timeout
            // ------------------------------------------------------------------
            else if (modeStr[0] == '2' && modeStr[1] == '\0') {
                // Convert string to int
                int newTimeout = atoi(pass1Str);

                if(newTimeout > 0) {
                    currentTimeout = (uint32_t)newTimeout;

                    // Write New Timeout to EEPROM
                    EEPROMProgram(&currentTimeout, EEPROM_ADDR_TIMEOUT, sizeof(currentTimeout));

                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); // Blue ON (feedback)
                    SysCtlDelay(16000000);
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
                    UART1_SendString("Timeout$Saved#");
                }
            }

            // Clear buffer for next message
            memset(rxBuffer, 0, sizeof(rxBuffer));
        }
        else {                    // Normal character received
            if (rxIndex < RX_BUFFER_SIZE - 1) {
                rxBuffer[rxIndex++] = receivedChar;
            }
            else {
                // Buffer overflow → reset
                rxIndex = 0;
                memset(rxBuffer, 0, sizeof(rxBuffer));
            }
        }
    }
}