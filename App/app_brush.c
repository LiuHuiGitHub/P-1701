#include "sys.h"
#include "app_brush.h"
#include "app_config.h"
#include "app_time.h"
#include "hwa_eeprom.h"
#include "hwa_fm1702.h"
#include "sys_eeprom.h"
#include "sys_uart.h"
#include "sys_delay.h"
#include "drive_led.h"
#include "drive_key.h"
#include "drive_buzzer.h"
#include "string.h"
#include "drive_relay.h"
#include "app_test.h"

code UINT8 PWD_Card[] = { 0xAC, 0x1E, 0x57, 0xAF, 0x19, 0x4E };	//密码卡密码
data UINT8 LastCardId[5] = {0x00,0x00,0x00,0x00,0x00};

void app_Show(void)
{
	UINT32 Money = s_Money.MoneySum / 10 % 1000000;
	if (b_FactorySystem == FALSE)
	{
		sys_delayms(1000);
        drv_ledDisplayMoney(Money);
		sys_delayms(1000);
	}
}

void app_brushInit(void)
{
	if(hwa_fm1702Init() == FALSE)
    {
        drv_buzzerNumber(4);
    }
}

/*搜索卡，返回卡片类型，
	0->无卡
	1->管理卡
	2->密码卡
	3->用户卡
	4->错误卡
	*/
#define NONE_CARD       0
#define MEM_CARD        1
#define USER_CARD       2
#define PWD_CARD        3
//无卡时寻卡约20ms
UINT8 app_brushCard(void)
{
	UINT8 Sector;
	UINT8 CardIndex;
    UINT8 i;
    
    drv_fm1702Init();
    
	for (CardIndex = MEM_CARD; CardIndex <= PWD_CARD; CardIndex++)
	{
		if (b_FactorySystem)
		{
			CardIndex = PWD_CARD;
		}
		if (CardIndex == MEM_CARD)
		{
			drv_fm1702LoadKey(&s_System.MGM_Card);
		}
		else if (CardIndex == USER_CARD)
		{
			drv_fm1702LoadKey(&s_System.USER_Card);
		}
		else if (CardIndex == PWD_CARD)
		{
			drv_fm1702LoadKey(PWD_Card);
		}
        drv_fm1702HaltCard();
		if (drv_fm1702Request(RF_CMD_REQUEST_STD) == FM1702_OK)
		{
            for(i=0; i<2; i++)
            {
                if (drv_fm1702AntiColl() == FM1702_OK && drv_fm1702SelectCard() == FM1702_OK)
                {
                    if (CardIndex == USER_CARD)     //用户卡验证钱所在扇区
                    {
                        Sector = s_System.Sector;
                    }
                    else                            //管理和密码卡验证1扇区
                    {
                        Sector = 1;
                    }
                    if (drv_fm1702Auth(gCard_UID, Sector, 0x60) == FM1702_OK)
                    {
                        return CardIndex;
                    }
                }
            }
		}
	}
    
	return NONE_CARD;
}

UINT8 testBuff[16];

void app_brushMemSetting(void)
{
	BOOL flag = FALSE;
    #define U8_FIRST_BRUSH_CARD_DLY     6
	UINT8 u8_FirstBrushCardDly = 0;			//第一次刷管理卡显示信息，再次刷则更改
    
    app_timeClear();          //清除时间
    
    #if 0
    memset(testBuff, 0x00, 16);       //改为时间管理卡
    testBuff[0] = 0xFF;
    testBuff[1] = 0xFF;
    hwa_fm1702WriteBlock(testBuff, 4, FALSE);
    return;
    #endif
    
	do
	{
		if (app_brushCard() == MEM_CARD && hwa_fm1702ReadBlock(testBuff, 4))
		{
			if (testBuff[0] == 0x01 && testBuff[1] == 0x0A)			    //金额管理卡
			{
				if (u8_FirstBrushCardDly)
				{
					s_System.Money += 10;
					if (s_System.Money > 200)
					{
						s_System.Money = 10;
					}
					flag = TRUE;
				}
                drv_ledSetMoney(s_System.Money);
			}
			else if (testBuff[0] == 0xFA && testBuff[1] == 0x01)		//时间管理卡
			{
				if (u8_FirstBrushCardDly)
				{
					s_System.Time += 30;
					if (s_System.Time > 600)
					{
						s_System.Time = 30;
					}
					flag = TRUE;
				}
                drv_ledSetTime(s_System.Time);
			}
            else if (testBuff[0] == 0xFF && testBuff[1] == 0x00)		//脉冲数量
			{
				if (u8_FirstBrushCardDly)
				{
					s_System.PulseCount += 1;
					if (s_System.PulseCount > 100)
					{
						s_System.PulseCount = 1;
					}
					flag = TRUE;
				}
                drv_ledSetPulse(s_System.PulseCount);
			}
            else if (testBuff[0] == 0xFF && testBuff[1] == 0xFF)		//测试卡
			{
                app_testGetFuseState();
                u16_DisplayTime = 100;
                u8_FirstBrushCardDly = U8_FIRST_BRUSH_CARD_DLY;
			}
            else
            {
            }
			drv_buzzerNumber(1);
			u8_FirstBrushCardDly = 1;
		}
		sys_delayms(500);
		u8_FirstBrushCardDly++;
	} while (u8_FirstBrushCardDly < U8_FIRST_BRUSH_CARD_DLY);

	if (flag)
	{
		app_configWrite(SYSTEM_SETTING_SECTOR);
	}
}

void app_brushCycle500ms(void)
{
    static BOOL b_oldFaultFlag = FALSE;
	UINT16 Money;
#define BRUSH_SEL_CHANNEL_TIME             20
	static UINT8 u8_BrushSelChannelTime = 0;
    static UINT8 u8_PowerOnCounter = 6;
    static BOOL b_ReadConfigFlag = FALSE;
    
    if(u8_BrushSelChannelTime)
    {
        u8_BrushSelChannelTime--;
    }
    
    if(u8_PowerOnCounter)
    {
        u8_PowerOnCounter--;
    }
    
    switch (app_brushCard())
    {
    case MEM_CARD:
        app_brushMemSetting();
        break;

    case PWD_CARD:										//从初始卡中读取管理卡密码，并储存至E2
        if(u8_PowerOnCounter)
        {
            b_ReadConfigFlag = TRUE;
            gBuff[0] = s_System.MGM_Card[0];
            gBuff[1] = s_System.MGM_Card[1];
            gBuff[2] = s_System.MGM_Card[2];
            gBuff[3] = s_System.MGM_Card[3];
            gBuff[4] = s_System.MGM_Card[4];
            gBuff[5] = s_System.MGM_Card[5];
            gBuff[6] = s_System.Sector;
            gBuff[7] = 0;
            gBuff[8] = 0;
            gBuff[9] = 0;
            gBuff[10] = s_System.USER_Card[0];
            gBuff[11] = s_System.USER_Card[1];
            gBuff[12] = s_System.USER_Card[2];
            gBuff[13] = s_System.USER_Card[3];
            gBuff[14] = s_System.USER_Card[4];
            gBuff[15] = s_System.USER_Card[5];
            if(hwa_fm1702WriteBlock(gBuff, 4, FALSE))
            {
                gBuff[0]  = s_System.RecoveryOldCard;
                gBuff[1]  = 0;
                gBuff[2]  = s_System.Refund;
                gBuff[3]  = 0;
                gBuff[4]  = 0;
                gBuff[5]  = s_System.Money>>8;
                gBuff[6]  = s_System.Money;
                gBuff[7]  = s_System.Time>>8;
                gBuff[8]  = s_System.Time;
                gBuff[9]  = 0;
                gBuff[10] = 0;
                gBuff[11] = s_System.PulseCount>>8;
                gBuff[12] = s_System.PulseCount;
                gBuff[13] = 0;
                gBuff[14] = 0;
                gBuff[15] = 0;
                if(hwa_fm1702WriteBlock(gBuff, 5, FALSE))
                {
                    u8_PowerOnCounter = 0;
                    drv_buzzerOn(50);
                }
            }
        }
        else if (!b_ReadConfigFlag && hwa_fm1702ReadBlock(gBuff, 4))			//读取管理卡和用户卡密码以及扇区
        {
            if (hwa_fm1702ReadBlock(gBuff, 4) == FALSE)         //读取管理卡和用户卡密码以及扇区
            {
                break;
            }
            s_System.MGM_Card[0] = gBuff[0];
            s_System.MGM_Card[1] = gBuff[1];
            s_System.MGM_Card[2] = gBuff[2];
            s_System.MGM_Card[3] = gBuff[3];
            s_System.MGM_Card[4] = gBuff[4];
            s_System.MGM_Card[5] = gBuff[5];
            s_System.Sector      = gBuff[6];
            s_System.USER_Card[0] = gBuff[10];
            s_System.USER_Card[1] = gBuff[11];
            s_System.USER_Card[2] = gBuff[12];
            s_System.USER_Card[3] = gBuff[13];
            s_System.USER_Card[4] = gBuff[14];
            s_System.USER_Card[5] = gBuff[15];
            if (hwa_fm1702ReadBlock(gBuff, 5) == FALSE)			//读取管理卡和用户卡密码以及扇区
            {
                break;
            }
            s_System.RecoveryOldCard = gBuff[0];
//            s_System.Refund          = gBuff[2];
            s_System.Money           = (UINT16)gBuff[5]<<8|gBuff[6];
            s_System.Time            = (UINT16)gBuff[7]<<8|gBuff[8];
            s_System.PulseCount      = (UINT16)gBuff[11]<<8|gBuff[12];
            s_System.Refund++;
            s_System.Refund &= 0x01;
            b_FactorySystem = FALSE;
            app_configWrite(SYSTEM_SETTING_SECTOR);
            app_timeClear();              //清除时间
            drv_ledDispalyVlaue(s_System.Sector*100+s_System.Refund*10+s_System.RecoveryOldCard);
            drv_buzzerNumber(1);
            sys_delayms(1000);
            sys_uartSendData((UINT8*)&s_System, 32);
        }
        break;

    case USER_CARD:
        
    #if 0
        memset(gBuff, 0x00, sizeof(gBuff));
        pMoney->money = 20000;										//充钱
        if (hwa_fm1702WriteSector(gBuff, s_System.Sector))
        {
            drv_buzzerNumber(1);
        }
        break;
    #endif
    
        if(b_oldFaultFlag != b_f_faultFlag)
        {
            b_oldFaultFlag = b_f_faultFlag;
            if(b_f_faultFlag)
            {
                app_timeClear();    //故障，清除时间
            }
        }

        if (!b_f_faultFlag && hwa_fm1702ReadSector(gBuff, s_System.Sector))// 继电器或采样电阻故障
        {            
            if (s_System.Refund == FALSE
                || u16_DisplayTime == 0
                || u8_BrushSelChannelTime != 0
                )     //不返款型刷卡
            {
                if (u16_DisplayTime && memcmp(s_Money.Card_ID, gCard_UID, 5))	//与上次扣款卡号不一致，不允许刷卡
                {
                    break;
                }
                if(u16_DisplayTime == 0)
                {
                    if(app_testGetFuseState())
                    {
                        drv_buzzerNumber(4);
                        break;
                    }
                }
                if (pMoney->money >= s_System.Money)						//确保余额充足
                {
                    pMoney->money -= s_System.Money;
                    if (hwa_fm1702WriteSector(gBuff, s_System.Sector))
                    {
                        memcpy(s_Money.Card_ID, gCard_UID, 5);
                        s_Money.MoneySum += s_System.Money;					//累计营业额
                        
                        app_timeAddTime(s_System.Money);
                        app_configWrite(MONEY_SECTOR);				        //保存卡号和营业额
                        drv_buzzerNumber(1);
                        drv_ledDisplayMoney(pMoney->money);                 //显示剩余金额
                        sys_delayms(2000);
                        
                        u8_BrushSelChannelTime = BRUSH_SEL_CHANNEL_TIME;
                        break;
                    }
                    break;
                }
                else
                {
                    drv_buzzerNumber(3);
                    break;
                }
            }
            else if(s_System.Refund == TRUE)                        //返款
            {
                Money = app_timeRefundMoney(&pMoney->money);
                if (hwa_fm1702WriteSector(gBuff, s_System.Sector))
                {
                    app_timeClear();
                    if (s_Money.MoneySum > Money)
                    {
                        s_Money.MoneySum -= Money;           //累计营业额
                    }
					app_configWrite(MONEY_SECTOR);				//保存营业额
                    drv_buzzerNumber(1);
                    drv_ledDisplayMoney(pMoney->money);		//显示余额
                    sys_delayms(2000);
                }
                break;
            }
        }
        break;

    default:
        break;
    }
    
    drv_fm1702EnterHardPowerDown();
}

