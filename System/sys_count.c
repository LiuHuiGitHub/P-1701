/******************************************************************************
  Copyright (C), 2001-2011, DCN Co., Ltd.
 ******************************************************************************
  File Name     : sys_count.c
  Version       : Initial Draft
  Author        : hui.liu
  Created       : 2015/2/1
  Last Modified :
  Description   :
  Function List :
              sys_countAddCnt
              sys_countGetTime
              sys_countInit
              sys_countResetChannel
  History       :
  1.Date        : 2015/2/1
    Author      : hui.liu
    Modification: Created file
******************************************************************************/
#include "sys.h"
#include "typedef.h"
#ifdef SYS_COUNT

typedef struct _Counter_STRUCT
{
	UINT16 u16_LastCount;
	UINT16 u16_UpdateCnt;
} Counter_STRUCT;

static Counter_STRUCT s_Counter[SYS_COUNT_CHANNEL_NUM] = {0};


void sys_countAddCnt(void)
{
	register UINT8 i;
	for(i = 0; i < SYS_COUNT_CHANNEL_NUM; i++)
	{ s_Counter[i].u16_UpdateCnt++; }
}

void sys_countInit(void)
{
	register UINT8 i;
	for(i = 0; i < SYS_COUNT_CHANNEL_NUM; i++)
	{ s_Counter[i].u16_UpdateCnt = 0; }
}
/*****************************************************************************
 Prototype    : sys_countResetChannel
 Description  : init channel (0<=channle<=SYS_COUNT_CHANNEL_NUM)
 Input        : UINT8 channel
 Output       : None
 Return Value :

  History        :
  1.Date         : 2015/2/1
    Author       : hui.liu
    Modification : Created function
*****************************************************************************/
void sys_countResetChannel(UINT8 channel)
{
	s_Counter[channel].u16_LastCount = GET_TIME_CNT();
	s_Counter[channel].u16_UpdateCnt = 0;
}

/*****************************************************************************
 Prototype    : sys_countGetTime
 Description  : return time (unit us)
 Input        : UINT8 channel
 Output       : None
 Return Value :

  History        :
  1.Date         : 2015/2/1
    Author       : hui.liu
    Modification : Created function
*****************************************************************************/
UINT32 sys_countGetTime(UINT8 channel)
{
	UINT32 temp = GET_TIME_CNT();
	if(s_Counter[channel].u16_UpdateCnt)
	{ temp += s_Counter[channel].u16_UpdateCnt * (COUNT_OVER_TIME + 1); }
	temp -= s_Counter[channel].u16_LastCount;
	sys_countResetChannel(channel);
	return temp;
}
#endif
