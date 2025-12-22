/*****************************************************************************
  hassabou's code
 * 
 * This implementation uses actual TivaWare peripheral library functions:
 *   - SysCtlPeripheralEnable()
 *   - EEPROMInit()
 *   - EEPROMProgram()
 *   - EEPROMRead()
 *   - EEPROMMassErase()
 * 
 * Include paths for TivaWare:
 *   - driverlib/sysctl.h
 *   - driverlib/eeprom.h
 *****************************************************************************/

#include "eepromDriver.h"

/******************************************************************************
 *                          Private Functions                                  *
 ******************************************************************************/

/*
 * CalculateAddress
 * Calculates byte address from block and offset.
 */
static uint32_t CalculateAddress(uint32_t block, uint32_t offset)
{
    return (block * EEPROM_BLOCK_SIZE * EEPROM_WORD_SIZE) + 
           (offset * EEPROM_WORD_SIZE);
}

/******************************************************************************
 *                          Public Functions                                   *
 ******************************************************************************/

/*
 * EEPROM_Init
 * Initializes the EEPROM module using TivaWare library.
 */
uint8_t EEPROM_Init(void)
{
    uint32_t result;
    
    /* Enable EEPROM peripheral using TivaWare function */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    
    /* Wait for EEPROM peripheral to be ready */
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0))
    {
    }
    
    /* Initialize EEPROM using TivaWare function */
    result = EEPROMInit();
    
    /* Check initialization result */
    if(result != EEPROM_INIT_OK)
    {
        return EEPROM_ERROR;
    }
    
    /* Check if timeout is initialized */
    uint8_t currentTimeout;
    if (EEPROM_ReadTimeout(&currentTimeout) == EEPROM_SUCCESS)
    {
        /* If uninitialized (0xFF) or invalid (0), set default */
        if (currentTimeout == 0xFF || currentTimeout == 0 || currentTimeout==0x0F)
        {
            EEPROM_SaveTimeout(15); /* Default 15 seconds */
        }
    }
    
    return EEPROM_SUCCESS;
}

/*
 * EEPROM_WriteWord
 * Writes a word to EEPROM using TivaWare EEPROMProgram().
 */
uint8_t EEPROM_WriteWord(uint32_t block, uint32_t offset, uint32_t data)
{
    uint32_t address;
    uint32_t result;
    
    /* Validate parameters */
    if(block >= EEPROM_TOTAL_BLOCKS || offset >= EEPROM_BLOCK_SIZE)
    {
        return EEPROM_ERROR;
    }
    
    /* Calculate byte address */
    address = CalculateAddress(block, offset);
    
    /* Write data using TivaWare function */
    result = EEPROMProgram(&data, address, sizeof(uint32_t));
    
    if(result != 0)
    {
        return EEPROM_ERROR;
    }
    
    return EEPROM_SUCCESS;
}

/*
 * EEPROM_ReadWord
 * Reads a word from EEPROM using TivaWare EEPROMRead().
 */
uint8_t EEPROM_ReadWord(uint32_t block, uint32_t offset, uint32_t *data)
{
    uint32_t address;
    
    /* Validate parameters */
    if(block >= EEPROM_TOTAL_BLOCKS || offset >= EEPROM_BLOCK_SIZE || data == 0)
    {
        return EEPROM_ERROR;
    }
    
    /* Calculate byte address */
    address = CalculateAddress(block, offset);
    
    /* Read data using TivaWare function */
    EEPROMRead(data, address, sizeof(uint32_t));
    
    return EEPROM_SUCCESS;
}

/*
 * EEPROM_WriteBuffer
 * Writes a buffer to EEPROM using TivaWare EEPROMProgram().
 */
uint8_t EEPROM_WriteBuffer(uint32_t block, uint32_t offset, const uint8_t *buffer, uint32_t length)
{
    uint32_t address;
    uint32_t result;
    
    /* Validate parameters */
    if(buffer == 0 || (length % 4) != 0)
    {
        return EEPROM_ERROR;
    }
    
    if(block >= EEPROM_TOTAL_BLOCKS || offset >= EEPROM_BLOCK_SIZE)
    {
        return EEPROM_ERROR;
    }
    
    /* Calculate starting byte address */
    address = CalculateAddress(block, offset);
    
    /* Write buffer using TivaWare function */
    /* Note: EEPROMProgram accepts uint32_t* so we cast, but data must be word-aligned */
    result = EEPROMProgram((uint32_t*)buffer, address, length);
    
    if(result != 0)
    {
        return EEPROM_ERROR;
    }
    
    return EEPROM_SUCCESS;
}

/*
 * EEPROM_ReadBuffer
 * Reads a buffer from EEPROM using TivaWare EEPROMRead().
 */
uint8_t EEPROM_ReadBuffer(uint32_t block, uint32_t offset, uint8_t *buffer, uint32_t length)
{
    uint32_t address;
    
    /* Validate parameters */
    if(buffer == 0 || (length % 4) != 0)
    {
        return EEPROM_ERROR;
    }
    
    if(block >= EEPROM_TOTAL_BLOCKS || offset >= EEPROM_BLOCK_SIZE)
    {
        return EEPROM_ERROR;
    }
    
    /* Calculate starting byte address */
    address = CalculateAddress(block, offset);
    
    /* Read buffer using TivaWare function */
    EEPROMRead((uint32_t*)buffer, address, length);
    
    return EEPROM_SUCCESS;
}

/*
 * EEPROM_MassErase
 * Erases entire EEPROM using TivaWare EEPROMMassErase().
 */
uint8_t EEPROM_MassErase(void)
{
    uint32_t result;
    
    /* Erase using TivaWare function */
    result = EEPROMMassErase();
    
    if(result != 0)
    {
        return EEPROM_ERROR;
    }
    
    return EEPROM_SUCCESS;
}

/*
 * EEPROM_SaveTimeout
 */
/*
 * EEPROM_SaveTimeout
 */
uint8_t EEPROM_SaveTimeout(uint8_t timeout)
{
    uint32_t word = 0;
    
    /* Read existing word to preserve Password tail */
    if (EEPROM_ReadWord(TIMEOUT_EEPROM_BLOCK, TIMEOUT_EEPROM_OFFSET, &word) != EEPROM_SUCCESS)
    {
        /* If read fails, we might be uninitialized, but we should try to proceed or error out. 
           Let's assume 0 if read fails (risky for password) or return error. */
         return EEPROM_ERROR;
    }
    
    /* Modify Byte 1 (Comma) and Byte 2 (Timeout) */
    /* Word Structure: [Byte 0: Pass4] [Byte 1: ','] [Byte 2: Timeout] [Byte 3: ?] */
    
    /* Clear Byte 1 and Byte 2 */
    word &= ~(0x00FFFF00);
    
    /* Set Comma (0x2C) at Byte 1 */
    word |= ((uint32_t)',' << 8);
    
    /* Set Timeout at Byte 2 */
    word |= ((uint32_t)timeout << 16);
    
    return EEPROM_WriteWord(TIMEOUT_EEPROM_BLOCK, TIMEOUT_EEPROM_OFFSET, word);
}

/*
 * EEPROM_ReadTimeout
 */
uint8_t EEPROM_ReadTimeout(uint8_t *timeout)
{
    uint32_t word;
    uint8_t status;
    
    if (timeout == 0)
    {
        return EEPROM_ERROR;
    }
    
    status = EEPROM_ReadWord(TIMEOUT_EEPROM_BLOCK, TIMEOUT_EEPROM_OFFSET, &word);
    
    if (status == EEPROM_SUCCESS)
    {
        /* Timeout is at Byte 2 (bits 16-23) */
        *timeout = (uint8_t)((word >> 16) & 0xFF);
    }
    
    return status;
}

/*
 * EEPROM_IsPasswordInitialized
 * Checks if the password has been initialized by reading the magic value.
 */
uint8_t EEPROM_IsPasswordInitialized(void)
{
    uint32_t magicValue = 0;
    
    if (EEPROM_ReadWord(PASSWORD_FLAG_EEPROM_BLOCK, PASSWORD_FLAG_EEPROM_OFFSET, &magicValue) != EEPROM_SUCCESS)
    {
        return 0U; /* Error reading, assume not initialized */
    }
    
    /* Check if magic value matches */
    if (magicValue == PASSWORD_INITIALIZED_MAGIC)
    {
        return 1U; /* Password is initialized */
    }
    
    return 0U; /* Password not initialized */
}

/*
 * EEPROM_SetPasswordInitialized
 * Marks the password as initialized by writing the magic value.
 */
uint8_t EEPROM_SetPasswordInitialized(void)
{
    return EEPROM_WriteWord(PASSWORD_FLAG_EEPROM_BLOCK, 
                           PASSWORD_FLAG_EEPROM_OFFSET, 
                           PASSWORD_INITIALIZED_MAGIC);
}
