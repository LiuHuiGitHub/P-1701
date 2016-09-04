#ifndef __DRIVE_LED_H__
#define __DRIVE_LED_H__

#include "typedef.h"

void drv_ledInit(void);
void drv_ledHandler1ms(void);
void drv_ledDisplayTime(UINT16 time, BOOL flag);
void drv_ledSetTime(UINT16 time);
void drv_ledSetPulse(UINT16 pulse);
void drv_ledDispalyVlaue(UINT16 value);
void drv_ledDisplayMoney(UINT16 money);
void drv_ledSetMoney(UINT16 money);
void drv_ledRuning(BOOL pulse);
void drv_ledRuningOff(void);
void drv_ledError(BOOL error);

#endif
