#ifndef __DRIVE_BUZZER_H__
#define __DRIVE_BUZZER_H__

#include "typedef.h"

void drv_buzzerInit(void);
void drv_buzzerHandler10ms(void);
void drv_buzzerNumber(UINT8 num);
void drv_buzzerOn(UINT8 time);


#endif
