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
        if (stored[i] != EEPROM_UNINITIALIZED)
        {
            uninitialized = 0U;
            break;
        }
    }
    
    /* If uninitialized, set default password "12345" */
    if (uninitialized == 1U)
    {
        const uint8_t defaultPassword[PASSWORD_LENGTH] = {'1', '2', '3', '4', '5'};
        return Password_Save(defaultPassword);
    }
    
    return PASSWORD_OK;
}

uint8_t Password_Save(const uint8_t *password)
{
    uint32_t word = 0U;
    uint8_t i;

    if (password == 0)
    {
        return PASSWORD_ERROR;
    }

    for (i = 0U; i < PASSWORD_LENGTH; i++)
    {
        word |= ((uint32_t)password[i] << ((i % 4U) * 8U));

        if ((i % 4U) == 3U || (i == (PASSWORD_LENGTH - 1U)))
        {
            /* Error checking: verify EEPROM write succeeded */
            if (EEPROM_WriteWord(PASSWORD_EEPROM_BLOCK,
                                 PASSWORD_EEPROM_OFFSET + (i / 4U),
                                 word) != EEPROM_SUCCESS)
            {
                return PASSWORD_ERROR;
            }
            word = 0U;
        }
    }

    return PASSWORD_OK;
}

uint8_t Password_Read(uint8_t *password)
{
    uint32_t word;
    uint8_t i;

    if (password == 0)
    {
        return PASSWORD_ERROR;
    }

    for (i = 0U; i < PASSWORD_LENGTH; i++)
    {
        /* Error checking: verify EEPROM read succeeded */
        if (EEPROM_ReadWord(PASSWORD_EEPROM_BLOCK,
                            PASSWORD_EEPROM_OFFSET + (i / 4U),
                            &word) != EEPROM_SUCCESS)
        {
            return PASSWORD_ERROR;
        }

        password[i] = (uint8_t)((word >> ((i % 4U) * 8U)) & 0xFFU);
    }

    return PASSWORD_OK;
}

uint8_t Password_Compare(const uint8_t *password)
{
    uint8_t stored[PASSWORD_LENGTH];
    uint8_t i;

    if (password == 0)
    {
        return PASSWORD_ERROR;
    }

    /* Error checking: verify Password_Read succeeded */
    if (Password_Read(stored) != PASSWORD_OK)
    {
        return PASSWORD_ERROR;
    }

    for (i = 0U; i < PASSWORD_LENGTH; i++)
    {
        if (password[i] != stored[i])
        {
            return PASSWORD_MISMATCH;
        }
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
