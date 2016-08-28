#ifndef __APP_TIME_H__
#define __APP_TIME_H__

#include "typedef.h"
#include "app_adc.h"

#define TIME_SAVE_SECTOR                2

extern data UINT16 u16_DisplayTime;

void app_timeInit(void);
void app_timeClear(void);
void app_timeLoad(void);
void app_timeSaveTime(void);
void app_timeDisplay500ms(void);
void app_timePower1min(void);
void app_timeAddTime(UINT16 money);
UINT16 app_timeRefundMoney(UINT16 *money);


#endif
