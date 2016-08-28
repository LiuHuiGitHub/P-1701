#include <STC15F2K60S2.H>
#include "drv_fm1702_itf.h"

sbit RC500RST   = P5^5;
sbit RC500SI    = P1^5;
sbit RC500CS    = P1^6;
sbit RC500SO    = P1^7;
sbit RC500SCK   = P5^4;

static UINT8 drv_spiReadWriteByte(UINT8 wdata);
static void drv_spiWriteByte(UINT8 wdata);

void drv_fm1702ItfClearReset(void)
{
	RC500RST = 0;
}

void drv_fm1702ItfSetReset(void)
{
    /* Init Io */
	RC500RST = 1;
	RC500SCK = 0;
	RC500SO = 1;
	RC500SI = 1;
    RC500CS = 0;
}

static UINT8 drv_spiReadWriteByte(UINT8 wdata)
{
	UINT8 rdata;
    /* 0 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    rdata <<= 1;
    if (RC500SI)
    {
        rdata++;
    }
    RC500SCK = 0;
    /* 1 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    rdata <<= 1;
    if (RC500SI)
    {
        rdata++;
    }
    RC500SCK = 0;
    /* 2 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    rdata <<= 1;
    if (RC500SI)
    {
        rdata++;
    }
    RC500SCK = 0;
    /* 3 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    rdata <<= 1;
    if (RC500SI)
    {
        rdata++;
    }
    RC500SCK = 0;
    /* 4 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    rdata <<= 1;
    if (RC500SI)
    {
        rdata++;
    }
    RC500SCK = 0;
    /* 5 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    rdata <<= 1;
    if (RC500SI)
    {
        rdata++;
    }
    RC500SCK = 0;
    /* 6 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    rdata <<= 1;
    if (RC500SI)
    {
        rdata++;
    }
    RC500SCK = 0;
    /* 7 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    rdata <<= 1;
    if (RC500SI)
    {
        rdata++;
    }
    RC500SCK = 0;
    return rdata;
}

static void drv_spiWriteByte(UINT8 wdata)
{
    /* 0 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    RC500SCK = 0;
    /* 1 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    RC500SCK = 0;
    /* 2 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    RC500SCK = 0;
    /* 3 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    RC500SCK = 0;
    /* 4 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    RC500SCK = 0;
    /* 5 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    RC500SCK = 0;
    /* 6 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    RC500SCK = 0;
    /* 7 */
    wdata <<= 1;
    RC500SO = CY;
    RC500SCK = 1;
    RC500SCK = 0;
}
//=======================================================
//	���ƣ�	drv_fm1702ItfReadByte
//	���ܣ�	SPI��ȡ����
//	���:
//		addr:	Ҫ��ȡ��FM1702SL�ڵļĴ�����ַ[0x01~0x3f]
//	����:
//	˵����Ϊ������ٶȣ��Ե�ַ����Ч�Բ������
//========================================================
UINT8 drv_fm1702ItfReadByte(UINT8 addr)
{
    UINT8 rdata;
    
    RC500CS = 0;
	
    addr <<= 1;
    addr |= 0x80;
	
	drv_spiWriteByte(addr);
	rdata = drv_spiReadWriteByte(0x00);
	
    RC500CS = 1;
    
    return (rdata);
}

void drv_fm1702ItfRead(UINT8 addr, UINT8* dat, UINT8 len)
{
    RC500CS = 0;
	
    addr <<= 1;
    addr |= 0x80;
	
	drv_spiReadWriteByte(addr);
	
	while(len--)
	{
		*dat++ = drv_spiReadWriteByte(addr);
	}
	
	RC500CS = 1;
}


//=======================================================
//	���ƣ�drv_fm1702ItfWriteOneByte
//	���ܣ�SPIд�����
//	���:	addr:	Ҫд��FM1702SL�ڵļĴ�����ַ[0x01~0x3f]
//          wData ��  	Ҫд�������
//	����:
//	˵����Ϊ������ٶȣ��Ե�ַ����Ч�Բ������
//========================================================
void drv_fm1702ItfWriteByte(UINT8 addr, UINT8 byte)
{
    RC500CS = 0;
	
    addr <<= 1;
    addr &= 0x7E;

	drv_spiWriteByte(addr);
	
	drv_spiWriteByte(byte);
	
    RC500CS = 1;
}

void drv_fm1702ItfWrite(UINT8 addr, UINT8* dat, UINT8 len)
{
    RC500CS = 0;
	
    addr <<= 1;
    addr &= 0x7E;

	drv_spiWriteByte(addr);
	
	while(len--)
	{
		drv_spiWriteByte(*dat++);
	}
	
    RC500CS = 1;
}
