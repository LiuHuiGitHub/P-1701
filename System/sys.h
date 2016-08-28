#ifndef __SYS_H__
#define __SYS_H__

#include "typedef.h"
#include "sys_conf.h"
#include "sys_cpu.h"

typedef enum
{
    TASK_DEL,
    TASK_PEND,
    TASK_WAIT_RUN,
    TASK_WAIT_EVENT,
    TASK_RUN,
    
    TASK_CYCLE,
    TASK_HANDLER,
    TASK_ONCE,
} TASK_ENUM;

typedef struct
{
	void (*ptask)(void);
	SYS_UINT dly;
	SYS_UINT period;
	TASK_ENUM state;
} STASK_STRUCT;

/*sys_core.c*/
void sys_taskInit(void);
void sys_taskCreate(void(*ptask)(void), SYS_UINT period, TASK_ENUM type);
void sys_taskScheduler(void);
void sys_taskStart(void);
void sys_taskDel(void(*ptask)(void));

/*sys_cpu.c*/
void sys_timeInit(void);
UINT16 sys_timeTick(void);
#ifdef SYS_TASK_WATCHDOG
void sys_taskWatchDogInit(void);
void sys_taskWatchDog(void);
#endif
#ifdef SYS_TASK_SLEEP
void sys_taskSleep(void);
#endif

#endif