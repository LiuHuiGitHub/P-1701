#include "STC15F2K60S2.h"
#include "sys_adc.h"
#include "intrins.h"

#define ADC_POWER   0x80            //ADC��Դ����λ
#define ADC_FLAG    0x10            //ADC��ɱ�־
#define ADC_START   0x08            //ADC��ʼ����λ
#define ADC_SPEEDLL 0x00            //540��ʱ��
#define ADC_SPEEDL  0x20            //360��ʱ��
#define ADC_SPEEDH  0x40            //180��ʱ��
#define ADC_SPEEDHH 0x60            //90��ʱ��

/*----------------------------
��ȡADC���
----------------------------*/
UINT16 sys_adcGetValue(UINT8 ch)
{
    UINT16 value;
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
    _nop_();                        //�ȴ�4��NOP
    _nop_();
    _nop_();
    _nop_();
    while (!(ADC_CONTR & ADC_FLAG));//�ȴ�ADCת�����
    ADC_CONTR &= ~ADC_FLAG;         //Close ADC
    value = ADC_RES<<2 | ADC_RESL;
    return value;                 	//����ADC���
}

/*----------------------------
��ʼ��ADC
----------------------------*/
void sys_adcInit(void)
{
    P1ASF = 0x40;				//����P16��ΪAD��
	P1M1 |= 0x40;       		//��������
    P1M0 &= ~0x40;
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL;
}
