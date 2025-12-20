#ifndef PASSWORD_MANAGER_H_
#define PASSWORD_MANAGER_H_

#include <stdint.h>

/******************************************************************************
 *                                DEFINES                                     *
 ******************************************************************************/

#define MAX_PASSWORD_ATTEMPTS      3U

/******************************************************************************
 *                           FUNCTION PROTOTYPES                               *
 ******************************************************************************/

/* Handle password check for opening door */
uint8_t PasswordMgr_Verify(void);

/* Handle change password process */
uint8_t PasswordMgr_Change(void);

/* Reset attempts counter */
void PasswordMgr_ResetAttempts(void);

#endif /* PASSWORD_MANAGER_H_ */
