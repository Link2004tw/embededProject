#include "password.h"
#include "../../BACK/EEPROM/eepromDriver.h"

/******************************************************************************
 *                            FUNCTION DEFINITIONS                             *
 ******************************************************************************/

/*
 * Password_Init
 * Checks if password is initialized in EEPROM.
 * If EEPROM is uninitialized (all 0xFF), sets default password "12345".
 */
uint8_t Password_Init(void)
{
    uint8_t stored[PASSWORD_LENGTH];
    uint8_t i;
    uint8_t uninitialized = 1U;
    
    /* Read current password from EEPROM */
    if (Password_Read(stored) != PASSWORD_OK)
    {
        return PASSWORD_ERROR;
    }
    
    /* Check if EEPROM is uninitialized (all bytes are 0xFF) */
    for (i = 0U; i < PASSWORD_LENGTH; i++)
    {
        if (stored[i] != EEPROM_UNINITIALIZED && stored[i] != 0x1E)
        {
            uninitialized = 0U;
            break;
        }
    }
    
    /* If uninitialized, set default password "12345" */
//    if (uninitialized == 1U)
//    {
//        const uint8_t defaultPassword[PASSWORD_LENGTH] = {'1', '2', '3', '4', '5'};
//        return Password_Save(defaultPassword);
//    }
    
    return PASSWORD_OK;
}

/*
 * Password_IsInitialized
 * Checks if password has been initialized in EEPROM.
 * Returns: 1 if initialized, 0 if not initialized
 */
#define PASSWORD_MAGIC 0xA5  /* Magic byte to indicate valid password */

#define PASSWORD_MAGIC_OFFSET  2  /* Store magic after 2 words of password */
#define PASSWORD_MAGIC_VALUE   0xA5C3E7B1  /* Unique validation pattern */

/* Modified Save function - adds magic number */
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

    /* Write magic number to indicate valid password */
    if (EEPROM_WriteWord(PASSWORD_EEPROM_BLOCK,
                         PASSWORD_EEPROM_OFFSET + PASSWORD_MAGIC_OFFSET,
                         PASSWORD_MAGIC_VALUE) != EEPROM_SUCCESS)
        return PASSWORD_ERROR;

    return PASSWORD_OK;
}

/* Improved IsInitialized using magic number */
uint8_t Password_IsInitialized(void)
{
    uint32_t magic;
    
    /* Read magic number from EEPROM */
    if (EEPROM_ReadWord(PASSWORD_EEPROM_BLOCK,
                        PASSWORD_EEPROM_OFFSET + PASSWORD_MAGIC_OFFSET,
                        &magic) != EEPROM_SUCCESS)
    {
        return 0U; /* Error reading */
    }
    
    /* Check if magic number matches */
    return (magic == PASSWORD_MAGIC_VALUE) ? 1U : 0U;
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

uint8_t Password_Compare(const uint8_t *password){
  
  uint8_t stored[PASSWORD_LENGTH];
  if (Password_Read(stored) != PASSWORD_OK)
    {
        return 0U; /* Error reading, assume not initialized */
    }
for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
{
  
    if (password[i] != stored[i])
        return PASSWORD_MISMATCH;
}
return PASSWORD_OK;
}

uint8_t Password_Change(const uint8_t *old_pass,
                        const uint8_t *new_pass)
{
    if (Password_Compare(old_pass) != PASSWORD_OK)
    {
        return PASSWORD_MISMATCH;
    }

    return Password_Save(new_pass);
}
