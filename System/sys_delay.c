#include "sys_delay.h"
#include "stc15f2k60s2.h"


void sys_delayus(UINT16 xus)
{
    while(xus--);
}

void sys_delayms(UINT16 xms)
{   
    UINT8 i, j;
	while(xms--)
    {
    	WDT_CONTR |= 0x35;
        i = 22;
        j = 128;
        do
        {
            while (--j);
        } while (--i);
    }
}