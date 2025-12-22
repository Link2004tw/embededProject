#ifndef PASSWORD_H_
#define PASSWORD_H_

#include <stdint.h>

/******************************************************************************
 *                                DEFINES                                     *
 ******************************************************************************/

/* Password properties */
#define PASSWORD_LENGTH            5U
#define PASSWORD_WORDS             2U   /* 5 bytes fit in 2 EEPROM words */

/* EEPROM mapping */
#define PASSWORD_EEPROM_BLOCK      0U
#define PASSWORD_EEPROM_OFFSET     0U

/* Return codes */
#define PASSWORD_OK                0U
#define PASSWORD_ERROR             1U
#define PASSWORD_MISMATCH          2U
#define PASSWORD_NOT_INITIALIZED   3U

/******************************************************************************
 *                           FUNCTION PROTOTYPES                               *
 ******************************************************************************/

/* Initialize password module - sets default password if EEPROM is empty */
uint8_t Password_Init(void);

/* Check if password has been initialized */
uint8_t Password_IsInitialized(void);

/* Initialize password for the first time (user sets it) */
uint8_t Password_FirstTimeSetup(const uint8_t *password);

/* Save new password to EEPROM */
uint8_t Password_Save(const uint8_t *password);

/* Read stored password from EEPROM */
uint8_t Password_Read(uint8_t *password);

/* Compare entered password with stored one */
uint8_t Password_Compare(const uint8_t *password);

/* Change password after verifying old one */
uint8_t Password_Change(const uint8_t *old_pass,
                        const uint8_t *new_pass);

#endif /* PASSWORD_H_ */
