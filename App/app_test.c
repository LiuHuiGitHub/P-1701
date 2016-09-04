#include "stc15f2k60s2.h"
#include "app_test.h"
#include "sys_delay.h"
#include "drive_relay.h"
#include "typedef.h"

BOOL b_testMode = FALSE;
sbit b_input = P2^6;
BOOL b_lastInput = FALSE;
UINT8 u8_timeCount = 0;

BOOL b_f_faultFlag = FALSE;

UINT8 u8_testBuff = 0x00;

void app_testInit(void)
{
	P2M1 |= 0x40;      //ธ฿ื่
	P2M0 &= ~0x40;
    b_input = 1;
}

BOOL app_testGetFuseFaultState(void)
{
    UINT8 i = 100;
    b_f_faultFlag = FALSE;
    drv_relayOpen();
    sys_delayms(20);
    while(i--)
    {
        if(b_input)     
        {
            b_f_faultFlag = TRUE;
            break;
        }
        sys_delayms(1);
    }
    return b_f_faultFlag;
}