#include "app_time.h"
#include "app_config.h"
#include "drive_relay.h"
#include "drive_led.h"
#include "hwa_eeprom.h"
#include "string.h"

data UINT16 u16_DisplayTime = 100;//ʣ��ʱ�䣬(��ʾʱ���10��)

void app_timeClear(void)
{
    u16_DisplayTime = 0;
    memset(s_Money.Card_ID, 0x00, 5);
    app_timeSaveTime();
    app_configWrite(MONEY_SECTOR);
}

void app_timeLoad(void)
{
    if(hwa_eepromReadSector((UINT8*)&u16_DisplayTime, TIME_SAVE_SECTOR) == FALSE)
    {
        app_timeClear();
    }
    else
    {
        if(u16_DisplayTime > 24*60*10)
        {
            app_timeClear();
        }
    }
}

void app_timeSaveTime(void)
{
    hwa_eepromWriteSector((UINT8*)&u16_DisplayTime, TIME_SAVE_SECTOR);
}

void app_timeDisplay500ms(void)     //ʱ��Ϊ0ʱ��˸��ʾͨ����
{
	static BOOL b_lightFlash = FALSE;
    
    if (b_FactorySystem == FALSE)
	{
        if (u16_DisplayTime == 0)                       //ʱ��Ϊ0��˸��ʾͨ����
        {
            drv_ledDisplayTime(0, 0);
            drv_relayClose();
        }
        else                                //ʱ�䲻Ϊ����ʾʱ��
        {
            b_lightFlash = !b_lightFlash;
            drv_ledDisplayTime(u16_DisplayTime/10, b_lightFlash);
            drv_relayOpen();
        }
	}
}

void app_timeAddTime(UINT16 money)
{
	u16_DisplayTime += (UINT16)((UINT32)money*s_System.Time *10 / s_System.Money);    //TBD
    if(u16_DisplayTime >= (UINT16)24*60*10)     //���24Сʱ
    {
        u16_DisplayTime = (UINT16)24*60*10;
    }
    app_timeSaveTime();
}

/* ���ط�����
*/
UINT16 app_timeRefundMoney(UINT16 *money)
{
	UINT16 Money = (UINT16)((UINT32)u16_DisplayTime * s_System.Money /10 / s_System.Time);
	*money += Money;
    return Money;
}
				   
void app_timePower1min(void)
{
    UINT8 tmp;
    static UINT8 u8_TimeCount10min = 0;
    tmp = 10;
    if(u16_DisplayTime > tmp)
    {
        u16_DisplayTime -= tmp;
    }
    else if(u16_DisplayTime)
    {
        app_timeClear();
    }
    if(++u8_TimeCount10min >= 10)
    {
        u8_TimeCount10min = 0;
        if(u16_DisplayTime)
        {
            app_timeSaveTime();
        }
    }
}
