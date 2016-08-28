#include "sys.h"
#include "sys_cpu.h"

#ifdef SYS_TASK_WATCHDOG
void sys_taskWatchDogInit(void)
{
}
void sys_taskWatchDog(void)
{
    RESET_WATCH_DOG();          //reset watch dog
}
#endif

#ifdef SYS_TASK_SLEEP
void sys_taskSleep(void)
{
    SLEEP_MCU();
}
#endif

//#define T1MS (65536-FOSC/1000)      //1T
#define T1MS (65536-FOSC/12/1000) //12T

void sys_timeInit(void)
{
	DISABLE_INTERRUPT();
	TMOD &= 0xF0;
	TH0 = (UINT8)(T1MS>>8);
	TL0 = (UINT8)T1MS;
	ET0 = 1;
	TR0 = 1;
//	PT1 = 1;
//    EA = 1;
}


void sys_timeHandler(void) interrupt 1
{
	sys_taskScheduler();
}
