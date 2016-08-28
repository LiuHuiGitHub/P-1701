#include "typedef.h"
#include "drv_fm1702_itf.h"
#include "drv_fm1702.h"
#include "sys_delay.h"


////////////////////ȫ�ֱ���////////////////////////////////
UINT8 gBuff[16];
UINT8 gCard_UID[5];

static void drv_fm1702WriteFifo(UINT8 *buff, UINT8 count);
static BOOL drv_fm1702ClearFifo(void);

//;==============================================
//;FM1702SL�����������
//;==============================================
#define CMD_WRITE_E2            0x01
#define CMD_LOAD_KEY_E2         0x0B
#define CMD_LOAD_KEY            0x19
#define CMD_TRANSMIT            0x1A
#define CMD_TRANSCCEIVE         0x1E
#define CMD_AUTHENT_1           0x0C
#define CMD_AUTHENT_2           0x14


//=======================================================
//  ����: drv_fm1702PowerDownMode
//  ����: �ú���ʵ��FM1702����Ӳ���͹���ģʽ��
//      �˳���Ҫ���³�ʼ��
//  ����:N/A
//  ���: N/A
//=======================================================
void drv_fm1702EnterHardPowerDown(void)
{
    drv_fm1702ItfSetReset();
}

//=======================================================
//  ����: drv_fm1702EnterSoftPowerDown
//  ����: �ú���ʵ��FM1702��������͹���ģʽ
//      ��drv_fm1702ExitSoftPowerDown�˳�����512��ʱ������
//  ����:N/A
//  ���: N/A
//=======================================================
void drv_fm1702EnterSoftPowerDown(void)
{
	UINT8 u8_Result;
    u8_Result = drv_fm1702ItfReadByte(Control_Reg);
    u8_Result |= 0x20;
    drv_fm1702ItfWriteByte(Control_Reg, u8_Result);
}

//=======================================================
//  ����: drv_fm1702ExitSoftPowerDown
//  ����: �ú���ʵ��FM1702�˳�����͹���ģʽ
//  ����:N/A
//  ���: N/A
//=======================================================
void drv_fm1702ExitSoftPowerDown(void)
{
	UINT8 u8_Result;
	u8_Result = drv_fm1702ItfReadByte(Page_Reg);
	if(u8_Result == 0x80)
	{
		drv_fm1702Init();
	}
	else
	{
	    u8_Result = drv_fm1702ItfReadByte(Control_Reg);
	    u8_Result &= ~0x30;
	    drv_fm1702ItfWriteByte(Control_Reg, u8_Result);
	}
}

//=======================================================
//  ����: drv_fm1702Init
//  ����: �ú���ʵ�ֶ�FM1702��ʼ������
//  ����:N/A
//  ���: N/A
//=======================================================
BOOL drv_fm1702Init(void)
{
    UINT8 i;
    drv_fm1702ItfSetReset();    // FM1702��λ
    sys_delayus(10);
    drv_fm1702ItfClearReset();
    //���븴λ�׶Σ���Ҫ512��FM1702SL��ʱ�����ڣ�Լ38us
    sys_delayus(50);
    //�����ʼ���׶Σ���Ҫ128��ʱ�����ڣ�Լ10us

    // �ȴ�Command = 0,FM1702��λ�ɹ�
    for (i = 0; i < 250; i++) // ��ʱ
    {
        if(drv_fm1702ItfReadByte(Command_Reg) == 0x00)
        {
            break;
        }
        if(i >= 250)
        {
            return FALSE;
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////
    drv_fm1702ItfWriteByte(Page_Reg, 0x80); //��ʼ��SPI�ӿ�
    for (i = 0; i < 250; i++) // ��ʱ
    {
        if (drv_fm1702ItfReadByte(Command_Reg) == 0x00)   //SPI��ʼ���ɹ�
        {
            drv_fm1702ItfWriteByte(Page_Reg, 0);//����ʹ��SPI�ӿ�
            break;
        }
        if( i >= 250)
        {
            return FALSE;
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////
    drv_fm1702ItfWriteByte(InterruptEn_Reg, 0x7F);  //  ��ֹ�����ж��������λ��0��
    drv_fm1702ItfWriteByte(InterruptRq_Reg, 0x7F);  // ��ֹ�����ж������ʶ��0�����λ��0��

    //���õ�����������ԴΪ�ڲ�������, ��������TX1��TX2
    drv_fm1702ItfWriteByte(TxControl_Reg, 0x5B);        // ���Ϳ��ƼĴ���
    drv_fm1702ItfWriteByte(RxControl2_Reg, 0x01);
    drv_fm1702ItfWriteByte(RxWait_Reg, 5);

    //ѡ��TX1��TX2�ķ��������迹
    drv_fm1702ItfWriteByte(CwConductance_Reg, 0x3F);

    return TRUE;
}

//=======================================================
//  ����: drv_fm1702Request
//  ����:   �ú���ʵ�ֶԷ���FM1702������Χ֮�ڵĿ�Ƭ��Request����
//          ��������M1���ĸ�λ��Ӧ
//  ����:   mode: ALL(�������FM1702������Χ֮�ڵĿ�Ƭ)
//          STD(�����FM1702������Χ֮�ڴ���HALT״̬�Ŀ�Ƭ)
//          Comm_Set, �����룺ָFM1702����IC��������
//  ���:   FM1702_NOTAGERR: �޿�
//          FM1702_OK: Ӧ����ȷ
//          FM1702_REQERR: Ӧ�����
//=======================================================
UINT8 drv_fm1702Request(UINT8 mode)
{
    UINT8 temp;

    //ѡ������У�������ģʽ
    drv_fm1702ItfWriteByte(ChannelRedundancy_Reg, 0x03);
    //��������bit�ĸ�ʽ
    drv_fm1702ItfWriteByte(BitFraming_Reg, 0x07);
    gBuff[0] = mode;        //Requestģʽѡ��
    temp = drv_fm1702ItfReadByte(Control_Reg);
    temp = temp & (0xf7);
    drv_fm1702ItfWriteByte(Control_Reg, temp);        //Control reset value is 00
    temp = drv_fm1702Command(CMD_TRANSCCEIVE, gBuff, 1);   //���ͽ�������
    if (temp == 0)
    {
        return FM1702_NOTAGERR;
    }

    temp = drv_fm1702ReadFifo(gBuff);       //��FIFO�ж�ȡӦ����Ϣ��gBuff��
    // �ж�Ӧ���ź��Ƿ���ȷ
    //2     Mifare Pro ��
    //4     Mifare One ��
    if ((gBuff[0] == 0x04) & (gBuff[1] == 0x0) & (temp == 2))
    {
        return FM1702_OK;
    }
    return FM1702_REQERR;
}

//=======================================================
//���ƣ�drv_fm1702ClearFifo
//���ܣ����FM1702��FIFO
//����:
//���: TRUE:�ɹ�,FALSE:ʧ��
//========================================================
static BOOL drv_fm1702ClearFifo(void)
{
    UINT8 ucResult, i;
    ucResult = drv_fm1702ItfReadByte(Control_Reg);
    ucResult |= 0x01;
    drv_fm1702ItfWriteByte(Control_Reg, ucResult);
    for (i = 0; i < 0xA0; i++)
    {
        ucResult = drv_fm1702ItfReadByte(FIFOLength_Reg);
        if (ucResult == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=======================================================
//���ƣ�drv_fm1702WriteFifo
//���ܣ�������д��FM1702��FIFO
//����: buff��Ҫд���������Ram�е��׵�ַ
//      count:Ҫд����ֽ���
//���:
//========================================================
static void drv_fm1702WriteFifo(UINT8 *buff, UINT8 count)
{
    drv_fm1702ItfWrite(FIFO_Reg, buff, count);
}

//=======================================================
//  ����: drv_fm1702ReadFifo
//  ����: �ú���ʵ�ִ�FM1702��FIFO�ж���x bytes����
//  ����:   buff, ָ��������ݵ�ָ��
//  ���:   ���ݳ��ȣ���λ���ֽڣ�
//=======================================================
UINT8 drv_fm1702ReadFifo(UINT8 *buff)
{
    UINT8 ucResult;

    ucResult = drv_fm1702ItfReadByte(FIFOLength_Reg);
    if (ucResult == 0 || ucResult > 16)
    {
        return 0;
    }
    drv_fm1702ItfRead(FIFO_Reg, buff, ucResult);

    return ucResult;
}

//=======================================================
//  ����: drv_fm1702Command
//  ����: �ú���ʵ����FM1702��������Ĺ���
//  ����:   count, ����������ĳ���
//          buff, ָ����������ݵ�ָ��
//          Comm_Set, �����룺ָFM1702����IC��������
//  ���:   TRUE, �����ȷִ��
//          FALSE, ����ִ�д���
//=======================================================
UINT8 drv_fm1702Command(UINT8 Comm_Set, UINT8 *buff, UINT8 count)
{
    UINT8 ucResult1, ucResult2, i;
    drv_fm1702ItfWriteByte(Command_Reg, 0x00);          //�л���IDLE
    if (drv_fm1702ClearFifo() == FALSE)
    {
        return FALSE;
    }
    drv_fm1702WriteFifo(buff, count);
    drv_fm1702ItfWriteByte(Command_Reg, Comm_Set);
    for (i = 0; i < 0xA0; i++)
    {
        ucResult1 = drv_fm1702ItfReadByte(Command_Reg);
        ucResult2 = drv_fm1702ItfReadByte(InterruptRq_Reg) & 0x80;
        if (ucResult1 == 0 || ucResult2 == 0x80)
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=======================================================
//  ����: drv_fm1702AntiColl
//  ����:   �ú���ʵ�ֶԷ���FM1702������Χ֮�ڵĿ�Ƭ�ķ���ͻ���
//  ����:   ��ȡ����ID(6Byte)
//  ���:   FM1702_NOTAGERR: �޿�
//          FM1702_OK: Ӧ����ȷ
//          FM1702_BYTECOUNTERR: �����ֽڴ���
//          FM1702_SERNRERR: ��Ƭ���к�Ӧ�����
//=======================================================
UINT8 drv_fm1702AntiColl(void)
{
    UINT8 temp, i;

    //ѡ������У�������ģʽ
    drv_fm1702ItfWriteByte(ChannelRedundancy_Reg, 0x03);

    gBuff[0] = RF_CMD_ANTICOL;
    gBuff[1] = 0x20;
    temp = drv_fm1702Command(CMD_TRANSCCEIVE, gBuff, 2);
    if (temp == 0)
    {
        return FM1702_NOTAGERR;
    }

    temp = drv_fm1702ItfReadByte(FIFOLength_Reg);
    if (temp == 0)
    {
        return FM1702_BYTECOUNTERR;
    }

    drv_fm1702ItfRead(FIFO_Reg, gBuff, temp);

    temp = drv_fm1702ItfReadByte(ErrorFlag_Reg);    // �жϽӅ������Ƿ��г�ͻλ
    temp = temp & 0x01;
    if (temp == 0x00)
    {
        for (i = 0; i < 5; i++)
        {
            temp ^= gBuff[i];
        }
        if (temp)
        {
            return FM1702_SERNRERR;
        }
        for (i = 0; i < 5; i++)
        {
            gCard_UID[i] = gBuff[i];
        }
        return FM1702_OK;
    }
    else //�г�ͻλ
    {
        return FM1702_SERNRERR;
    }
}

//=======================================================
//  ����: drv_fm1702SelectCard
//  ����:   �ú���ʵ�ֶԷ���FM1702������Χ֮�ڵ�ĳ�ſ�Ƭ����ѡ��
//  ����:   N/A
//  ���:   FM1702_NOTAGERR: �޿�
//          FM1702_OK: Ӧ����ȷ
//          FM1702_BYTECOUNTERR: �����ֽڴ���
//          FM1702_PARITYERR: ��żУ���
//          FM1702_CRCERR: CRCУ���
//          FM1702_SELERR: ѡ������
//=======================================================
UINT8 drv_fm1702SelectCard(void)
{
    UINT8   temp, i;

    //ѡ������У�������ģʽ
    drv_fm1702ItfWriteByte(ChannelRedundancy_Reg, 0x0F);

    gBuff[0] = RF_CMD_SELECT;
    gBuff[1] = 0x70;
    for (i = 0; i < 5; i++)
    {
        gBuff[i + 2] = gCard_UID[i];
    }
    temp = drv_fm1702Command(CMD_TRANSCCEIVE, gBuff, 7);
    if (temp == 0)
    {
        return FM1702_NOTAGERR;
    }
    else
    {
        temp = drv_fm1702ItfReadByte(ErrorFlag_Reg);
        if (temp & 0x02)
        {
            return FM1702_PARITYERR;
        }
        if (temp & 0x04)
        {
            return FM1702_FRAMINGERR;
        }
        if (temp & 0x08)
        {
            return FM1702_CRCERR;
        }
        temp = drv_fm1702ItfReadByte(FIFOLength_Reg);
        if (temp != 0x01)
        {
            return FM1702_BYTECOUNTERR;
        }

        temp = drv_fm1702ItfReadByte(FIFO_Reg);     //��FIFO�ж�ȡӦ����Ϣ
        if (temp == 0x08)                           // �ж�Ӧ���ź��Ƿ���ȷ
        {
            return FM1702_OK;
        }
        else
        {
            return FM1702_SELERR;
        }
    }
}

UINT8 drv_fm1702LoadKey(UINT8 *ramadr)
{
    UINT8 acktemp, temp, i;
    UINT8 u8_Buff[12];
    for (i = 0; i < 6; i++)
    {
        temp = ramadr[i];
        u8_Buff[2 * i] = (((ramadr[i] & 0xF0) >> 4) | ((~ramadr[i]) & 0xF0));
        u8_Buff[2 * i + 1] = ((temp & 0x0F) | (~(temp & 0x0F) << 4));
    }
    acktemp = drv_fm1702Command(CMD_LOAD_KEY, u8_Buff, 12);
    temp = drv_fm1702ItfReadByte(ErrorFlag_Reg);
    if (temp & 0x40)
    {
        temp = 0x0;
        drv_fm1702ItfWriteByte(Command_Reg, temp);
        return FALSE;
    }
    temp = 0x0;
    drv_fm1702ItfWriteByte(Command_Reg, temp);
    return TRUE;
}
/*
//=======================================================
//  ����: drv_fm1702LoadKeyToE2
//  ����: �ú���ʵ�ְ��������FM1702��E2��
//          ��Կ��Կ��ַ =0x80+n*12
//  ����:   index: ��Կ����(ͨ����Ӧ������0~15��A���룬16~31��B����)
//  ���:   TRUE, ��Կװ�سɹ�
//          FALSE, ��Կװ��ʧ��
//=======================================================
UINT8 drv_fm1702LoadKeyToE2(UINT8 index, UINT8 *ramadr)
{
    UINT8 acktemp, temp, i;
    UINT8 u8_Buff[16];
    UINT16 u16_Data;
    u16_Data = 0x80 + index * 12;
    u8_Buff[0] = (UINT8)(u16_Data & 0x00FF);
    u8_Buff[1] = (UINT8)(u16_Data >> 8);
    for (i = 0; i < 6; i++)
    {
        temp = ramadr[i];
        u8_Buff[2 + i + i] = (((ramadr[i] & 0xF0) >> 4) | ((~ramadr[i]) & 0xF0));
        u8_Buff[3 + i + i] = ((temp & 0x0F) | (~(temp & 0x0F) << 4));
    }
    acktemp = drv_fm1702Command(CMD_WRITE_E2, u8_Buff, 0x0e);
    sys_delayms(4);
    temp = drv_fm1702ItfReadByte(SecondaryStatus_Reg);
    if (temp & 0x40)
    {
        temp = 0x0;
        drv_fm1702ItfWriteByte(Command_Reg, temp);
        return FALSE;
    }
    temp = 0x0;
    drv_fm1702ItfWriteByte(Command_Reg, temp);
    return TRUE;
}

//=======================================================
//  ����: drv_fm1702LoadKeyfromE2
//  ����: �ú���ʵ�ְ�E2���������FM1702��keyRevBuffer��
//  ����:   index: ��Կ����(ͨ����Ӧ������0~15��A���룬16~31��B����)
//  ���:   TRUE, ��Կװ�سɹ�
//          FALSE, ��Կװ��ʧ��
//=======================================================
UINT8 drv_fm1702LoadKeyfromE2(UINT8 index)
{
    UINT8 u8_Data[2], ucResult;
    UINT16 u16_Data;
    if(index >= 32)
    {
        return FALSE;
    }
    u16_Data = 0x80 + index * 12;
    u8_Data[0] = (UINT8)(u16_Data & 0x00FF);
    u8_Data[1] = (UINT8)(u16_Data >> 8);
    ucResult = drv_fm1702Command(CMD_LOAD_KEY_E2, u8_Data, 2);
    if(ucResult == FALSE)
    {
        return FALSE;
    }
    sys_delayms(1);
    ucResult = drv_fm1702ItfReadByte(ErrorFlag_Reg);
    if(ucResult & 0x40)
    {
        return FALSE;
    }
    return TRUE;
}
*/

//=======================================================
//  ����: drv_fm1702Auth
//  ����:   �ú���ʵ��������֤�Ĺ���
//  ����:   UID: ��Ƭ���кŵ�ַ
//          SecNR: ������
//          mode: ģʽ
//  ���:   FM1702_NOTAGERR: �޿�
//          FM1702_OK: Ӧ����ȷ
//          FM1702_PARITYERR: ��żУ���
//          FM1702_CRCERR: CRCУ���
//          FM1702_AUTHERR: Ȩ����֤�д�
//=======================================================
UINT8 drv_fm1702Auth(UINT8 *UID, UINT8 SecNR, UINT8 mode)
{
    UINT8 i;
    UINT8 temp, temp1;

    if (SecNR >= 16)
    {
        SecNR = SecNR % 16;
    }

    //ѡ������У�������ģʽ
    drv_fm1702ItfWriteByte(ChannelRedundancy_Reg, 0x0F);
    gBuff[0] = mode;
    gBuff[1] = SecNR * 4 + 3;
    for (i = 0; i < 4; i++)
    {
        gBuff[2 + i] = UID[i];
    }

    temp = drv_fm1702Command(CMD_AUTHENT_1, gBuff, 6);
    if (temp == 0)
    {
        return 0x99;
    }

    temp = drv_fm1702ItfReadByte(ErrorFlag_Reg);
    if ((temp & 0x02) == 0x02) return FM1702_PARITYERR;
    if ((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
    if ((temp & 0x08) == 0x08) return FM1702_CRCERR;
    temp = drv_fm1702Command(CMD_AUTHENT_2, gBuff, 0);
    if (temp == 0)
    {
        return 0x88;
    }

    temp = drv_fm1702ItfReadByte(ErrorFlag_Reg);

    if ((temp & 0x02) == 0x02)
    {
        return FM1702_PARITYERR;
    }
    if ((temp & 0x04) == 0x04)
    {
        return FM1702_FRAMINGERR;
    }
    if ((temp & 0x08) == 0x08)
    {
        return FM1702_CRCERR;
    }
    temp1 = drv_fm1702ItfReadByte(Control_Reg);
    temp1 = temp1 & 0x08;
    if (temp1 == 0x08)
    {
        return FM1702_OK;
    }

    return FM1702_AUTHERR;
}

//=======================================================
//  ����: drv_fm1702ReadBlock
//  ����:   �ú���ʵ�ֶ�MIFARE�������ֵ
//  ����:   buff: �������׵�ַ
//          index: ���ַ
//  ���:   FM1702_NOTAGERR: �޿�
//          FM1702_OK: Ӧ����ȷ
//          FM1702_PARITYERR: ��żУ���
//          FM1702_CRCERR: CRCУ���
//          FM1702_BYTECOUNTERR: �����ֽڴ���
//=======================================================

UINT8 drv_fm1702ReadBlock(UINT8 *buff, UINT8 index)
{
    UINT8 ucCmdLine[2], ucResult;
    drv_fm1702ItfWriteByte(ChannelRedundancy_Reg, 0x0F);
    ucCmdLine[0] = RF_CMD_READ;
    ucCmdLine[1] = index;
    ucResult = drv_fm1702Command(CMD_TRANSCCEIVE, ucCmdLine, 2);
    if (ucResult == FALSE)
    {
        return FM1702_NOTAGERR;   //�޿�
    }
    ucResult = drv_fm1702ItfReadByte(ErrorFlag_Reg);
    if ((ucResult & 0x02) == 0x02)
    {
        return FM1702_PARITYERR;
    }
    if ((ucResult & 0x04) == 0x04)
    {
        return FM1702_FRAMINGERR;
    }
    if ((ucResult & 0x08) == 0x08)
    {
        return FM1702_CRCERR;
    }
    ucResult = drv_fm1702ReadFifo(buff);
    if (ucResult != 0x10)
    {
        return FM1702_BYTECOUNTERR;
    }
    else
    {
        return FM1702_OK;
    }
}

//=======================================================
//  ����: drv_fm1702WriteBlock
//  ����:   �ú���ʵ��дMIFARE�������ֵ
//  ����:   buff: �������׵�ַ
//          index: ���ַ
//  ���:   FM1702_NOTAGERR: �޿�
//          FM1702_NOTAUTHERR: δ��Ȩ����֤
//          FM1702_EMPTY: �����������
//          FM1702_WRITEERR: д�������ݳ���
//          FM1702_OK: Ӧ����ȷ
//          FM1702_PARITYERR: ��żУ���
//          FM1702_CRCERR: CRCУ���
//          FM1702_BYTECOUNTERR: �����ֽڴ���
//=======================================================
UINT8 drv_fm1702WriteBlock(UINT8 *buff, UINT8 index)
{
    UINT8 ucResult, u8_Data[16];
    drv_fm1702ItfWriteByte(ChannelRedundancy_Reg, 0x07);   /* Note: this line is for 1702, different from RC500*/
    u8_Data[0] = RF_CMD_WRITE;
    u8_Data[1] = index;
    ucResult = drv_fm1702Command(CMD_TRANSCCEIVE, u8_Data, 2);
    if (ucResult == FALSE)
    {
        return FM1702_NOTAGERR;
    }
    ucResult = drv_fm1702ReadFifo(u8_Data);
    if (ucResult == 0)
    {
        return FM1702_BYTECOUNTERR;
    }
    switch (u8_Data[0])
    {
        case 0x00:
            return FM1702_NOTAUTHERR;
        case 0x04:
            return FM1702_EMPTY;
        case 0x0a:
            break;                   //����
        case 0x01:
            return FM1702_CRCERR;
        case 0x05:
            return FM1702_PARITYERR;
        default:
            return FM1702_WRITEERR;
    }
    ucResult = drv_fm1702Command(CMD_TRANSCCEIVE, buff, 16);
    if (ucResult == TRUE)
    {
        return FM1702_OK;
    }
    else
    {
        ucResult = drv_fm1702ItfReadByte(ErrorFlag_Reg);
        if ((ucResult & 0x02) == 0x02)
        {
            return FM1702_PARITYERR;
        }
        else if ((ucResult & 0x04) == 0x04)
        {
            return FM1702_FRAMINGERR;
        }
        else if ((ucResult & 0x08) == 0x08)
        {
            return FM1702_CRCERR;
        }
        else
        {
            return FM1702_WRITEERR;
        }
    }
}

//=======================================================
//  ����: drv_fm1702HaltCard
//  ����: �ú���ʵ����ͣMIFARE��
//  ����:   N/A
//  ���:   FM1702_OK: Ӧ����ȷ
//          FM1702_PARITYERR: ��żУ���
//          FM1702_FRAMINGERR:FM1702֡����
//          FM1702_CRCERR: CRCУ���
//          FM1702_NOTAGERR: �޿�
//=======================================================
UINT8 drv_fm1702HaltCard(void)
{
    UINT8 temp;
    //ѡ������У�������ģʽ
    drv_fm1702ItfWriteByte(ChannelRedundancy_Reg, 0x03);
    *gBuff = RF_CMD_HALT;
    *(gBuff + 1) = 0x00;
    temp = drv_fm1702Command(CMD_TRANSMIT, gBuff, 2);//����FIFO�����ַ
    if (temp == TRUE)
    {
        return FM1702_OK;
    }
    else
    {
        temp = drv_fm1702ItfReadByte(ErrorFlag_Reg);
        if ((temp & 0x02) == 0x02)
        {
            return FM1702_PARITYERR;
        }

        if ((temp & 0x04) == 0x04)
        {
            return FM1702_FRAMINGERR;
        }
        return FM1702_NOTAGERR;
    }
}

