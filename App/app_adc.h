#ifndef __APP_ADC_H__
#define __APP_ADC_H__

#include "typedef.h"

#define ADC_ONCEING         1024

void app_adcInit(void);
UINT16 app_adcGetValue(UINT8 channel);
void app_adcHandler1ms(void);
void app_adcOnceSetChannel(UINT8 channel);
UINT16 app_adcGetOnceChannelValue(void);
UINT8 app_adcCycleSamplingChannel(void);

#endif
