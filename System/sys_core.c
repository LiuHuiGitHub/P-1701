#include "sys.h"
#include "string.h"

STASK_STRUCT st_Task[MAX_TASK];

SYS_UINT8 u8_UsetTaskNum = 0;
SYS_BOOL b_TaskStart = FALSE;

void sys_taskInit(void)
{
#ifdef SYS_TASK_WATCHDOG
	sys_taskWatchDogInit();
#endif
	memset(st_Task, 0x00, sizeof(st_Task));
	sys_timeInit();
}

void sys_taskCreate(void(*ptask)(void), SYS_UINT period, TASK_ENUM type)
{
	SYS_UINT8 taskId;
	DISABLE_INTERRUPT();
	for (taskId = 0; taskId < MAX_TASK; taskId++)
	{
		if (st_Task[taskId].ptask == SYS_NULL)
		{
			if (u8_UsetTaskNum <= taskId)
			{
				u8_UsetTaskNum = taskId + 1;
			}
			st_Task[taskId].ptask = ptask;
			st_Task[taskId].dly = period + 1;
			st_Task[taskId].period = period;
			st_Task[taskId].state = type;
			break;
		}
	}
	if (b_TaskStart)
	{
		ENABLE_INTERRUPT();
	}
}

void sys_taskScheduler(void)
{
	SYS_UINT8 TaskId;
	TASK_ENUM TaskState;
	for (TaskId = 0; TaskId < u8_UsetTaskNum; TaskId++)
	{
		TaskState = st_Task[TaskId].state;
		if (TaskState == TASK_HANDLER					//中断式任务
			|| TaskState == TASK_CYCLE)				//周期性任务
		{
			if (st_Task[TaskId].dly)
			{
				st_Task[TaskId].dly--;
				if (st_Task[TaskId].dly == 0)
				{
					st_Task[TaskId].dly = st_Task[TaskId].period;
					if (TaskState == TASK_HANDLER)
					{
						(*st_Task[TaskId].ptask)();
					}
					else
					{
						st_Task[TaskId].state = TASK_RUN;
					}
				}
			}
		}
	}
}

void sys_taskStart(void)
{
	SYS_UINT8 TaskId;
	b_TaskStart = TRUE;
	ENABLE_INTERRUPT();
	while (1)
	{
		for (TaskId = 0; TaskId < u8_UsetTaskNum; TaskId++)
		{
			if (st_Task[TaskId].state == TASK_RUN
				|| st_Task[TaskId].state == TASK_ONCE
				&& st_Task[TaskId].ptask != SYS_NULL)
			{
				if (st_Task[TaskId].state == TASK_RUN)
				{
					st_Task[TaskId].state = TASK_CYCLE;
					(*st_Task[TaskId].ptask)();
				}
				else
				{
					st_Task[TaskId].state = TASK_DEL;
					(*st_Task[TaskId].ptask)();
				}
			}
		}
#ifdef SYS_TASK_WATCHDOG
		sys_taskWatchDog();
#endif
#ifdef SYS_TASK_SLEEP
		sys_taskSleep();
#endif
	}
}

void sys_taskDel(void(*ptask)(void))
{
	SYS_UINT8 TaskId;
	DISABLE_INTERRUPT();
	for (TaskId = 0; TaskId < u8_UsetTaskNum; TaskId++)
	{
		if (st_Task[TaskId].ptask == ptask)
		{
			if (TaskId == u8_UsetTaskNum - 1)
			{
				u8_UsetTaskNum--;
			}
			st_Task[TaskId].ptask = SYS_NULL;
			st_Task[TaskId].state = TASK_DEL;
			break;
		}
	}
	if (b_TaskStart)
	{
		ENABLE_INTERRUPT();
	}
}

