#include <STC15F2K60S2.H>
#include "drive_key.h"
#include "sys_delay.h"


UINT8 drive_keyGetKey(void)
{
	UINT8 channel;
	if (P33 == 0)
	{
        sys_delayms(10);
        if(P33 == 0)
        {
            channel = 0;
        }
	}
	else if (P32 == 0)
	{
        sys_delayms(10);
        if(P32 == 0)
        {
            channel = 1;
        }
	}
	else
	{
		channel = KEY_NUMBER;
	}
	return channel;
}
