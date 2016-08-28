#ifndef __SYS_CPU_H__
#define __SYS_CPU_H__

#include "stc15f2k60s2.h"
#include "intrins.h"


#define DISABLE_INTERRUPT() do{EA=0;}while(0)
#define ENABLE_INTERRUPT()  do{EA=1;}while(0)

#define RESET_WATCH_DOG()   do{WDT_CONTR=0x33;}while(0)
#define RESET_MCU()         do{IAP_CONTR=0x20;}while(0)
#define SLEEP_MCU()         do{PCON|=0x01;}while(0)
#define STOP_MCU()          do{PCON|=0x02;}while(0)

#define NOP()               _nop_();

#endif
