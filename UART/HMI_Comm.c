#include "HMI_Comm.h"

volatile short failedAttempts = 0;  // ← definition
char rxBuffer[RX_BUFFER_SIZE];       // define buffer
volatile uint8_t rxIndex = 0;        // define index
volatile bool messageReady = false;  // define message flag

void UART1_Init(void)
{
    // 1️Enable peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART1));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    // 2️Configure GPIO pins for UART1
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // 3️Configure UART
    UARTDisable(UART1_BASE);

    UARTConfigSetExpClk(
        UART1_BASE,
        SysCtlClockGet(),
        9600,
        UART_CONFIG_WLEN_8 |
        UART_CONFIG_STOP_ONE |
        UART_CONFIG_PAR_NONE
    );

    // Enable FIFOs (recommended)
    UARTFIFOEnable(UART1_BASE);

    // Clear RX FIFO
    while (UARTCharsAvail(UART1_BASE))
    {
        UARTCharGetNonBlocking(UART1_BASE);
    }

    // Clear UART errors
    UARTRxErrorClear(UART1_BASE);

    // 4️Enable UART interrupts
    UARTIntDisable(UART1_BASE, 0xFFFFFFFF);

    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);

    IntEnable(INT_UART1);

    // 5Enable UART
    UARTEnable(UART1_BASE);
}


void UART1_Handler(void)
{
    uint32_t status = UARTIntStatus(UART1_BASE, true);
    UARTIntClear(UART1_BASE, status);

    while (UARTCharsAvail(UART1_BASE))
    {
        char c = UARTCharGetNonBlocking(UART1_BASE);

        if (c == '#')
        {
            rxBuffer[rxIndex] = '\0';
            rxIndex = 0;
            messageReady = true;
        }
        else if (rxIndex < RX_BUFFER_SIZE - 1)
        {
            rxBuffer[rxIndex++] = c;
        }
    }
}


void PROCESS_MESSAGE(void)
{
    char rxCopy[RX_BUFFER_SIZE];
    strcpy(rxCopy, rxBuffer);

    char *modeStr  = strtok(rxCopy, ",");
    char *passStr  = strtok(NULL, ",");
    char *valueStr = strtok(NULL, ",");

    // ------------------------------
    // Basic format validation
    // ------------------------------
    if (modeStr == NULL || passStr == NULL)
    {
        UART1_SendString("ERR#");
        return;
    }

    // ------------------------------
    // MODE 0: Open Door
    // ------------------------------
    if (strcmp(modeStr, "0") == 0)
    {
        if (strcmp(passStr, PASSWORD) == 0)
        {
            failedAttempts = 0;

            UART1_SendString("OK#");

            Door_Unlock();          // motor unlock
            Start_AutoLock_Timer(); // uses stored timeout
        }
        else
        {
            failedAttempts++;
            UART1_SendString("WRONG#");

            if (failedAttempts >= 3)
            {
                failedAttempts = 0;
                Activate_Lockout(); // buzzer + delay (timer-based)
            }
        }
    }

    // ------------------------------
    // MODE 1: Change Password
    // ------------------------------
    else if (strcmp(modeStr, "1") == 0)
    {
        if (strcmp(passStr, PASSWORD) == 0 && valueStr != NULL)
        {
            SavePasswordToEEPROM(valueStr);
            UART1_SendString("CHANGED#");
            failedAttempts = 0;
        }
        else
        {
            failedAttempts++;
            UART1_SendString("WRONG#");

            if (failedAttempts >= 3)
            {
                failedAttempts = 0;
                Activate_Lockout();
            }
        }
    }

    // ------------------------------
    // MODE 2: Set Timeout
    // ------------------------------
    else if (strcmp(modeStr, "2") == 0)
    {
        if (strcmp(passStr, PASSWORD) == 0 && valueStr != NULL)
        {
            uint8_t timeout = atoi(valueStr);

            if (timeout >= 5 && timeout <= 30)
            {
                SaveTimeoutToEEPROM(timeout);
                UART1_SendString("TIMEOUT_OK#");
                failedAttempts = 0;
            }
            else
            {
                UART1_SendString("BAD_VALUE#");
            }
        }
        else
        {
            failedAttempts++;
            UART1_SendString("WRONG#");

            if (failedAttempts >= 3)
            {
                failedAttempts = 0;
                Activate_Lockout();
            }
        }
    }

    // ------------------------------
    // Unknown mode
    // ------------------------------
    else
    {
        UART1_SendString("ERR#");
    }

    // Clear buffer after processing
    memset(rxBuffer, 0, sizeof(rxBuffer));
}
void UART1_SendString(char* str)
{
    while (*str)
    {
        // Wait until space is available in TX FIFO
        UARTCharPut(UART1_BASE, *str++);
    }
}

// Simple placeholder function
void EEPROMWriteByte(uint32_t base, uint32_t addr, uint8_t data)
{
    // TODO: implement actual EEPROM or Flash writing
    // For now, just ignore or store in RAM for testing
    (void)base;
    (void)addr;
    (void)data;
}

void Door_Unlock(void) {
    // Activate the unlocking mechanism
    GPIOPinWrite(LOCK_PORT, LOCK_PIN, LOCK_PIN); // Example for Tiva C, set pin low to unlock
    // Optionally start a timer to relock
    Start_AutoLock_Timer();
}

void Start_AutoLock_Timer(void) {
    // Configure a timer (SysTick or hardware timer) to call AutoLock function after timeout
    TimerLoadSet(TIMER0_BASE, TIMER_A, TIMEOUT_VALUE); // Example
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void Activate_Lockout(void) {
    // Optionally turn on an LED or buzzer
    GPIOPinWrite(LED_PORT, LED_PIN, 1);

    // Start lockout timer
    TimerLoadSet(TIMER1_BASE, TIMER_A, LOCKOUT_DURATION);
    TimerEnable(TIMER1_BASE, TIMER_A);
}


void SavePasswordToEEPROM(char *newPass) {
    for(uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
        EEPROMWriteByte(EEPROM_BASE, i, newPass[i]);
    }
}

void SaveTimeoutToEEPROM(uint8_t timeout) {
    EEPROMWriteByte(EEPROM_BASE, TIMEOUT_ADDRESS, timeout);
}