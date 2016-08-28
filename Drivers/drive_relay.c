#include "STC15F2K60S2.h"
#include "drive_relay.h"
#include "sys_delay.h"

sbit relay0 = P2 ^ 7;

void drv_relayInit(void)
{
    relay0 = 0;
    P2M1 &= ~0x80;      //ÍÆÍìÊä³ö
    P2M0 |= 0x80;
}

void drv_relayOpen(void)
{
    relay0 = 1;
}

void drv_relayClose(void)
{
    relay0 = 0;
}

