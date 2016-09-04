#include <STC15F2K60S2.H>
#include "drive_led.h"
#include "sys_delay.h"

BOOL b_OnAllFlag = FALSE;

sbit LED_CS0 = P2 ^ 1;
sbit LED_CS1 = P2 ^ 2;
sbit LED_CS2 = P2 ^ 3;
sbit LED_CS3 = P2 ^ 4;
sbit LED_CS4 = P2 ^ 5;

/*数码管管脚定义
P27~0
g c b f a e d h

*//*Seg Class*/
#define SEG_CLASS       0

//与显示相关变量
/*Seg Port*/
#define SEG_PORT        P3

/*Seg Pin*/
#define SEG_PORT_A      BIT2
#define SEG_PORT_B      BIT3
#define SEG_PORT_C      BIT4
#define SEG_PORT_D      BIT5
#define SEG_PORT_E      BIT6
#define SEG_PORT_F      BIT7
#define SEG_PORT_G      BIT0
#define SEG_PORT_H      BIT1

/*Seg Coding*/
#define SEG_0           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F)
#define SEG_1           (SEG_PORT_B|SEG_PORT_C)
#define SEG_2           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_D|SEG_PORT_E|SEG_PORT_G)
#define SEG_3           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_G)
#define SEG_4           (SEG_PORT_B|SEG_PORT_C|SEG_PORT_F|SEG_PORT_G)
#define SEG_5           (SEG_PORT_A|SEG_PORT_C|SEG_PORT_D|SEG_PORT_F|SEG_PORT_G)
#define SEG_6           (SEG_PORT_A|SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_7           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C)
#define SEG_8           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_9           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_F|SEG_PORT_G)
#define SEG_A           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_C|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_B           (SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_C           (SEG_PORT_A|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F)
#define SEG_D           (SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_G)
#define SEG_E           (SEG_PORT_A|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_F           (SEG_PORT_A|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_P           (SEG_PORT_A|SEG_PORT_B|SEG_PORT_E|SEG_PORT_F|SEG_PORT_G)
#define SEG_U           (SEG_PORT_B|SEG_PORT_C|SEG_PORT_D|SEG_PORT_E|SEG_PORT_F)
#define SEG_LINE        (SEG_PORT_G)
#define SEG_NULL        (~(SEG_8|SEG_PORT_H))
#define SEG_FULL        (SEG_8|SEG_PORT_H)

/**/
#define SEG_ON          (SEG_CLASS==0)
#define SEG_OFF         (SEG_CLASS==1)

code UINT8 ledCoding[] = {  
                            SEG_0,SEG_1,SEG_2,SEG_3,SEG_4,
                            SEG_5,SEG_6,SEG_7,SEG_8,SEG_9,
                            SEG_A,SEG_B,SEG_C,SEG_D,SEG_E,
                            SEG_F,SEG_LINE,SEG_P,SEG_U,SEG_NULL,
                            SEG_FULL
                        };
bdata UINT8 u8_ledDisBuff[5] = { 1, 7, 0, 1 , 0x00}; // P-1701
static data UINT8 u8_ledIndex = 0;//从左到右为0,1,2

sbit sbit_set           = u8_ledDisBuff[4]^7;   //设置
sbit sbit_time          = u8_ledDisBuff[4]^4;   //时间
sbit sbit_money         = u8_ledDisBuff[4]^5;   //金额
sbit sbit_point         = u8_ledDisBuff[4]^6;   //
sbit sbit_pulse         = u8_ledDisBuff[4]^2;   //运行
sbit sbit_error         = u8_ledDisBuff[4]^3;   //错误
sbit sbit_yuan          = u8_ledDisBuff[4]^0;   //元


void drv_ledInit(void)
{
	P2M1 &= ~0x3F;      //推挽输出
	P2M0 |= 0x3F;
	P3M1 &= ~0xFC;      //推挽输出
	P3M0 |= 0xFC;
}

/* LED动态显示 */
void drv_ledHandler1ms(void)
{
	LED_CS0 = SEG_OFF;
	LED_CS1 = SEG_OFF;
	LED_CS2 = SEG_OFF;
	LED_CS3 = SEG_OFF;
	LED_CS4 = SEG_OFF;
    
	u8_ledIndex++;
	u8_ledIndex %= 5;
	if (u8_ledIndex == 0)
	{
        SEG_PORT |= 0xFC;
        SEG_PORT &= ~((ledCoding[u8_ledDisBuff[u8_ledIndex]])&0xFC);
        P20 = (ledCoding[u8_ledDisBuff[u8_ledIndex]]&SEG_PORT_G)?0:1;
		LED_CS0 = SEG_ON;
	}
	else if (u8_ledIndex == 1)
	{
        SEG_PORT |= 0xFC;
        SEG_PORT &= ~((ledCoding[u8_ledDisBuff[u8_ledIndex]])&0xFC);
        P20 = (ledCoding[u8_ledDisBuff[u8_ledIndex]]&SEG_PORT_G)?0:1;
		LED_CS1 = SEG_ON;
	}
	else if (u8_ledIndex == 2)
	{
        SEG_PORT |= 0xFC;
        SEG_PORT &= ~((ledCoding[u8_ledDisBuff[u8_ledIndex]])&0xFC);
        P20 = (ledCoding[u8_ledDisBuff[u8_ledIndex]]&SEG_PORT_G)?0:1;
		LED_CS2 = SEG_ON;
	}
	else if (u8_ledIndex == 3)
	{
        SEG_PORT |= 0xFC;
        SEG_PORT &= ~((ledCoding[u8_ledDisBuff[u8_ledIndex]])&0xFC);
        P20 = (ledCoding[u8_ledDisBuff[u8_ledIndex]]&SEG_PORT_G)?0:1;
		LED_CS3 = SEG_ON;
	}
	else if (u8_ledIndex == 4)
	{
        SEG_PORT |= 0xFC;
        SEG_PORT &= ~((u8_ledDisBuff[4])&0xFC);
        P20 = (u8_ledDisBuff[4]&SEG_PORT_G)?0:1;
		LED_CS4 = SEG_ON;
	}
}

void drv_ledDisplayTime(UINT16 time, BOOL flag)
{
    static UINT8 u8_index = 0;
    UINT8 haur, min;
    
    sbit_set    = 0;
    sbit_time   = 0;
    sbit_money  = 0;
    sbit_point  = 0;
    sbit_yuan   = 0;
    
    haur = time/60%60;
    min = time%60;
    u8_ledDisBuff[0] = haur/10;
    u8_ledDisBuff[1] = haur%10;
    u8_ledDisBuff[2] = min/10;
    u8_ledDisBuff[3] = min%10;
    if(time)
    {
        sbit_point  = flag;
        sbit_time     = 1;
    }
    else
    {
        sbit_set    = 0;
        sbit_time   = 0;
        sbit_money  = 0;
        sbit_point  = 0;
        sbit_pulse  = 0;
        sbit_error  = 0;
        sbit_yuan   = 0;
        u8_ledDisBuff[0] = 19;
        u8_ledDisBuff[1] = 19;
        u8_ledDisBuff[2] = 19;
        u8_ledDisBuff[3] = 19;
        u8_ledDisBuff[4] = 0;
        u8_ledDisBuff[u8_index/2] = 16;
        if(++u8_index >= 8)
        {
            u8_index = 0;
        }
    }
}

void drv_ledSetTime(UINT16 time)
{
    drv_ledDisplayTime(time, 1);
    sbit_set    = 1;
}

void drv_ledSetPulse(UINT16 pulse)
{
    drv_ledDispalyVlaue(pulse);
    sbit_set    = 0;
    sbit_time   = 0;
    sbit_money  = 0;
    sbit_point  = 0;
    sbit_yuan   = 0;
    sbit_set    = 1;
    sbit_pulse  = 1;
}

void drv_ledDispalyVlaue(UINT16 value)
{
    u8_ledDisBuff[0] = value/1000;
    u8_ledDisBuff[1] = value/100%10;
    u8_ledDisBuff[2] = value/10%10;
    u8_ledDisBuff[3] = value%10;
}

void drv_ledDisplayMoney(UINT16 money)
{
    drv_ledDispalyVlaue(money/10);
    sbit_set    = 0;
    sbit_time   = 0;
    sbit_money  = 1;
    sbit_point  = 0;
    sbit_yuan   = 1;
}

void drv_ledDisplayPayMoney(UINT16 money)
{
    drv_ledDispalyVlaue(money/10);
    sbit_set    = 0;
    sbit_time   = 0;
    sbit_money  = 1;
    sbit_point  = 0;
    sbit_yuan   = 0;
}

void drv_ledSetMoney(UINT16 money)
{
    drv_ledDisplayMoney(money);
    sbit_set    = 1;
}

void drv_ledRuning(BOOL pulse)
{
    if(!sbit_set && !b_OnAllFlag)
    {
        sbit_pulse = pulse;
    }
}

void drv_ledError(BOOL error)
{
    if(!b_OnAllFlag)
    {
        sbit_error = error;
    }
}

void drv_ledAllOn(void)
{    
    b_OnAllFlag = 1;
    sbit_set    = 1;
    sbit_time   = 1;
    sbit_money  = 1;
    sbit_point  = 1;
    sbit_pulse  = 1;
    sbit_error  = 1;
    sbit_yuan   = 1;
}

void drv_ledAllOff(void)
{    
    b_OnAllFlag = 0;
    sbit_set    = 0;
    sbit_time   = 0;
    sbit_money  = 0;
    sbit_point  = 0;
    sbit_yuan   = 0;
}

