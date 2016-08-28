#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "typedef.h"

/*define*/
#define SET_NOREFUND			(0x00)
#define SET_REFUND				(0x01)

#define SET_FUN_TIME			(0x00)
#define SET_FUN_POWER			(0x01)


typedef struct
{
	UINT8 MGM_Card[6];
	UINT8 Sector;                   //Ǯ��������
	UINT8 Reserved[3];
	UINT8 USER_Card[6];
	UINT8 Function;
	UINT8 Refund;
    UINT8 IMaxValue;				//0~9	->0~4A
    UINT8 OverCurrentMultiple;		//1~9    ->����ʱ�������ۼӻ��������ı���1~9��
    UINT8 RecoveryOldCard;          //1->���ݾɿ�
	UINT16 Money;                  //ˢ���۳�����λ��
	UINT16 Time;		            //ˢ���ۿ�һ�μӵĵ���ʱ��
    UINT16 BaseCurrent;				//�����������ã�ADֵС�ڵ��ڸ�ֵʱ����ֵ���㣬���ڸ�ֵʱ�ɱ�������
    UINT16 PulseCount;
} SYSTEM_STRUCT;

extern BOOL b_FactorySystem;
extern data SYSTEM_STRUCT s_System;
extern code SYSTEM_STRUCT s_FactorySystem;

typedef struct
{
    UINT8 Card_ID[5];
    UINT32 MoneySum;               //Ӫҵ����������
} MONEY_STRUCT;

extern data MONEY_STRUCT s_Money;


#define SYSTEM_SETTING_SECTOR               0
#define MONEY_SECTOR                        1


void app_configInit(void);
BOOL app_configRead(UINT8 Sector);
void app_configWrite(UINT8 Sector);
void app_confingHandler1s(void);

#endif

