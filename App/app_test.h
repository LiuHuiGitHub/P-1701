#ifndef __APP_TEST_H__
#define __APP_TEST_H__

#include "typedef.h"

extern BOOL b_testMode;
extern BOOL b_f_faultFlag;

void app_testInit(void);
void app_testHandler1ms(void);
BOOL app_testGetFuseState(void);

#endif