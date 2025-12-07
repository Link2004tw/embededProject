#ifndef DISPLAY_MANAGER_H_
#define DISPLAY_MANAGER_H_

#include <stdint.h>

void DISPLAY_Init(void);

void DISPLAY_HandleKey(char key);

void DISPLAY_EnablePasswordMode(void);
void DISPLAY_DisablePasswordMode(void);

void DISPLAY_ShowMainMenu(void);
void DISPLAY_ShowMessage(const char *msg);
void DISPLAY_ShowTimeoutValue(uint8_t value);
void DISPLAY_ClearScreen(void);

#endif