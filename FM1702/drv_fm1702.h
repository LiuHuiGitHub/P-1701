#ifndef __DRV_FM1702_H__
#define __DRV_FM1702_H__

#include "typedef.h"
#include "drv_fm1702_itf.h"

//;==============================================
//;FM1702SL���ƼĴ�������
//;==============================================
#define	Page_Reg                0x00	//����0x80  д��0x00
#define	Command_Reg             0x01	//����0x82	д��0x02
#define	FIFO_Reg                0x02	//����0x84	д��0x04
#define	FIFOLength_Reg		    0x04	//����0x88
#define	SecondaryStatus_Reg  	0x05	//����0x8A
#define	InterruptEn_Reg		    0x06	//			д��0x0C
#define	InterruptRq_Reg		    0x07	//����0x8E	д��0x0E
#define	Control_Reg		        0x09	//����0x92	д��0x12 
#define	ErrorFlag_Reg           0x0A	//����0x94
#define	BitFraming_Reg	     	0x0F	//			д��0x1E
#define	TxControl_Reg           0x11	//			д��0x22
#define	CwConductance_Reg	    0x12
#define	RxControl2_Reg		    0x1E
#define RxWait_Reg              0x21
#define	ChannelRedundancy_Reg	0x22

//;==============================================
//;����������붨��
//;==============================================
#define FM1702_OK			0		// ��ȷ
#define FM1702_NOTAGERR		1		// �޿�
#define FM1702_CRCERR		2		// ��ƬCRCУ�����
#define FM1702_EMPTY		3		// ��ֵ�������
#define FM1702_AUTHERR		4		// ��֤���ɹ�
#define FM1702_PARITYERR    5		// ��Ƭ��żУ�����
#define FM1702_CODEERR		6		// ͨѶ����(BCCУ���)
#define FM1702_SERNRERR		8		// ��Ƭ���кŴ���(anti-collision ����)
#define FM1702_SELECTERR    9		// ��Ƭ���ݳ����ֽڴ���(SELECT����)
#define FM1702_NOTAUTHERR	10		// 0x0A ��Ƭû��ͨ����֤
#define FM1702_BITCOUNTERR	11		// �ӿ�Ƭ���յ���λ������
#define FM1702_BYTECOUNTERR	12		// �ӿ�Ƭ���յ����ֽ����������������Ч
#define FM1702_RESTERR		13		// ����restore��������
#define FM1702_TRANSERR		14		// ����transfer��������
#define FM1702_WRITEERR		15		// 0x0F ����write��������
#define FM1702_INCRERR		16		// 0x10 ����increment��������
#define FM1702_DECRERR		17      // 0x11 ����decrement��������
#define FM1702_READERR		18      // 0x12 ����read��������
#define FM1702_LOADKEYERR	19      // 0x13 ����LOADKEY��������
#define FM1702_FRAMINGERR	20      // 0x14 FM1702֡����
#define FM1702_REQERR		21      // 0x15 ����req��������
#define FM1702_SELERR		22      // 0x16 ����sel��������
#define FM1702_ANTICOLLERR	23      // 0x17 ����anticoll��������
#define FM1702_INTIVALERR	24      // 0x18 ���ó�ʼ����������
#define FM1702_READVALERR	25      // 0x19 ���ø߼�����ֵ��������
#define FM1702_DESELECTERR	26      // 0x1A
#define FM1702_CMD_ERR		42      // 0x2A �������

//;==============================================
//;��Ƶ��ͨ�������붨��
//;==============================================
#define RF_CMD_REQUEST_STD	0x26
#define RF_CMD_REQUEST_ALL	0x52
#define RF_CMD_ANTICOL		0x93
#define RF_CMD_SELECT		0x93
#define RF_CMD_AUTH_LA		0x60
#define RF_CMD_AUTH_LB		0x61
#define RF_CMD_READ         0x30
#define RF_CMD_WRITE		0xa0
#define RF_CMD_INC		    0xc1
#define RF_CMD_DEC		    0xc0
#define RF_CMD_RESTORE		0xc2
#define RF_CMD_TRANSFER		0xb0
#define RF_CMD_HALT		    0x50


extern UINT8 gBuff[16];             //M1�����ݿ��ȡ������
extern UINT8 gCard_UID[5];	//4���ֽڿ��ţ�32λ����һ��У���ֽ�

////===============================================
//����ΪFM1702��д���ӳ���
//===============================================
void drv_fm1702EnterHardPowerDown(void);
void drv_fm1702EnterSoftPowerDown(void);
void drv_fm1702ExitSoftPowerDown(void);
BOOL drv_fm1702Init(void);
UINT8 drv_fm1702ReadFifo(UINT8 *buff);
UINT8 drv_fm1702Command(UINT8 Comm_Set, UINT8 *buff, UINT8 count);
UINT8 drv_fm1702Request(UINT8 mode);
UINT8 drv_fm1702AntiColl(void);
UINT8 drv_fm1702SelectCard(void);
UINT8 drv_fm1702LoadKey(UINT8 *ramadr);
UINT8 drv_fm1702Auth(UINT8 *UID, UINT8 SecNR, UINT8 mode);
UINT8 drv_fm1702ReadBlock(UINT8 *buff, UINT8 index);
UINT8 drv_fm1702WriteBlock(UINT8 *buff, UINT8 index);
UINT8 drv_fm1702HaltCard(void);

#endif