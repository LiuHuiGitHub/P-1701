#ifndef __SYS_UART_H__
#define __SYS_UART_H__

#include "typedef.h"

#define BOUD_OVER_TIME		(5)		//5ms
#define RX_BUFF_LEN         (100)


void sys_uartInit(void);
void sys_uartSendData(UINT8 *pData, UINT8 len);
UINT8 sys_uartReadData(UINT8 *pData);

#endif
