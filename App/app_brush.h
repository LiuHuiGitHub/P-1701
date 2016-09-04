#ifndef __APP_BRUSH_H__
#define __APP_BRUSH_H__

#include "typedef.h"

extern UINT16 u16_pulseCounter;
extern UINT16 u16_pulseCounterTest;

void app_Show(void);
void app_brushInit(void);
void app_brushCycle500ms(void);

#endif

