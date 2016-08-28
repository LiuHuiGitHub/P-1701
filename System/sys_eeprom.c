#include "STC15F2K60S2.h"
#include "sys_eeprom.h"
#include "intrins.h"


#define CMD_IDLE    0               //����ģʽ
#define CMD_READ    1               //IAP�ֽڶ�����
#define CMD_PROGRAM 2               //IAP�ֽڱ������
#define CMD_ERASE   3               //IAP������������               

#define ENABLE_IAP 0x80           //if SYSCLK<30MHz
//#define ENABLE_IAP 0x81           //if SYSCLK<24MHz
//#define ENABLE_IAP  0x82            //if SYSCLK<20MHz
//#define ENABLE_IAP 0x83           //if SYSCLK<12MHz
//#define ENABLE_IAP 0x84           //if SYSCLK<6MHz
//#define ENABLE_IAP 0x85           //if SYSCLK<3MHz
//#define ENABLE_IAP 0x86           //if SYSCLK<2MHz
//#define ENABLE_IAP 0x87           //if SYSCLK<1MHz

static void IapIdle(void);
/*----------------------------
�ر�IAP
----------------------------*/
void IapIdle(void)
{
    IAP_CONTR = 0;                  //�ر�IAP����
    IAP_CMD = 0;                    //�������Ĵ���
    IAP_TRIG = 0;                   //��������Ĵ���
    IAP_ADDRH = 0x80;               //����ַ���õ���IAP����
    IAP_ADDRL = 0;
}

///*----------------------------
//��ISP/IAP/EEPROM�����ȡһ�ֽ�
//----------------------------*/
//UINT8 IapReadByte(UINT16 addr)
//{
//    UINT8 dat;                       //���ݻ�����

//    IAP_CONTR = ENABLE_IAP;         //ʹ��IAP
//    IAP_CMD = CMD_READ;             //����IAP����
//    IAP_ADDRL = addr;               //����IAP�͵�ַ
//    IAP_ADDRH = addr >> 8;          //����IAP�ߵ�ַ
//    IAP_TRIG = 0x5a;                //д��������(0x5a)
//    IAP_TRIG = 0xa5;                //д��������(0xa5)
//    _nop_();                        //�ȴ�ISP/IAP/EEPROM�������
//    dat = IAP_DATA;                 //��ISP/IAP/EEPROM����
//    IapIdle();                      //�ر�IAP����

//    return dat;                     //����
//}

///*----------------------------
//дһ�ֽ����ݵ�ISP/IAP/EEPROM����
//----------------------------*/
//void IapProgramByte(UINT16 addr, UINT8 dat)
//{
//    IAP_CONTR = ENABLE_IAP;         //ʹ��IAP
//    IAP_CMD = CMD_PROGRAM;          //����IAP����
//    IAP_ADDRL = addr;               //����IAP�͵�ַ
//    IAP_ADDRH = addr >> 8;          //����IAP�ߵ�ַ
//    IAP_DATA = dat;                 //дISP/IAP/EEPROM����
//    IAP_TRIG = 0x5a;                //д��������(0x5a)
//    IAP_TRIG = 0xa5;                //д��������(0xa5)
//    _nop_();                        //�ȴ�ISP/IAP/EEPROM�������
//    IapIdle();
//}

/*----------------------------
��������
----------------------------*/
void sys_eepromEraseSector(UINT16 addr)
{
    IAP_CONTR = ENABLE_IAP;         //ʹ��IAP
    IAP_CMD = CMD_ERASE;            //����IAP����
    IAP_ADDRL = addr;               //����IAP�͵�ַ
    IAP_ADDRH = addr >> 8;          //����IAP�ߵ�ַ
    IAP_TRIG = 0x5a;                //д��������(0x5a)
    IAP_TRIG = 0xa5;                //д��������(0xa5)
    _nop_();                        //�ȴ�ISP/IAP/EEPROM�������
    IapIdle();
}

void sys_eepromRead(UINT16 addr, UINT8 *Data, UINT16 num)
{
    IAP_CONTR = ENABLE_IAP;
    IAP_CMD = CMD_READ;
    while(num--)
    {
        IAP_ADDRL = addr;
        IAP_ADDRH = addr >> 8;
        IAP_TRIG = 0x5a;
        IAP_TRIG = 0xa5;
        addr++;
        *Data++ = IAP_DATA;
    }
    IapIdle();
}

void sys_eepromWrite(UINT16 addr, UINT8 *Data, UINT16 num)
{
    IAP_CONTR = ENABLE_IAP;
    IAP_CMD = CMD_PROGRAM;
    while(num--)
    {
        IAP_ADDRL = addr;
        IAP_ADDRH = addr >> 8;
        IAP_DATA = *Data;
        IAP_TRIG = 0x5a;
        IAP_TRIG = 0xa5;
        addr++;
        Data++;
    }
    IapIdle();
}

