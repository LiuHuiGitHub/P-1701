#include "sys.h"
#include "stc15f2k60s2.h"
#include "drive_buzzer.h"
#include "drive_relay.h"
#include "drive_led.h"
#include "drive_key.h"
#include "sys_uart.h"
#include "hwa_uart.h"
#include "app_brush.h"
#include "app_adc.h"
#include "app_config.h"
#include "app_time.h"
#include "app_test.h"

UINT8 taskCycleCnt500ms = 0;
UINT8 taskCycleCnt60s = 0;
BOOL taskCycle10msFlag = FALSE;
BOOL taskCycle500msFlag = FALSE;
BOOL b_pulse = FALSE;

void sys_taskInit(void)
{
    drv_buzzerInit();
	drv_relayInit();
	drv_ledInit();
//    hwa_uartInit();
	app_brushInit();
	app_configInit();
	app_timeLoad();
    app_testInit();
    app_testGetFuseFaultState();
}

#define T1MS (65536-FOSC/12/1000) //12T

void sys_timeInit(void)
{
//	DISABLE_INTERRUPT();
	TMOD &= 0xF0;
	TH0 = (UINT8)(T1MS>>8);
	TL0 = (UINT8)T1MS;
	ET0 = 1;
	TR0 = 1;
//	PT1 = 1;
    EA = 1;
}

void sys_tim0Isr(void) interrupt 1      //1ms cycle task
{
    static data UINT8 count = 0;
    static data UINT8 count1 = 0;
    static BOOL b_oldPulse = FALSE;
    drv_ledHandler1ms();
//    hwa_uartHandler1ms();
    if(++count>=10)
    {
        drv_buzzerHandler10ms();
        count = 0;
        taskCycle10msFlag = TRUE;
        if(++count1 >= 50)
        {
            count1 = 0;
            taskCycle500msFlag = TRUE;
        }
    }
    
    if(b_oldPulse != P10)
    {
        b_oldPulse = P10;
        if(b_oldPulse)
        {
            b_pulse = TRUE;
            u16_pulseCounter++;
        }
        drv_ledRuning(b_pulse);
    }
    
    drv_ledError(b_f_faultFlag);
}

void main(void)
{
    sys_taskInit();
    sys_timeInit();
    app_Show();
	drv_buzzerNumber(1);
    while(1)
    {
        WDT_CONTR |= 0x35;          //reset watch dog       max time 2.27s
        if(taskCycle10msFlag)
        {
        	taskCycle10msFlag = FALSE;
//            hwa_uartHandler10ms();
            if(u16_pulseCounter >= s_System.PulseCount)
            {
                u16_pulseCounter = 0;
                taskCycleCnt60s = 0;
                app_timePower1min();
            }
        }
        if(taskCycle500msFlag)
        {
            taskCycle500msFlag = FALSE;
            app_brushCycle500ms();
            app_timeDisplay500ms();
			if(++taskCycleCnt60s>=120)
			{
				taskCycleCnt60s = 0;
                u16_pulseCounter = 0;
                app_timePower1min();
			}
        }
        PCON |= 0x01;
    }
}
