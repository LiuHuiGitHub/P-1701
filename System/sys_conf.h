#ifndef __SYS_CONF_H__
#define __SYS_CONF_H__

/*typedef*/
typedef bit							SYS_BOOL;
typedef unsigned char				SYS_UINT8;
typedef unsigned short              SYS_UINT16;
typedef unsigned long				SYS_UINT32;

/*define*/
#define SYS_NULL                    ((void *)0L)

/*config*/
#define MAX_TASK                    (6u)
#define FOSC                        (22118400)

#define SYS_TASK_WATCHDOG
//#define SYS_TASK_SLEEP

#define PERIOD_16BIT
//#define USE_FULL_ASSERT

#ifdef PERIOD_16BIT
#define SYS_UINT				SYS_UINT16
#else
#define SYS_UINT				SYS_UINT8
#endif

#ifdef  USE_FULL_ASSERT
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((SYS_UINT8 *)__FILE__, __LINE__))
void assert_failed(SYS_UINT8 *file, SYS_UINT32 line);
#else
#define assert_param(expr) //((void)0)
#endif /* USE_FULL_ASSERT */

#endif
