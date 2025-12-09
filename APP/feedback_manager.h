#ifndef FEEDBACK_MANAGER_H_
#define FEEDBACK_MANAGER_H_

#include <stdint.h>

void DISPLAY_AskEnterPassword(void);
void DISPLAY_AskConfirmPassword(void);
void DISPLAY_AskPassword(void);
void DISPLAY_PasswordMismatch(void);
void DISPLAY_WrongPassword(uint8_t attempt);
void DISPLAY_Lockout(void);
void DISPLAY_AccessGranted(void);
void DISPLAY_AccessDenied(void);
void DISPLAY_DoorOpening(void);
void DISPLAY_DoorOpenWait(uint8_t sec);
void DISPLAY_DoorClosing(void);
void DISPLAY_ShowTimeoutMenu(void);
void DISPLAY_SaveTimeoutPrompt(void);
void DISPLAY_TimeoutSaved(void);
void DISPLAY_Processing(void);
void DISPLAY_Error(const char *msg);