#include "HMI_Comm.h"
#include <string.h>
#include <stdlib.h>

// --- MACROS & DEFINITIONS (From Snippet) ---
#define EEPROM_MAGIC_NUMBER      0xA5A5A5A5
#define EEPROM_ADDR_MAGIC        0x0000         // Address 0: Magic Number
#define EEPROM_ADDR_TIMEOUT      0x0004         // Address 4: Timeout Value
#define EEPROM_ADDR_PASS         0x0010         // Address 16: Password (start)
#define PASSWORD_MAX_LEN         8              // Max password length (multiple of 4)

// --- GLOBAL VARIABLES ---
char rxBuffer[RX_BUFFER_SIZE];
volatile uint8_t rxIndex = 0;
volatile bool messageReady = false;

char currentPassword[PASSWORD_MAX_LEN];
uint32_t currentTimeout = 30;
short failedAttempts = 0;

// --- FUNCTION PROTOTYPES ---
void Save_Settings(void);
void BUZZ(void);

// --- HELPER: SAVE SETTINGS TO EEPROM ---
void Save_Settings(void) {
    uint32_t magic = EEPROM_MAGIC_NUMBER;

    // Save Magic Number
    EEPROMProgram(&magic, EEPROM_ADDR_MAGIC, sizeof(magic));

    // Save Timeout
    EEPROMProgram(&currentTimeout, EEPROM_ADDR_TIMEOUT, sizeof(currentTimeout));

    // Save Password (cast to uint32_t* for Tiva API)
    EEPROMProgram((uint32_t *)currentPassword, EEPROM_ADDR_PASS, PASSWORD_MAX_LEN);
}

// --- INITIALIZATION ---
void UART1_Init(void) {
    // 1. Enable Peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);

    // Wait for peripherals to be ready
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART1));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0));

    // 2. Configure GPIO (LEDs & UART Pins)
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_6);

    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // 3. EEPROM Initialization & Check
    uint32_t eepromStatus = EEPROMInit();
    if(eepromStatus != EEPROM_INIT_OK) {
        // Error: Turn on Red LED forever
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
    }

    // Check for First Time Run (Magic Number)
    uint32_t checkMagic;
    EEPROMRead(&checkMagic, EEPROM_ADDR_MAGIC, sizeof(checkMagic));

    if (checkMagic != EEPROM_MAGIC_NUMBER) {
        // FIRST TIME: Write Defaults
        strcpy(currentPassword, "12345");
        currentTimeout = 30;
        Save_Settings();
    }
    else {
        // SUBSEQUENT TIMES: Read from EEPROM
        EEPROMRead(&currentTimeout, EEPROM_ADDR_TIMEOUT, sizeof(currentTimeout));
        EEPROMRead((uint32_t *)currentPassword, EEPROM_ADDR_PASS, PASSWORD_MAX_LEN);
    }

    // 4. Configure UART settings
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    UARTFIFOEnable(UART1_BASE);

    // Clear RX FIFO
    while (UARTCharsAvail(UART1_BASE)) {
        UARTCharGetNonBlocking(UART1_BASE);
    }

    // 5. Enable Interrupts
    UARTIntDisable(UART1_BASE, 0xFFFFFFFF);
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
    IntEnable(INT_UART1);
    UARTEnable(UART1_BASE);

    // Startup Flash (Green LED)
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    SysCtlDelay(8000000);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
}

// --- INTERRUPT HANDLER ---
void UART1_Handler(void) {
    uint32_t status = UARTIntStatus(UART1_BASE, true);
    UARTIntClear(UART1_BASE, status);

    while (UARTCharsAvail(UART1_BASE)) {
        char c = UARTCharGetNonBlocking(UART1_BASE);

        if (c == '#') {
            rxBuffer[rxIndex] = '\0';     // Null terminate
            rxIndex = 0;
            messageReady = true;          // Signal main loop
        }
        else if (rxIndex < RX_BUFFER_SIZE - 1) {
            rxBuffer[rxIndex++] = c;
        }
    }
}

// --- SEND STRING ---
void UART1_SendString(char* str) {
    // Green LED feedback during TX
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);

    while(*str) {
        // Wait until there is space in the FIFO
        UARTCharPut(UART1_BASE, *str++);
    }
    while(UARTBusy(UART1_BASE));

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
}

// --- BUZZER / ALARM ---
void BUZZ(void) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);     // Red LED ON
    SysCtlDelay(16000000);     // Delay
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
}

// --- PROCESS MESSAGE (Call this when messageReady == true) ---
void PROCESS_MESSAGE(void) {
    // Create a local copy to manipulate with strtok
    char rxCopy[RX_BUFFER_SIZE];
    strcpy(rxCopy, rxBuffer);

    char *modeStr;
    char *pass1Str;
    char *pass2Str;

    // Expected format: Mode,Param1,Param2#
    modeStr  = strtok(rxCopy, ",");
    pass1Str = strtok(NULL, ",");
    pass2Str = strtok(NULL, ",");         // Can be NULL

    // Basic Validation
    if (modeStr == NULL || pass1Str == NULL) {
        UART1_SendString("ERR#");
        messageReady = false;
        return;
    }

    // --------------------------------------------------
    // MODE 0: UNLOCK DOOR (0,password#)
    // --------------------------------------------------
    if (strcmp(modeStr, "0") == 0) {
        if (strcmp(pass1Str, currentPassword) == 0) {
            UART1_SendString("OK#");
            failedAttempts = 0;

            // Unlock Hardware Logic
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);     // Green LED

            // Call your timer/unlock function here:
            // Door_Unlock();

            // --- RELAY CONTROL (PB6) ---
            GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_PIN_6); // Turn ON Relay
            SysCtlDelay(16000000);                                 // Delay ~1-3 seconds (Adjust based on clock speed)
            GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0);          // Turn OFF Relay
        }
        else {
            failedAttempts++;
            UART1_SendString("WRONG#");
            if (failedAttempts >= 3) {
                BUZZ();
                failedAttempts = 0;
            }
        }
    }

    // --------------------------------------------------
    // MODE 1: CHANGE PASSWORD (1,old_pass,new_pass#)
    // --------------------------------------------------
    else if (strcmp(modeStr, "1") == 0) {
        // Verify Old Password first
        if (strcmp(pass1Str, currentPassword) == 0) {
            // Check if New Password exists
            if (pass2Str != NULL && pass2Str[0] != '\0') {
                // Update RAM
                memset(currentPassword, 0, PASSWORD_MAX_LEN);
                strncpy(currentPassword, pass2Str, PASSWORD_MAX_LEN - 1);

                // Update EEPROM
                EEPROMProgram((uint32_t *)currentPassword, EEPROM_ADDR_PASS, PASSWORD_MAX_LEN);

                UART1_SendString("CHANGED#");
                failedAttempts = 0;
            }
            else {
                UART1_SendString("ERR_EMPTY#");
            }
        }
        else {
            failedAttempts++;
            UART1_SendString("WRONG_OLD#");
            if (failedAttempts >= 3) {
                BUZZ();
                failedAttempts = 0;
            }
        }
    }

    // --------------------------------------------------
    // MODE 2: SET TIMEOUT (2,new_timeout#)
    // --------------------------------------------------
    else if (strcmp(modeStr, "2") == 0) {
        int newTimeout = atoi(pass1Str);

        if (newTimeout >= 5 && newTimeout <= 60) {
            currentTimeout = (uint32_t)newTimeout;

            // Save to EEPROM
            EEPROMProgram(&currentTimeout, EEPROM_ADDR_TIMEOUT, sizeof(currentTimeout));

            // Blue LED Feedback
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
            SysCtlDelay(16000000);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

            UART1_SendString("TIMEOUT_OK#");
        }
        else {
            UART1_SendString("BAD_VALUE#");
        }
    }

    // Unknown Mode
    else {
        UART1_SendString("ERR_MODE#");
    }

    // Reset Flag
    messageReady = false;
    memset(rxBuffer, 0, sizeof(rxBuffer));
}