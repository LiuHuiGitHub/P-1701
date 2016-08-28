#include <STC15F2K60S2.H>
#include <intrins.h>
#include "sys_delay.h"
#include "mifare.h"

sbit RC500RST = P5 ^ 5;
sbit RC500SO = P1 ^ 4;
sbit RC500CS = P3 ^ 6;
sbit RC500SI = P3 ^ 5;
sbit RC500SCK = P1 ^ 3;

////////////////////ȫ�ֱ���////////////////////////////////
data UINT8 gBuff[16];		 //������ע�Ͳ�д�ˣ��û������洢�ӿ�������������ݣ�1����2�У�
data UINT8 gCard_UID[5];

//=======================================================
//	���ƣ�	SPIReadOne
//	���ܣ�	SPI��ȡ����
//	���:
//		SpiAddress:	Ҫ��ȡ��FM1702SL�ڵļĴ�����ַ[0x01~0x3f]
//	����: 	
//	˵����Ϊ������ٶȣ��Ե�ַ����Ч�Բ������	
//========================================================
UINT8 SPIReadOne(UINT8 SpiAddress)
{
	UINT8 i, rdata;
	//��ַ�ֽ����λ��1����ʾΪ�����ݣ����λ�̶�Ϊ0���м�6λΪ��ַ
	SpiAddress <<= 1;
	SpiAddress |= 0x80;
	//�����ַ �Ӹ�λ����λ����
	RC500CS = 0;
	for (i = 0; i < 8; i++)
	{
		if (SpiAddress & 0x80)
			RC500SO = 1;
		else
			RC500SO = 0;
		RC500SCK = 1;
		SpiAddress = SpiAddress << 1;
		RC500SCK = 0;
	}

	//��������
	rdata = 0;
	for (i = 0; i < 8; i++)
	{
		RC500SCK = 1;
		rdata = rdata << 1;
		if (RC500SI)
			rdata += 1;
		RC500SCK = 0;
	}
	RC500CS = 1;
	return (rdata);
}

//=======================================================
//	���ƣ�SPIWriteOne
//	���ܣ�SPIд�����
//	���:	SpiAddress:	Ҫд��FM1702SL�ڵļĴ�����ַ[0x01~0x3f]
//          wData ��  	Ҫд�������
//	����: 	
//	˵����Ϊ������ٶȣ��Ե�ַ����Ч�Բ������	
//========================================================
void SPIWriteOne(UINT8 SpiAddress, UINT8 wData)
{
	UINT8 i;
	SpiAddress <<= 1;
	SpiAddress &= 0x7E;

	//�����ַ �Ӹ�λ����λ����
	RC500CS = 0;
	for (i = 0; i < 8; i++)
	{
		if (SpiAddress & 0x80)
			RC500SO = 1;
		else
			RC500SO = 0;
		RC500SCK = 1;
		SpiAddress = SpiAddress << 1;
		RC500SCK = 0;
	}
	//�������ݣ��Ӹ�λ��ʼ
	for (i = 0; i < 8; i++)
	{
		if (wData & 0x80)
			RC500SO = 1;
		else
			RC500SO = 0;
		RC500SCK = 1;
		wData = wData << 1;
		RC500SCK = 0;
	}
	RC500CS = 1;
}

//=======================================================
//	����: Init_FM1702 
//	����: �ú���ʵ�ֶ�FM1702��ʼ������
//	����:N/A
//	���: N/A 
//=======================================================
BOOL Init_FM1702(void)
{
	UINT8 temp;
	UINT16 i = 100;
	RC500SCK = 0;
	RC500SO = 1;
	RC500SI = 1;
	RC500RST = 1;	// FM1702��λ 
	sys_delayms(10);
	RC500RST = 0;
	//���븴λ�׶Σ���Ҫ512��FM1702SL��ʱ�����ڣ�Լ38us
	sys_delayus(200);
	//�����ʼ���׶Σ���Ҫ128��ʱ�����ڣ�Լ10us
	do{ 		// �ȴ�Command = 0,FM1702��λ�ɹ�
		temp = SPIReadOne(Command_Reg);	 //(Command_Reg<<1)|0x80 = 0x82
	} while (temp != 0 && i--);
	if (i == 0)
	{
		return FALSE;
	}
	///////////////////////////////////////////////////////////////////////////////////
	SPIWriteOne(Page_Reg, 0x80);	//��ʼ��SPI�ӿ�
	for (i = 0; i < 0x1fff; i++) // ��ʱ
	{
		temp = SPIReadOne(Command_Reg);
		if (temp == 0x00)	//SPI��ʼ���ɹ�
		{
			SPIWriteOne(Page_Reg, 0);//����ʹ��SPI�ӿ�
			break;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////
	SPIWriteOne(InterruptEn_Reg, 0x7F);	//  ��ֹ�����ж��������λ��0��
	SPIWriteOne(InterruptRq_Reg, 0x7F);	// ��ֹ�����ж������ʶ��0�����λ��0��

	//���õ�����������ԴΪ�ڲ�������, ��������TX1��TX2
	SPIWriteOne(TxControl_Reg, 0x5B); 		// ���Ϳ��ƼĴ��� 
	SPIWriteOne(RxControl2_Reg, 0x01);
	SPIWriteOne(RxWait_Reg, 5);
	return TRUE;
}

//=======================================================
//	����: Request
//	����: 	�ú���ʵ�ֶԷ���FM1702������Χ֮�ڵĿ�Ƭ��Request����
//			��������M1���ĸ�λ��Ӧ
//	����:	mode: ALL(�������FM1702������Χ֮�ڵĿ�Ƭ) 
//			STD(�����FM1702������Χ֮�ڴ���HALT״̬�Ŀ�Ƭ) 
//			Comm_Set, �����룺ָFM1702����IC��������
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_REQERR: Ӧ�����
//=======================================================
UINT8 Request(UINT8 mode)
{

	UINT8 temp;

	//ѡ��TX1��TX2�ķ��������迹
	SPIWriteOne(CwConductance_Reg, 0x3f);
	//ѡ������У�������ģʽ
	SPIWriteOne(ChannelRedundancy_Reg, 0x03);
	//��������bit�ĸ�ʽ
	SPIWriteOne(BitFraming_Reg, 0x07);
	gBuff[0] = mode;		//Requestģʽѡ�� 
	temp = SPIReadOne(Control_Reg);
	temp = temp & (0xf7);
	SPIWriteOne(Control_Reg, temp);			//Control reset value is 00
	temp = Command_Send(Transceive, gBuff, 1);   //���ͽ������� 
	if (temp == 0)
	{
		return FM1702_NOTAGERR;
	}

	temp = Read_FIFO(gBuff);		//��FIFO�ж�ȡӦ����Ϣ��gBuff�� 
	// �ж�Ӧ���ź��Ƿ���ȷ 
	//2  	Mifare Pro ��
	//4 	Mifare One ��
	if ((gBuff[0] == 0x04) & (gBuff[1] == 0x0) &(temp == 2))
	{
		return FM1702_OK;
	}
	return FM1702_REQERR;
}

//=======================================================
//���ƣ�Clear_FIFO
//���ܣ����FM1702��FIFO
//����:
//���: TRUE:�ɹ�,FALSE:ʧ��
//========================================================
BOOL Clear_FIFO(void)
{
	UINT8 ucResult, i;
	ucResult = SPIReadOne(Control_Reg);
	ucResult |= 0x01;
	SPIWriteOne(Control_Reg, ucResult);
	for (i = 0; i < 0xA0; i++)
	{
		ucResult = SPIReadOne(FIFOLength_Reg);
		if (ucResult == 0)
			return TRUE;
	}
	return FALSE;
}

//=======================================================
//���ƣ�Write_FIFO
//���ܣ�������д��FM1702��FIFO
//����:	buff��Ҫд���������Ram�е��׵�ַ
//		count��	Ҫд����ֽ���
//���: 
//========================================================
void Write_FIFO(UINT8 *buff, UINT8 count)
{
	UINT8 i;
	if (count == 0)
		return;
	for (i = 0; i < count; i++)
		SPIWriteOne(FIFO_Reg, buff[i]);
	//		SPIWrite(FIFO_Reg,buff,count);
}

//=======================================================
//	����: ReadFIFO
//	����: �ú���ʵ�ִ�FM1702��FIFO�ж���x bytes����
//	����:	buff, ָ��������ݵ�ָ��
//	���: 	���ݳ��ȣ���λ���ֽڣ� 
//=======================================================
UINT8 Read_FIFO(UINT8 *buff)
{
	UINT8	ucResult, i;
	ucResult = SPIReadOne(FIFOLength_Reg);
	if (ucResult == 0 || ucResult > 16)
		return 0;
	for (i = 0; i < ucResult; i++)
	{
		buff[i] = SPIReadOne(FIFO_Reg);
	}

	return ucResult;
}

//=======================================================
//	����: Command_Send
//	����: �ú���ʵ����FM1702��������Ĺ���
//	����:	count, ����������ĳ���  
//			buff, ָ����������ݵ�ָ�� 
//			Comm_Set, �����룺ָFM1702����IC��������
//	���: 	TRUE, �����ȷִ�� 
//			FALSE, ����ִ�д���
//=======================================================
UINT8 Command_Send(UINT8 Comm_Set, UINT8 *buff, UINT8 count)
{
	UINT8 ucResult1, ucResult2, i;
	SPIWriteOne(Command_Reg, 0x00);
	if (Clear_FIFO() == FALSE)
		return FALSE;
	Write_FIFO(buff, count);
	SPIWriteOne(Command_Reg, Comm_Set);
	for (i = 0; i < 0xA0; i++)
	{
		ucResult1 = SPIReadOne(Command_Reg);
		ucResult2 = SPIReadOne(InterruptRq_Reg) & 0x80;
		if (ucResult1 == 0 || ucResult2 == 0x80)
			return TRUE;
	}
	return FALSE;
}

//=======================================================
//	����: AntiColl
//	����: 	�ú���ʵ�ֶԷ���FM1702������Χ֮�ڵĿ�Ƭ�ķ���ͻ���
//	����:	��ȡ����ID(6Byte)
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_BYTECOUNTERR: �����ֽڴ���
//			FM1702_SERNRERR: ��Ƭ���к�Ӧ�����
//=======================================================
UINT8 AntiColl(void)
{

	UINT8	temp, i;

	//ѡ��TX1��TX2�ķ��������迹
	SPIWriteOne(CwConductance_Reg, 0x3f);
	//ѡ������У�������ģʽ
	SPIWriteOne(ChannelRedundancy_Reg, 0x03);

	gBuff[0] = RF_CMD_ANTICOL;
	gBuff[1] = 0x20;
	temp = Command_Send(Transceive, gBuff, 2);
	if (temp == 0)
	{
		return(FM1702_NOTAGERR);
	}

	temp = SPIReadOne(FIFOLength_Reg);
	if (temp == 0)
	{
		return FM1702_BYTECOUNTERR;
	}

	for (i = 0; i < temp; i++)
	{
		gBuff[i] = SPIReadOne(FIFO_Reg);
	}
	temp = SPIReadOne(ErrorFlag_Reg);	// �жϽӅ������Ƿ��г�ͻλ
	temp = temp & 0x01;
	if (temp == 0x00)
	{
		for (i = 0; i < 5; i++)
			temp ^= gBuff[i];
		if (temp)
			return(FM1702_SERNRERR);
		for (i = 0; i < 5; i++)
		{
			gCard_UID[i] = gBuff[i];
		}
		return(FM1702_OK);
	}
	else //�г�ͻλ
		return FM1702_SERNRERR;
}

//=======================================================
//	����: SelectCard
//	����: 	�ú���ʵ�ֶԷ���FM1702������Χ֮�ڵ�ĳ�ſ�Ƭ����ѡ��
//	����:	N/A
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_BYTECOUNTERR: �����ֽڴ���
//			FM1702_PARITYERR: ��żУ���
//			FM1702_CRCERR: CRCУ���
//			FM1702_SELERR: ѡ������
//=======================================================
UINT8 SelectCard(void)
{

	UINT8	temp, i;

	//ѡ��TX1��TX2�ķ��������迹
	SPIWriteOne(CwConductance_Reg, 0x3f);
	//ѡ������У�������ģʽ
	SPIWriteOne(ChannelRedundancy_Reg, 0x0F);

	gBuff[0] = RF_CMD_SELECT;
	gBuff[1] = 0x70;
	for (i = 0; i < 5; i++)
	{
		gBuff[i + 2] = gCard_UID[i];
	}
	temp = Command_Send(Transceive, gBuff, 7);
	if (temp == 0)
	{
		return(FM1702_NOTAGERR);
	}
	else
	{
		temp = SPIReadOne(ErrorFlag_Reg);
		if ((temp & 0x02) == 0x02) return(FM1702_PARITYERR);
		if ((temp & 0x04) == 0x04) return(FM1702_FRAMINGERR);
		if ((temp & 0x08) == 0x08) return(FM1702_CRCERR);
		temp = SPIReadOne(FIFOLength_Reg);
		if (temp != 1) return(FM1702_BYTECOUNTERR);
		//		SPIRead(FIFO_Reg,gBuff,temp);	//��FIFO�ж�ȡӦ����Ϣ
		gBuff[0] = SPIReadOne(FIFO_Reg);
		if (gBuff[0] == 0x08) 	  	// �ж�Ӧ���ź��Ƿ���ȷ 
			return(FM1702_OK);
		else
			return(FM1702_SELERR);
	}
}
//�洢��Կ
/* n:�����n�����루n��0��ʼ������ */
//*ramadr:��Կ
//gBuff[0]��Կ��Կ��ַ =0x80+n*12
//gBuff[1]:0
//gBuff[2]~gBuff[9]���ܷ�����//////////////
/*UINT8 Load_Key(UINT8 n,UINT8 *ramadr)
{
	UINT8 acktemp,temp[1],i;
	UINT8 ucBuff[16];
	UINT16	unData;
	unData=0x80+n*12;
	ucBuff[0]=(UINT8)(unData&0x00ff);
	ucBuff[1]=(UINT8)(unData>>8);
	for (i=0;i<6;i++)
	{
	temp[0]=ramadr[i];
	ucBuff[2+i+i]=(((ramadr[i]&0xf0)>>4)|((~ramadr[i])&0xf0));
	ucBuff[3+i+i]=((temp[0]&0xf)|(~(temp[0]&0xf)<<4));
	}
	acktemp=Command_Send(WriteEE,ucBuff,0x0e);
	sys_delayms(4);
	temp[0] = SPIReadOne(SecondaryStatus_Reg);
	if (temp[0]&0x40)
	{
	temp[0]=0x0;
	SPIWriteOne(Command_Reg,temp[0]);
	return(0);
	}
	temp[0]=0x0;
	SPIWriteOne(Command_Reg,temp[0]);
	return(1);
}*/

UINT8 Load_Key(UINT8 *ramadr)
{
	UINT8 acktemp, temp, i;
	UINT8 ucBuff[12];
	for (i = 0; i < 6; i++)
	{
		temp = ramadr[i];
		ucBuff[2 * i] = (((ramadr[i] & 0xf0) >> 4) | ((~ramadr[i]) & 0xf0));
		ucBuff[2 * i + 1] = ((temp & 0xf) | (~(temp & 0xf) << 4));
	}
	acktemp = Command_Send(LOAD_KEY, ucBuff, 12);
	temp = SPIReadOne(SecondaryStatus_Reg);
	if (temp & 0x40)
	{
		temp = 0x0;
		SPIWriteOne(Command_Reg, temp);
		return(0);
	}
	temp = 0x0;
	SPIWriteOne(Command_Reg, temp);
	return(1);
}

/*
//=======================================================
//	����: Load_keyE2
//	����: �ú���ʵ�ְ�E2���������FM1702��keyRevBuffer��
//	����:	index: ��Կ����(ͨ����Ӧ������0~15��A���룬16~31��B����)
//	���: 	TRUE, ��Կװ�سɹ�
//			FALSE, ��Կװ��ʧ��
//=======================================================
//������Կ
//���:gBuff[0]��Կ��Կ��ַ
//gBuff[1]:0
UINT8 Load_Key_EE(UINT8 index)
{
UINT8 ucData[2],ucResult;
UINT16 unData;
if(index>=32)
return FALSE;
unData = 0x80+index*12;
ucData[0] = (UINT8)(unData&0x00FF);
ucData[1] = (UINT8)(unData>>8);
ucResult = Command_Send(LoadKeyE2,ucData,2);
if(ucResult == FALSE)
return FALSE;
sys_delayms(1);
ucResult= SPIReadOne(ErrorFlag_Reg);
if(ucResult & 0x40)
return FALSE;
return TRUE;
}
*/

//=======================================================
//	����: Authentication
//	����: 	�ú���ʵ��������֤�Ĺ���
//	����:	UID: ��Ƭ���кŵ�ַ
//			SecNR: ������
//			mode: ģʽ
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_PARITYERR: ��żУ���
//			FM1702_CRCERR: CRCУ���
//			FM1702_AUTHERR: Ȩ����֤�д�
//=======================================================
UINT8 Authentication(UINT8 *UID, UINT8 SecNR, UINT8 mode)
{
	UINT8 i;
	UINT8 temp, temp1;

	if (SecNR >= 16)
		SecNR = SecNR % 16;

	//ѡ������У�������ģʽ
	SPIWriteOne(ChannelRedundancy_Reg, 0x0F);
	gBuff[0] = mode;
	gBuff[1] = SecNR * 4 + 3;
	for (i = 0; i < 4; i++)
	{
		gBuff[2 + i] = UID[i];
	}

	temp = Command_Send(Authent1, gBuff, 6);
	if (temp == 0)
	{
		return 0x99;
	}

	temp = SPIReadOne(ErrorFlag_Reg);
	if ((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if ((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if ((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp = Command_Send(Authent2, gBuff, 0);
	if (temp == 0)
	{
		return 0x88;
	}

	temp = SPIReadOne(ErrorFlag_Reg);
	//	Show(temp,0);
	if ((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if ((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if ((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp1 = SPIReadOne(Control_Reg);
	temp1 = temp1 & 0x08;
	if (temp1 == 0x08)
	{
		return FM1702_OK;
	}

	return FM1702_AUTHERR;
}

//=======================================================
//	����: Read_Block
//	����: 	�ú���ʵ�ֶ�MIFARE�������ֵ
//	����:	buff: �������׵�ַ
//			index: ���ַ
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_PARITYERR: ��żУ���
//			FM1702_CRCERR: CRCУ���
//			FM1702_BYTECOUNTERR: �����ֽڴ���
//=======================================================

UINT8 Read_Block(UINT8 *buff, UINT8 index)
{
	UINT8 ucCmdLine[2], ucResult;
	SPIWriteOne(ChannelRedundancy_Reg, 0x0F);
	ucCmdLine[0] = RF_CMD_READ;
	ucCmdLine[1] = index;
	ucResult = Command_Send(Transceive, ucCmdLine, 2);
	if (ucResult == FALSE)
		return FM1702_NOTAGERR;   //�޿�
	ucResult = SPIReadOne(ErrorFlag_Reg);
	if ((ucResult & 0x02) == 0x02) return FM1702_PARITYERR;
	if ((ucResult & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if ((ucResult & 0x08) == 0x08) return FM1702_CRCERR;
	ucResult = Read_FIFO(buff);
	if (ucResult != 0x10)
		return FM1702_BYTECOUNTERR;
	else
		return FM1702_OK;
}

//=======================================================
//	����: Write_Block
//	����: 	�ú���ʵ��дMIFARE�������ֵ
//	����:	buff: �������׵�ַ
//			index: ���ַ
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_NOTAUTHERR: δ��Ȩ����֤
//			FM1702_EMPTY: �����������
//			FM1702_WRITEERR: д�������ݳ���
//			FM1702_OK: Ӧ����ȷ
//			FM1702_PARITYERR: ��żУ���
//			FM1702_CRCERR: CRCУ���
//			FM1702_BYTECOUNTERR: �����ֽڴ���
//=======================================================
UINT8 Write_Block(UINT8 *buff, UINT8 index)
{
	UINT8 ucCmdLine[2], ucResult, ucData[16];
	SPIWriteOne(ChannelRedundancy_Reg, 0x07);   /* Note: this line is for 1702, different from RC500*/
	ucCmdLine[0] = RF_CMD_WRITE;
	ucCmdLine[1] = index;
	ucResult = Command_Send(Transceive, ucCmdLine, 2);
	if (ucResult == FALSE)
		return FM1702_NOTAGERR;
	ucResult = Read_FIFO(ucData);
	if (ucResult == 0)
		return FM1702_BYTECOUNTERR;
	switch (ucData[0])
	{
	case 0x00:	return(FM1702_NOTAUTHERR);
	case 0x04:	return(FM1702_EMPTY);
	case 0x0a:	break;                   //����
	case 0x01:	return(FM1702_CRCERR);
	case 0x05:	return(FM1702_PARITYERR);
	default:	return(FM1702_WRITEERR);
	}
	ucResult = Command_Send(Transceive, buff, 16);
	if (ucResult == TRUE)
		return FM1702_OK;
	else
	{
		ucResult = SPIReadOne(ErrorFlag_Reg);
		if ((ucResult & 0x02) == 0x02) return FM1702_PARITYERR;
		else if ((ucResult & 0x04) == 0x04) return FM1702_FRAMINGERR;
		else if ((ucResult & 0x08) == 0x08) return FM1702_CRCERR;
		else return FM1702_WRITEERR;
	}
}

//=======================================================
//	����: MIF_Halt
//	����: �ú���ʵ����ͣMIFARE��
//	����:	N/A 
//	���: 	FM1702_OK: Ӧ����ȷ 
//			FM1702_PARITYERR: ��żУ���
//			FM1702_FRAMINGERR:FM1702֡����
//			FM1702_CRCERR: CRCУ���
//			FM1702_NOTAGERR: �޿�
//=======================================================
UINT8 MIF_Halt(void)
{
	UINT8	temp;
	//ѡ��TX1��TX2�ķ��������迹
	SPIWriteOne(CwConductance_Reg, 0x3f);
	//ѡ������У�������ģʽ
	SPIWriteOne(ChannelRedundancy_Reg, 0x03);
	*gBuff = RF_CMD_HALT;
	*(gBuff + 1) = 0x00;
	temp = Command_Send(Transmit, gBuff, 2);//����FIFO�����ַ
	if (temp == TRUE)
		return FM1702_OK;
	else
	{
		temp = SPIReadOne(ErrorFlag_Reg);
		if ((temp & 0x02) == 0x02)
		{
			return(FM1702_PARITYERR);
		}

		if ((temp & 0x04) == 0x04)
		{
			return(FM1702_FRAMINGERR);
		}
		return(FM1702_NOTAGERR);
	}
}

