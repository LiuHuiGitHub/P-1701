#ifndef __SYS_ADC_H__
#define __SYS_ADC_H__

#include "typedef.h"

void sys_adcInit(void);
UINT16 sys_adcGetValue(UINT8 ch);

#endif
