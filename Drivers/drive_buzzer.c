#include <STC15F2K60S2.H>
#include "drive_buzzer.h"

sbit Buzzer = P1^1;

UINT8 u8_buzzerNumberCount = 0;
UINT8 u8_buzzerCount = 0;
UINT8 u8_buzzerOnTime = 0;
#define BUZZER_ON_TIME         5
#define BUZZER_OFF_TIME        10

void drv_buzzerInit(void)
{
    P1M1 &= ~0x02;      //ÍÆÍìÊä³ö
    P1M0 |= 0x02;
    Buzzer = 0;
}

void drv_buzzerHandler10ms(void)
{
    if(u8_buzzerOnTime)
    {
        u8_buzzerOnTime--;
        u8_buzzerCount = 0;
        Buzzer = 1;
        return;
    }
    else if(u8_buzzerCount)
    {
        u8_buzzerCount--;
        return;
    }
    if(Buzzer)
	{
		Buzzer = 0;
        u8_buzzerCount = BUZZER_OFF_TIME;
	}
	else if(u8_buzzerNumberCount)
	{
		Buzzer = 1;
        u8_buzzerCount = BUZZER_ON_TIME;
		u8_buzzerNumberCount--;
	}
}

void drv_buzzerNumber(UINT8 num)
{
    u8_buzzerNumberCount = num;
}

void drv_buzzerOn(UINT8 time)
{
    u8_buzzerOnTime = time;
}
