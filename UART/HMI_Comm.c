#include "HMI_Comm.h"

volatile short failedAttempts = 0;  // ← definition
char rxBuffer[RX_BUFFER_SIZE];       // define buffer
volatile uint8_t rxIndex = 0;        // define index
volatile bool messageReady = false;  // define message flag

void UART1_Init(void)
{
    // 1️Enable peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART1));
    
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
    if (modeStr == NULL)
    {
        UART1_SendString("ERR#");
        return;
    }

    // ------------------------------
    // MODE 0: Open Door
    // ------------------------------
    if (strcmp(modeStr, "0") == 0)
    {
        char myPassword[PASSWORD_LENGTH];
        uint8_t res = Password_Compare((uint8_t *)passStr);
        if (res == PASSWORD_OK)
        {
            failedAttempts = 0;
            UART1_SendString("Door$Unlocked#");
            Door_Unlock();          // motor unlock
            //TimerStart(TIMEOUT_VALUE); // it is already in door unlock
        }
        else if(res == PASSWORD_MISMATCH)
        {
            failedAttempts++;
            UART1_SendString("WRONG#");
            if (failedAttempts >= 3)
            {
                failedAttempts = 0;
                Activate_Lockout(); // buzzer + delay (timer-based)
            }
            
        }
        else {
            UART1_SendString("ERR#");
        }
    }

    // ------------------------------
    // MODE 1: Change Password
    // ------------------------------
    else if (strcmp(modeStr, "1") == 0)
    {
        uint8_t res = Password_Change((uint8_t *)passStr, (uint8_t *)valueStr);
        if (res == PASSWORD_OK)
        {
            failedAttempts = 0;
            UART1_SendString("CHANGED#");
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
            if (passStr == NULL)
            {
                UART1_SendString("ERR#");
                return;
            }
            uint8_t res = Password_Compare((uint8_t *)passStr);
            if (res = PASSWORD_MISMATCH)
            {
                UART1_SendString("WRONG#");
                        // motor unlock
                //TimerStart(TIMEOUT_VALUE); // it is already in door unlock
            }
            uint8_t timeout = atoi(passStr);

            if (timeout >= 5 && timeout <= 30)
            {
                EEPROM_SaveTimeout(timeout);
                UART1_SendString("Time Saved#");
            }
            else
            {
                UART1_SendString("BAD_VALUE#");
            }
        
        
    }
    else if (strcmp(modeStr, "3") == 0)
    {
        //change password 3ala toul
    }
    else if (strcmp(modeStr, "4") == 0)
    {
        uint8_t res = Password_IsInitialized();
        if (res == 1)
        {
            UART1_SendString("1#");
        }
        else
        {
            UART1_SendString("0#");
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



void Door_Unlock(void) {
    // Activate the unlocking mechanism
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_PIN_6); // Start with LED OFF
   // Example for Tiva C, set pin low to unlock
    // Optionally start a timer to relock
    TimerStart();
    //Start_AutoLock_Timer();
}

//void Start_AutoLock_Timer(void)
//{
    // Reload the timer with the auto-lock timeout value
      // Use the reusable TimerStart function

    // Optional: if you want, you can clear any pending interrupts inside TimerStart itself
//}

void Activate_Lockout(void) {
    // Optionally turn on an LED or buzzer
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);

    // Start lockout timer
    Timer1AStart(LOCKOUT_DURATION);
    // TimerLoadSet(TIMER1_BASE, TIMER_A, LOCKOUT_DURATION);
    // TimerEnable(TIMER1_BASE, TIMER_A);
}




void Door_Lock(void){

  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); // Start with LED OFF

}