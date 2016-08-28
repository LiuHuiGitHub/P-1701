#ifndef __HWA_MIFARE_H__
#define __HWA_MIFARE_H__

#include "typedef.h"

typedef struct
{
    UINT16 money;
    UINT8 crc[2];
    UINT8 Reserved[12];
}MIFATE_STRUCT;

extern data MIFATE_STRUCT const *pMoney;

void hwa_mifareInit(void);
BOOL hwa_mifareReadBlock(UINT8 *buff, UINT8 block);
BOOL hwa_mifareWriteBlock(UINT8 *buff, UINT8 block);
BOOL hwa_mifareReadSector(UINT8 *dat, UINT8 sector);
BOOL hwa_mifareWriteSector(UINT8 *dat, UINT8 sector);

#endif

