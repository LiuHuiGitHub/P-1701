#include "hwa_eeprom_cfg.h"

#include "app_time.h"
#include "app_config.h"

/*config*/
#define SECTOR_NUM              ((UINT8)3)
#define SECTOR_MAX_USE_SIZE		((UINT16)50)
#define SECTOR_SIZE             ((UINT16)512)
#define SECTOR_ADDR             ((UINT16)0x0000)
#define SECTOR_BACKUP_ADDR      ((UINT16)SECTOR_NUM*SECTOR_SIZE)

/*E2 Sector Number*/
code UINT8 c_u8_eepromSectorNum = SECTOR_NUM;

/*E2 read and write buff*/
UINT8 e2_data[SECTOR_MAX_USE_SIZE+2], e2_data_bak[SECTOR_MAX_USE_SIZE+2];


/*E2 Sector Size*/
code EEPROM_CONFIG_STRUCT c_s_eepromConfig[SECTOR_NUM] =
{
    /*sector 0*/    (UINT16)sizeof(s_System), ENABLE,			//system setting
	/*sector 1*/    (UINT16)sizeof(s_Money), ENABLE,            //money save
	/*sector 2*/    (UINT16)sizeof(u16_DisplayTime), ENABLE,    //time save
};

/*E2 Sector Size*/
code UINT16 c_u16_eepromSectorSize = SECTOR_SIZE;

/*E2 Sector Size*/
code UINT16 c_u16_eepromSectorAddr = SECTOR_ADDR;

/*E2 Sector Size*/
code UINT16 c_u16_eepromSectorBakAddr = SECTOR_BACKUP_ADDR;


#define SecretKey       0xA5

void hwa_eepromEncrypt(UINT8 *dat, UINT16 size)        //º”√‹
{
	UINT16 i;
	UINT8 tmp;
	for (i = 0; i<size; i++)
	{
		tmp = dat[i];
		tmp ^= SecretKey;
		tmp = (tmp >> 1) | (tmp << 7);
		tmp ^= SecretKey;
		dat[i] = tmp;
	}
}

void hwa_eepromDecrypt(UINT8 *dat, UINT16 size)        //Ω‚√‹
{
	UINT16 i;
	UINT8 tmp;
	for (i = 0; i<size; i++)
	{
		tmp = dat[i];
		tmp ^= SecretKey;
		tmp = (tmp << 1) | (tmp >> 7);
		tmp ^= SecretKey;
		dat[i] = tmp;
	}
}
