#include "app_config.h"
#include "app_time.h"
#include "sys_eeprom.h"
#include "hwa_eeprom.h"
#include "drive_buzzer.h"
#include "drive_led.h"
#include "string.h"
#include "intrins.h"

/*
typedef struct
{
UINT8 MGM_Card[6];
UINT8 Sector;                   //钱所在扇区
UINT8 Reserved[3];
UINT8 USER_Card[6];
UINT8 Refund;
UINT8 Function;
UINT8 IMaxValue;				//0~9	->0~4A
UINT8 OverCurrentMultiple;		//1~9    ->过流时，电量累加基本电流的倍数1~9倍
UINT8 RecoveryOldCard;          //1->兼容旧卡
UINT16 Money;                  //刷卡扣除金额，单位分
UINT16 Time;		            //刷卡扣款一次加的电量时间
UINT16 BaseCurrent;				//基本电流设置，AD值小于等于该值时按该值计算，大于该值时成倍计量，
} SYSTEM_STRUCT;
*/
data MONEY_STRUCT s_Money;
data SYSTEM_STRUCT s_System;

BOOL b_FactorySystem = FALSE;       //恢复出厂设置标志


void app_configInit(void)
{
	hwa_eepromInit();
	if (app_configRead(SYSTEM_SETTING_SECTOR) == FALSE
		|| (s_System.Refund > 1)
		|| (s_System.Sector > 15 || s_System.Sector < 1)
		|| (s_System.RecoveryOldCard > 1)
		|| (s_System.Money < 10 || s_System.Money > 200)
		|| (s_System.Time< 30 || s_System.Time > 600)
		|| (s_System.PulseCount < 1 || s_System.PulseCount > 100)
		)
	{
        drv_ledDispalyVlaue(8888);
		b_FactorySystem = TRUE;
	}

	if (app_configRead(MONEY_SECTOR) == FALSE)
	{
		memset(&s_Money, 0x00, sizeof(s_Money));
		app_configWrite(MONEY_SECTOR);
	}
}

BOOL app_configRead(UINT8 Sector)
{
	BOOL b_errorFlag;
	if (Sector == SYSTEM_SETTING_SECTOR)
	{
		b_errorFlag = hwa_eepromReadSector((UINT8*)&s_System, SYSTEM_SETTING_SECTOR);
	}
	else
	{
		b_errorFlag = hwa_eepromReadSector((UINT8*)&s_Money, MONEY_SECTOR);
	}
	return b_errorFlag;
}

void app_configWrite(UINT8 Sector)
{
	if (Sector == SYSTEM_SETTING_SECTOR)
	{
		hwa_eepromWriteSector((UINT8*)&s_System, SYSTEM_SETTING_SECTOR);
	}
	else
	{
		hwa_eepromWriteSector((UINT8*)&s_Money, MONEY_SECTOR);
	}
}

