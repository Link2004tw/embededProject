#ifndef PASSWORD_MANAGER_H_
#define PASSWORD_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

#define PASSWORD_LENGTH 5
#define MAX_ATTEMPTS 3

void PASSWORD_Init(void);                    
void PASSWORD_SetNew(void);                
bool PASSWORD_Check(void); 
uint8_t PASSWORD_GetAttempts(void);


#endif
