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
	UINT8 Sector;                   //钱所在扇区
	UINT8 Reserved[3];
	UINT8 USER_Card[6];
	UINT8 Function;
	UINT8 Refund;
    UINT8 IMaxValue;				//0~9	->0~4A
    UINT8 OverCurrentMultiple;		//1~9    ->过流时，电量累加基本电流的倍数1~9倍
    UINT8 RecoveryOldCard;          //1->兼容旧卡
	UINT16 Money;                  //刷卡扣除金额，单位角
	UINT16 Time;		            //刷卡扣款一次加的电量时间
    UINT16 BaseCurrent;				//基本电流设置，AD值小于等于该值时按该值计算，大于该值时成倍计量，
    UINT16 PulseCount;
} SYSTEM_STRUCT;

extern BOOL b_FactorySystem;
extern data SYSTEM_STRUCT s_System;
extern code SYSTEM_STRUCT s_FactorySystem;

typedef struct
{
    UINT8 Card_ID[5];
    UINT32 MoneySum;               //营业额整数部分
} MONEY_STRUCT;

extern data MONEY_STRUCT s_Money;


#define SYSTEM_SETTING_SECTOR               0
#define MONEY_SECTOR                        1


void app_configInit(void);
BOOL app_configRead(UINT8 Sector);
void app_configWrite(UINT8 Sector);
void app_confingHandler1s(void);

#endif

