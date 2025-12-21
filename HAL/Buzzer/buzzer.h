/*
 * buzzer.h
 *
 *  Created on: Dec 21, 2025
 */

#ifndef HAL_BUZZER_BUZZER_H_
#define HAL_BUZZER_BUZZER_H_

#include <stdint.h>

void buzzerInit(void);
void buzzerON(void);
void buzzerOFF(void);
void GPIOF_InterruptInit(void);

#endif /* HAL_BUZZER_BUZZER_H_ */
