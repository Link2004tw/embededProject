#ifndef DISPLAY_MANAGER_H_
#define DISPLAY_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../HAL/LCD/lcd.h"
#include "potentiometer_manager.h"
#include "input_manager.h"
//#include "lcd.h"
#include "../Comm/HMI_Comm.h"
//bool static flag = false;
//char static savedPassword[5]="";

void DISPLAY_Init(void);

void DISPLAY_HandleKey(char key);

void DISPLAY_EnablePasswordMode(void);
void DISPLAY_DisablePasswordMode(void);

void DISPLAY_ShowMainMenu(void);
void DISPLAY_ShowMessage(const char *msg);
void DISPLAY_ShowTimeoutValue(uint8_t value);
void DISPLAY_ClearScreen(void);
void DISPLAY_OPEN(void);
void DISPLAY_CHANGEPASSWORD(void);
void DISPLAY_CHANGETIMEOUT(void);
void DISPLAY_ERROR(void);
void DISPLAY_NEW_PASSWORD(void);
void DISPLAY_OLD_PASSWORD(void);
void SHOW_BUFFER(char* buffer);
#endif