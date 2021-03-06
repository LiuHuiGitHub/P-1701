#include "typedef.h"
#include "drv_fm1702_itf.h"
#include "drv_fm1702.h"
#include "sys_delay.h"


////////////////////全局变量////////////////////////////////
UINT8 gBuff[16];
UINT8 gCard_UID[5];

static void drv_fm1702WriteFifo(UINT8 *buff, UINT8 count);
static BOOL drv_fm1702ClearFifo(void);

//;==============================================
//;FM1702SL发送命令代码
//;==============================================
#define CMD_WRITE_E2            0x01
#define CMD_LOAD_KEY_E2         0x0B
#define CMD_LOAD_KEY            0x19
#define CMD_TRANSMIT            0x1A
#define CMD_TRANSCCEIVE         0x1E
#define CMD_AUTHENT_1           0x0C
#define CMD_AUTHENT_2           0x14


//=======================================================
//  名称: drv_fm1702PowerDownMode
//  功能: 该函数实现FM1702进入硬件低功耗模式，
//      退出需要重新初始化
//  输入:N/A
//  输出: N/A
//=======================================================
void drv_fm1702EnterHardPowerDown(void)
{
    drv_fm1702ItfSetReset();
}

//=======================================================
//  名称: drv_fm1702EnterSoftPowerDown
//  功能: 该函数实现FM1702进入软件低功耗模式
//      可drv_fm1702ExitSoftPowerDown退出，需512个时钟周期
//  输入:N/A
//  输出: N/A
//=======================================================
void drv_fm1702EnterSoftPowerDown(void)
{
	UINT8 u8_Result;
    u8_Result = drv_fm1702ItfReadByte(Control_Reg);
    u8_Result |= 0x20;
    drv_fm1702ItfWriteByte(Control_Reg, u8_Result);
}

//=======================================================
//  名称: drv_fm1702ExitSoftPowerDown
//  功能: 该函数实现FM1702退出软件低功耗模式
//  输入:N/A
//  输出: N/A
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
//  名称: drv_fm1702Init
//  功能: 该函数实现对FM1702初始化操作
//  输入:N/A
//  输出: N/A
//=======================================================
BOOL drv_fm1702Init(void)
{
    UINT8 i;
    drv_fm1702ItfSetReset();    // FM1702复位
    sys_delayus(10);
    drv_fm1702ItfClearReset();
    //进入复位阶段，需要512个FM1702SL的时钟周期，约38us
    sys_delayus(50);
    //进入初始化阶段，需要128个时钟周期，约10us

    // 等待Command = 0,FM1702复位成功
    for (i = 0; i < 250; i++) // 延时
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
    drv_fm1702ItfWriteByte(Page_Reg, 0x80); //初始化SPI接口
    for (i = 0; i < 250; i++) // 延时
    {
        if (drv_fm1702ItfReadByte(Command_Reg) == 0x00)   //SPI初始化成功
        {
            drv_fm1702ItfWriteByte(Page_Reg, 0);//可以使用SPI接口
            break;
        }
        if( i >= 250)
        {
            return FALSE;
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////
    drv_fm1702ItfWriteByte(InterruptEn_Reg, 0x7F);  //  禁止所有中断请求（最高位置0）
    drv_fm1702ItfWriteByte(InterruptRq_Reg, 0x7F);  // 禁止所有中断请求标识置0（最高位置0）

    //设置调制器的输入源为内部编码器, 并且设置TX1和TX2
    drv_fm1702ItfWriteByte(TxControl_Reg, 0x5B);        // 发送控制寄存器
    drv_fm1702ItfWriteByte(RxControl2_Reg, 0x01);
    drv_fm1702ItfWriteByte(RxWait_Reg, 5);

    //选择TX1、TX2的发射天线阻抗
    drv_fm1702ItfWriteByte(CwConductance_Reg, 0x3F);

    return TRUE;
}

//=======================================================
//  名称: drv_fm1702Request
//  功能:   该函数实现对放入FM1702操作范围之内的卡片的Request操作
//          即：请求M1卡的复位响应
//  输入:   mode: ALL(监测所以FM1702操作范围之内的卡片)
//          STD(监测在FM1702操作范围之内处于HALT状态的卡片)
//          Comm_Set, 命令码：指FM1702发给IC卡的命令
//  输出:   FM1702_NOTAGERR: 无卡
//          FM1702_OK: 应答正确
//          FM1702_REQERR: 应答错误
//=======================================================
UINT8 drv_fm1702Request(UINT8 mode)
{
    UINT8 temp;

    //选择数据校验种类和模式
    drv_fm1702ItfWriteByte(ChannelRedundancy_Reg, 0x03);
    //调整面向bit的格式
    drv_fm1702ItfWriteByte(BitFraming_Reg, 0x07);
    gBuff[0] = mode;        //Request模式选择
    temp = drv_fm1702ItfReadByte(Control_Reg);
    temp = temp & (0xf7);
    drv_fm1702ItfWriteByte(Control_Reg, temp);        //Control reset value is 00
    temp = drv_fm1702Command(CMD_TRANSCCEIVE, gBuff, 1);   //发送接收命令
    if (temp == 0)
    {
        return FM1702_NOTAGERR;
    }

    temp = drv_fm1702ReadFifo(gBuff);       //从FIFO中读取应答信息到gBuff中
    // 判断应答信号是否正确
    //2     Mifare Pro 卡
    //4     Mifare One 卡
    if ((gBuff[0] == 0x04) & (gBuff[1] == 0x0) & (temp == 2))
    {
        return FM1702_OK;
    }
    return FM1702_REQERR;
}

//=======================================================
//名称：drv_fm1702ClearFifo
//功能：清空FM1702的FIFO
//输入:
//输出: TRUE:成功,FALSE:失败
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
//名称：drv_fm1702WriteFifo
//功能：将数据写入FM1702的FIFO
//输入: buff：要写入的数据在Ram中的首地址
//      count:要写入的字节数
//输出:
//========================================================
static void drv_fm1702WriteFifo(UINT8 *buff, UINT8 count)
{
    drv_fm1702ItfWrite(FIFO_Reg, buff, count);
}

//=======================================================
//  名称: drv_fm1702ReadFifo
//  功能: 该函数实现从FM1702的FIFO中读出x bytes数据
//  输入:   buff, 指向读出数据的指针
//  输出:   数据长度（单位：字节）
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
//  名称: drv_fm1702Command
//  功能: 该函数实现向FM1702发送命令集的功能
//  输入:   count, 待发送命令集的长度
//          buff, 指向待发送数据的指针
//          Comm_Set, 命令码：指FM1702发给IC卡的命令
//  输出:   TRUE, 命令被正确执行
//          FALSE, 命令执行错误
//=======================================================
UINT8 drv_fm1702Command(UINT8 Comm_Set, UINT8 *buff, UINT8 count)
{
    UINT8 ucResult1, ucResult2, i;
    drv_fm1702ItfWriteByte(Command_Reg, 0x00);          //切换到IDLE
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
//  名称: drv_fm1702AntiColl
//  功能:   该函数实现对放入FM1702操作范围之内的卡片的防冲突检测
//  输入:   读取卡的ID(6Byte)
//  输出:   FM1702_NOTAGERR: 无卡
//          FM1702_OK: 应答正确
//          FM1702_BYTECOUNTERR: 接收字节错误
//          FM1702_SERNRERR: 卡片序列号应答错误
//=======================================================
UINT8 drv_fm1702AntiColl(void)
{
    UINT8 temp, i;

    //选择数据校验种类和模式
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

    temp = drv_fm1702ItfReadByte(ErrorFlag_Reg);    // 判断接収数据是否有冲突位
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
    else //有冲突位
    {
        return FM1702_SERNRERR;
    }
}

//=======================================================
//  名称: drv_fm1702SelectCard
//  功能:   该函数实现对放入FM1702操作范围之内的某张卡片进行选择
//  输入:   N/A
//  输出:   FM1702_NOTAGERR: 无卡
//          FM1702_OK: 应答正确
//          FM1702_BYTECOUNTERR: 接收字节错误
//          FM1702_PARITYERR: 奇偶校验错
//          FM1702_CRCERR: CRC校验错
//          FM1702_SELERR: 选卡出错
//=======================================================
UINT8 drv_fm1702SelectCard(void)
{
    UINT8   temp, i;

    //选择数据校验种类和模式
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

        temp = drv_fm1702ItfReadByte(FIFO_Reg);     //从FIFO中读取应答信息
        if (temp == 0x08)                           // 判断应答信号是否正确
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
//  名称: drv_fm1702LoadKeyToE2
//  功能: 该函数实现把密码存入FM1702的E2中
//          密钥密钥地址 =0x80+n*12
//  输入:   index: 密钥索引(通常对应扇区，0~15是A密码，16~31是B密码)
//  输出:   TRUE, 密钥装载成功
//          FALSE, 密钥装载失败
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
//  名称: drv_fm1702LoadKeyfromE2
//  功能: 该函数实现把E2中密码存入FM1702的keyRevBuffer中
//  输入:   index: 密钥索引(通常对应扇区，0~15是A密码，16~31是B密码)
//  输出:   TRUE, 密钥装载成功
//          FALSE, 密钥装载失败
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
//  名称: drv_fm1702Auth
//  功能:   该函数实现密码认证的过程
//  输入:   UID: 卡片序列号地址
//          SecNR: 扇区号
//          mode: 模式
//  输出:   FM1702_NOTAGERR: 无卡
//          FM1702_OK: 应答正确
//          FM1702_PARITYERR: 奇偶校验错
//          FM1702_CRCERR: CRC校验错
//          FM1702_AUTHERR: 权威认证有错
//=======================================================
UINT8 drv_fm1702Auth(UINT8 *UID, UINT8 SecNR, UINT8 mode)
{
    UINT8 i;
    UINT8 temp, temp1;

    if (SecNR >= 16)
    {
        SecNR = SecNR % 16;
    }

    //选择数据校验种类和模式
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
//  名称: drv_fm1702ReadBlock
//  功能:   该函数实现读MIFARE卡块的数值
//  输入:   buff: 缓冲区首地址
//          index: 块地址
//  输出:   FM1702_NOTAGERR: 无卡
//          FM1702_OK: 应答正确
//          FM1702_PARITYERR: 奇偶校验错
//          FM1702_CRCERR: CRC校验错
//          FM1702_BYTECOUNTERR: 接收字节错误
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
        return FM1702_NOTAGERR;   //无卡
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
//  名称: drv_fm1702WriteBlock
//  功能:   该函数实现写MIFARE卡块的数值
//  输入:   buff: 缓冲区首地址
//          index: 块地址
//  输出:   FM1702_NOTAGERR: 无卡
//          FM1702_NOTAUTHERR: 未经权威认证
//          FM1702_EMPTY: 数据溢出错误
//          FM1702_WRITEERR: 写卡块数据出错
//          FM1702_OK: 应答正确
//          FM1702_PARITYERR: 奇偶校验错
//          FM1702_CRCERR: CRC校验错
//          FM1702_BYTECOUNTERR: 接收字节错误
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
            break;                   //正常
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
//  名称: drv_fm1702HaltCard
//  功能: 该函数实现暂停MIFARE卡
//  输入:   N/A
//  输出:   FM1702_OK: 应答正确
//          FM1702_PARITYERR: 奇偶校验错
//          FM1702_FRAMINGERR:FM1702帧错误
//          FM1702_CRCERR: CRC校验错
//          FM1702_NOTAGERR: 无卡
//=======================================================
UINT8 drv_fm1702HaltCard(void)
{
    UINT8 temp;
    //选择数据校验种类和模式
    drv_fm1702ItfWriteByte(ChannelRedundancy_Reg, 0x03);
    *gBuff = RF_CMD_HALT;
    *(gBuff + 1) = 0x00;
    temp = drv_fm1702Command(CMD_TRANSMIT, gBuff, 2);//发送FIFO缓存地址
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

