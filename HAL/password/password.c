#include "password.h"
#include "../../BACK/EEPROM/eepromDriver.h"

/******************************************************************************
 *                            FUNCTION DEFINITIONS                             *
 ******************************************************************************/

/*
 * Password_Init
 * Initializes the password module.
 * Does NOT set a default password - user must initialize it on first use.
 */
uint8_t Password_Init(void)
{
    /* Just verify EEPROM access is working */
    /* No longer sets default password automatically */
    return PASSWORD_OK;
}

/*
 * Password_IsInitialized
 * Checks if password has been set by the user.
 * Returns: 1 if initialized, 0 if not
 */
uint8_t Password_IsInitialized(void)
{
    return EEPROM_IsPasswordInitialized();
}

/*
 * Password_FirstTimeSetup
 * Sets the password for the first time.
 * Can only be called when password is not yet initialized.
 */
uint8_t Password_FirstTimeSetup(const uint8_t *password)
{
    if (password == 0)
    {
        return PASSWORD_ERROR;
    }
    
    /* Check if already initialized */
    if (Password_IsInitialized() == 1U)
    {
        return PASSWORD_ERROR; /* Already initialized */
    }
    
    /* Save the password */
    if (Password_Save(password) != PASSWORD_OK)
    {
        return PASSWORD_ERROR;
    }
    
    /* Mark as initialized */
    if (EEPROM_SetPasswordInitialized() != EEPROM_SUCCESS)
    {
        return PASSWORD_ERROR;
    }
    
    
    return PASSWORD_OK;
}

uint8_t Password_Save(const uint8_t *password)
{
    uint32_t word0 = 0;
    uint32_t word1 = 0;

    if (password == 0)
        return PASSWORD_ERROR;

    word0 =  password[0]
          | (password[1] << 8)
          | (password[2] << 16)
          | (password[3] << 24);

    word1 = password[4];

    if (EEPROM_WriteWord(PASSWORD_EEPROM_BLOCK,
                         PASSWORD_EEPROM_OFFSET,
                         word0) != EEPROM_SUCCESS)
        return PASSWORD_ERROR;

    if (EEPROM_WriteWord(PASSWORD_EEPROM_BLOCK,
                         PASSWORD_EEPROM_OFFSET + 1,
                         word1) != EEPROM_SUCCESS)
        return PASSWORD_ERROR;

    return PASSWORD_OK;
}


uint8_t Password_Read(uint8_t *password)
{
    uint32_t word0, word1;

    if (password == 0)
        return PASSWORD_ERROR;

    if (EEPROM_ReadWord(PASSWORD_EEPROM_BLOCK,
                        PASSWORD_EEPROM_OFFSET,
                        &word0) != EEPROM_SUCCESS)
        return PASSWORD_ERROR;

    if (EEPROM_ReadWord(PASSWORD_EEPROM_BLOCK,
                        PASSWORD_EEPROM_OFFSET + 1,
                        &word1) != EEPROM_SUCCESS)
        return PASSWORD_ERROR;

    /* Bytes 0–3 from word0 */
    password[0] = (word0 >> 0)  & 0xFF;
    password[1] = (word0 >> 8)  & 0xFF;
    password[2] = (word0 >> 16) & 0xFF;
    password[3] = (word0 >> 24) & 0xFF;

    /* Byte 4 from word1 */
    password[4] = word1 & 0xFF;

    return PASSWORD_OK;
}


for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
{
    if (password[i] != stored[i])
        return PASSWORD_MISMATCH;
}
return PASSWORD_OK;


uint8_t Password_Change(const uint8_t *old_pass,
                        const uint8_t *new_pass)
{
    if (Password_Compare(old_pass) != PASSWORD_OK)
    {
        return PASSWORD_MISMATCH;
    }

    return Password_Save(new_pass);
}

uint8_t Password_GetRawDebug(uint32_t *buffer)
{
    if (buffer == 0)
    {
        return PASSWORD_ERROR;
    }

    if (EEPROM_ReadWord(PASSWORD_EEPROM_BLOCK, PASSWORD_EEPROM_OFFSET, &buffer[0]) != EEPROM_SUCCESS)
    {
        return PASSWORD_ERROR;
    }

    if (EEPROM_ReadWord(PASSWORD_EEPROM_BLOCK, PASSWORD_EEPROM_OFFSET + 1, &buffer[1]) != EEPROM_SUCCESS)
    {
        return PASSWORD_ERROR;
    }

    return PASSWORD_OK;
}

