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
    uint32_t word = 0U;
    uint8_t i;

    if (password == 0)
    {
        return PASSWORD_ERROR;
    }

    for (i = 0U; i < PASSWORD_LENGTH; i++)
    {
        /* Case 1: First 4 bytes (Index 0-3), standard accumulation */
        if (i < 4)
        {
            word |= ((uint32_t)password[i] << ((i % 4U) * 8U));
            
            /* If we filled the word (i=3), write it */
            if (i == 3)
            {
                if (EEPROM_WriteWord(PASSWORD_EEPROM_BLOCK,
                                    PASSWORD_EEPROM_OFFSET + 0,
                                    word) != EEPROM_SUCCESS)
                {
                    return PASSWORD_ERROR;
                }
                word = 0U;
            }
        }
        /* Case 2: 5th byte (Index 4), Shared Word logic */
        else if (i == 4)
        {
            uint32_t existingWord = 0;
            
            /* Read existing Word 1 to preserve Comma and Timeout */
            if (EEPROM_ReadWord(PASSWORD_EEPROM_BLOCK,
                                PASSWORD_EEPROM_OFFSET + 1,
                                &existingWord) != EEPROM_SUCCESS)
            {
               return PASSWORD_ERROR;
            }
            
            /* Modify Byte 0 (Pass4) only */
            existingWord &= ~(0x000000FF); /* Clear Byte 0 */
            existingWord |= (uint32_t)password[i]; /* Set Byte 0 */
            
            /* Write Word 1 back */
            if (EEPROM_WriteWord(PASSWORD_EEPROM_BLOCK,
                                PASSWORD_EEPROM_OFFSET + 1,
                                existingWord) != EEPROM_SUCCESS)
            {
                return PASSWORD_ERROR;
            }
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
    if(strcmp((char*)password, (char*)stored) == 0){
        return PASSWORD_OK;
    }
    else{
        return PASSWORD_MISMATCH;
    }
    // for (i = 0U; i < PASSWORD_LENGTH; i++)
    // {
    //     if (password[i] != stored[i])
    //     {
    //         return PASSWORD_MISMATCH;
    //     }
    // }

    //return PASSWORD_OK;
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

