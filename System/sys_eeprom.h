
#ifndef	__SYS_EEPROM_H__
#define	__SYS_EEPROM_H__

#include "typedef.h"

#define   MOVC_ShiftAddress    0x4000

void	sys_eepromEraseSector(UINT16 addr);
void 	sys_eepromRead(UINT16 addr, UINT8 *Data, UINT16 num);
void 	sys_eepromWrite(UINT16 addr, UINT8 *Data, UINT16 num);


#endif