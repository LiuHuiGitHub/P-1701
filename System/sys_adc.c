#include "STC15F2K60S2.h"
#include "sys_adc.h"
#include "intrins.h"

#define ADC_POWER   0x80            //ADC电源控制位
#define ADC_FLAG    0x10            //ADC完成标志
#define ADC_START   0x08            //ADC起始控制位
#define ADC_SPEEDLL 0x00            //540个时钟
#define ADC_SPEEDL  0x20            //360个时钟
#define ADC_SPEEDH  0x40            //180个时钟
#define ADC_SPEEDHH 0x60            //90个时钟

/*----------------------------
读取ADC结果
----------------------------*/
UINT16 sys_adcGetValue(UINT8 ch)
{
    UINT16 value;
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
    _nop_();                        //等待4个NOP
    _nop_();
    _nop_();
    _nop_();
    while (!(ADC_CONTR & ADC_FLAG));//等待ADC转换完成
    ADC_CONTR &= ~ADC_FLAG;         //Close ADC
    value = ADC_RES<<2 | ADC_RESL;
    return value;                 	//返回ADC结果
}

/*----------------------------
初始化ADC
----------------------------*/
void sys_adcInit(void)
{
    P1ASF = 0x40;				//设置P16口为AD口
	P1M1 |= 0x40;       		//高阻输入
    P1M0 &= ~0x40;
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL;
}
