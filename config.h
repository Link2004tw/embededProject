#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/eeprom.h"

// Communication Constants
#define UART_BAUD_RATE 9600
#define EEPROM_TIMEOUT_ADDR 0x0000
#define EEPROM_PASS_ADDR    0x0010

// Command IDs to distinguish data types
#define CMD_SEND_TIMEOUT  0xAA
#define CMD_SEND_PASSWORD 0xBB
#define CMD_ACK           0x06
#define CMD_NACK          0x15

// Frontend uses UART5 (PE4 = Rx, PE5 = Tx)
void Init_Frontend_UART5(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // Unlock PE7 if needed, but we are using PE4/PE5 so it's fine.
    // Configure Pins for UART Mode
    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

// Backend uses UART1 (PB0 = Rx, PB1 = Tx)
void Init_Backend_UART1(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

// Simple blocking UART Send
void UART_SendByte(uint32_t ui32Base, uint8_t data) {
    UARTCharPut(ui32Base, data);
}

// Simple blocking UART Receive
uint8_t UART_ReceiveByte(uint32_t ui32Base) {
    return UARTCharGet(ui32Base);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------*/

// The function that send the timeout value from Frontend to Backend
void Frontend_SendPotentiometer(void) {
    // Init ADC0 on PE0 if not already done
    static bool isAdcInit = false;
    if(!isAdcInit) {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); // Port E already enabled by UART, but good practice
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);
        ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
        ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);
        ADCSequenceEnable(ADC0_BASE, 3);
        ADCIntClear(ADC0_BASE, 3);
        isAdcInit = true;
    }

    // Trigger and Read
    ADCProcessorTrigger(ADC0_BASE, 3);
    while(!ADCIntStatus(ADC0_BASE, 3, false));
    ADCIntClear(ADC0_BASE, 3);
    
    uint32_t adcValue;
    ADCSequenceDataGet(ADC0_BASE, 3, &adcValue);

    // Send Protocol: [CMD] [High Byte] [Low Byte]
    UART_SendByte(UART5_BASE, CMD_SEND_TIMEOUT);
    UART_SendByte(UART5_BASE, (adcValue >> 8) & 0xFF);
    UART_SendByte(UART5_BASE, adcValue & 0xFF);
}

/*-----------------------------------------------------------------------------*/

// The function that send the password value from Frontend to Backend
void Frontend_SendPassword(uint32_t* passwordBuffer, uint8_t length) {
    UART_SendByte(UART5_BASE, CMD_SEND_PASSWORD);
    UART_SendByte(UART5_BASE, length); 

    for(int i = 0; i < length; i++) {
        // Sending uint32_t as raw bytes or just casting to uint8 if password is digits 0-9
        // Assuming password is simpler digits (0-9), we send as byte.
        UART_SendByte(UART5_BASE, (uint8_t)passwordBuffer[i]);
    }

    // Wait for ACK/NACK from Backend
    uint8_t response = UART_ReceiveByte(UART5_BASE);
    if(response == CMD_ACK) {
        // Turn on Green LED (Example: PF3)
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); 
    } else {
        // Turn on Red LED (Example: PF1)
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); 
    }
}

/*----------------------------------------------------------------------------*/

// Read saved values from the EEPROM
void Backend_ReadValues(uint32_t* storedTimeout, uint32_t* storedPasswordBuffer, uint32_t passLen) {
    // Read Timeout
    EEPROMRead(storedTimeout, EEPROM_TIMEOUT_ADDR, sizeof(uint32_t));

    // Read Password
    EEPROMRead(storedPasswordBuffer, EEPROM_PASS_ADDR, passLen * sizeof(uint32_t));
}

/*----------------------------------------------------------------------------*/

// Checks if the entered password is similar to the one saved in the EEPROM or not
void Backend_ListenAndProcess(void) {
    // Wait for a command byte
    if (UARTCharsAvail(UART1_BASE)) {
        uint8_t cmd = UART_ReceiveByte(UART1_BASE);

        if (cmd == CMD_SEND_TIMEOUT) {
            uint32_t high = UART_ReceiveByte(UART1_BASE);
            uint32_t low = UART_ReceiveByte(UART1_BASE);
            uint32_t timeoutVal = (high << 8) | low;

            // Save Timeout to EEPROM
            EEPROMProgram(&timeoutVal, EEP_TIMEOUT_ADDR, sizeof(uint32_t));
        }
        else if (cmd == CMD_SEND_PASSWORD) {
            uint8_t len = UART_ReceiveByte(UART1_BASE);
            uint32_t receivedPass[16]; 
            
            for(int i=0; i<len; i++) {
                receivedPass[i] = (uint32_t)UART_ReceiveByte(UART1_BASE);
            }

            // Verify Password using your eepromDriver.h function
            // Ensure VerifyPassword reads from the correct address defined in the driver
            bool match = VerifyPassword(receivedPass, len);

            if(match) {
                UART_SendByte(UART1_BASE, CMD_ACK);
            } else {
                UART_SendByte(UART1_BASE, CMD_NACK);
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------*/

// main function
#include "systick.h"
#include "eepromDriver.h"
// ... include the helper functions above ...

#define ROLE_FRONTEND  1
#define ROLE_BACKEND   2

// *** CHANGE THIS LINE BEFORE FLASHING EACH BOARD ***
#define CURRENT_ROLE   ROLE_FRONTEND 

int main(void) {
    // System Clock (16MHz)
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    SysTick_Init(16000000 / 1000); // 1ms delay capability

    // Debug LEDs (Port F)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    if (BOARD_ROLE == ROLE_FRONTEND) {
        // --- FRONTEND SETUP ---
        Init_Frontend_UART5(); // UART5 for Frontend

        // Dummy Password for testing (Replace with your Keypad Logic)
        uint32_t testPass[] = {1, 2, 3, 4}; 
        
        while(1) {
            // Send Potentiometer Reading
            Frontend_SendPotentiometer();
            DelayMs(50);

            // Send Password (triggered every 2 seconds for test)
            Frontend_SendPassword(testPass, 4);
            DelayMs(2000);
        }

    } else {
        // --- BACKEND SETUP ---
        eepromInit();        // Initialize EEPROM
        Init_Backend_UART1(); // UART1 for Backend

        while(1) {
            Backend_ListenAndProcess();
        }
    }
}