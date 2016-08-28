#include "STC15F2K60S2.h"
#include <STC15F2K60S2.H>
#include "typedef.h"
#include "sys_uart.h"
#include "string.h"

#define FOSC 22118400          //ÏµÍ³ÆµÂÊ
#define BAUD 115200

void sys_uartInit(void)
{
    SCON = 0x50;
    T2L = (65536 - (FOSC/4/BAUD));
    T2H = (65536 - (FOSC/4/BAUD))>>8;
    AUXR |= 0x14;
    AUXR |= 0x01;
    ES = 1;
    //EA = 1;
}

UINT8 u8_BoudOverTime = 0;
UINT8 u8_RxIndex = 0;
UINT8 RxBuff[RX_BUFF_LEN];


void sys_uartInterrupt() interrupt 4
{
    if(RI)
    {
        RI = 0;
        RxBuff[u8_RxIndex] = SBUF;
        u8_BoudOverTime = BOUD_OVER_TIME;
        if(u8_RxIndex < RX_BUFF_LEN)
        {
            u8_RxIndex++;
        }
    }
}

void sys_uartSendData(UINT8 *pData, UINT8 len)
{
    if(len)
    {
        while(len--)
        {
            SBUF = *pData++;
            while(!TI);
            TI = 0;
        }
    }
}

UINT8 sys_uartReadData(UINT8 *pData)
{
	UINT8 len = u8_RxIndex;
    if(u8_BoudOverTime)
	{
		u8_BoudOverTime--;
        if(u8_BoudOverTime == 0)
        {
            memcpy(pData, RxBuff, len);
            memset(RxBuff, 0x00, RX_BUFF_LEN);
            u8_RxIndex = 0;
            u8_BoudOverTime = 0;
            return len;
        }
	}
    return 0;
}

