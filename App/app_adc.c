#include <STC15F2K60S2.H>
#include "app_adc.h"
#include "sys_adc.h"
#include "sys_delay.h"
#include "typedef.h"
#include "string.h"

#define SAMPLING_POINT      ((UINT8)93)         //AD开始采样位置 93%		采样数目约210个
#define SAMPLING_POINT_NUM  ((UINT16)3000)      //AD采样点个数
#define SAMPLING_START      ((UINT16)((UINT32)SAMPLING_POINT_NUM*SAMPLING_POINT/100))
#define SAMPLING_END        SAMPLING_POINT_NUM

//#define AD_CHANNEL_NUM      ((UINT8)2)         //AD采样通道数目
#define AD_SAMPLING_NUM     ((UINT16)(SAMPLING_POINT_NUM*(100-SAMPLING_POINT)/100))//AD采样个数 = 采样点个数*（1-采样点位置）

UINT16 ADOnceValue;
UINT16 ADValue[AD_CHANNEL_NUM];   //各通道AD采样值计算结果

UINT16 SamplingTimeCnt = 0;
UINT16 ResultIndex = 0;             //采样结果保存索引
UINT8 SamplingChannel = 0;          //当前周期采样通道

BOOL b_OnceSamplingEndFlag = FALSE;

#define     ADC_MODE_ONCE       0x00
#define     ADC_MODE_ONCEING    0x01
#define     ADC_MODE_CYCLE      0x02
UINT8 adc_mode = ADC_MODE_CYCLE;

sbit PA = P1^7;
//sbit PB = P1^3;
//sbit PC = P1^4;
//sbit PD = P1^5;

static void app_adcSetMuxChannel(UINT8 channel)
{
    switch(channel)
    {
        case 0:PA = 0;break;
        case 1:PA = 1;break;
//        case 0:PA = 0;PB = 1;PC = 0;PD = 0;break;
//        case 1:PA = 1;PB = 0;PC = 0;PD = 0;break;
//        case 2:PA = 0;PB = 0;PC = 0;PD = 0;break;
//        case 3:PA = 1;PB = 0;PC = 1;PD = 0;break;
//        case 4:PA = 1;PB = 1;PC = 1;PD = 0;break;
//        case 5:PA = 0;PB = 1;PC = 0;PD = 1;break;
//        case 6:PA = 1;PB = 0;PC = 0;PD = 1;break;
//        case 7:PA = 0;PB = 0;PC = 0;PD = 1;break;
//        case 8:PA = 1;PB = 0;PC = 1;PD = 1;break;
//        case 9:PA = 1;PB = 1;PC = 1;PD = 1;break;
        default:break;
    }
}

void app_adcInit(void)
{
    P1M1 &= ~0x80;      //P17推挽输出
    P1M0 |= 0x80;
    sys_adcInit();
    memset(ADValue, 0x00, AD_CHANNEL_NUM);
}

UINT32 adValueSum = 0;

void app_adcHandler1ms(void)
{
    if(adc_mode != ADC_MODE_ONCE)
    {
    	if(SamplingTimeCnt == 0)
    	{
	        adValueSum = 0;
	        ResultIndex = 0;
            SamplingChannel %= AD_CHANNEL_NUM;
        	app_adcSetMuxChannel(SamplingChannel);
    	}
	    else if(SamplingTimeCnt >= SAMPLING_END)
	    {
            if(adc_mode == ADC_MODE_ONCEING)
            {
                adc_mode = ADC_MODE_CYCLE;
                ADOnceValue = adValueSum/ResultIndex;
            }
            else
            {
	        	ADValue[SamplingChannel++] = adValueSum/ResultIndex;
            }
	        SamplingTimeCnt = 0;
	        b_OnceSamplingEndFlag = TRUE;
            return;
	    }
	    else if(SamplingTimeCnt >= SAMPLING_START)
	    {
	        adValueSum += sys_adcGetValue(6);
            ResultIndex ++;
	    }
	    SamplingTimeCnt++;
    }
    else
    {
        adc_mode = ADC_MODE_ONCEING;
        SamplingTimeCnt = 0;
    }
}

UINT16 app_adcGetValue(UINT8 channel)
{
    if(ADValue[channel] >= ADC_ONCEING)
    {
        ADValue[channel] = 1023;
    }
    return ADValue[channel];
}

void app_adcOnceSetChannel(UINT8 channel)
{
    SamplingChannel = channel;
    adc_mode = ADC_MODE_ONCE;
}
UINT16 app_adcGetOnceChannelValue(void)
{
    if(adc_mode == ADC_MODE_CYCLE)
    {
        return ADOnceValue;
    }
    return ADC_ONCEING;
}

UINT8 app_adcCycleSamplingChannel(void)
{
	return SamplingChannel;
}

