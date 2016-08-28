#include <STC15F2K60S2.H>
#include "hwa_uart.h"
#include "sys_uart.h"
#include "string.h"


#define TX_BUFF_LEN     RX_BUFF_LEN
BOOL b_comRxFalg = FALSE;
BOOL b_comTxFlag = FALSE;
UINT8 a_u8_comRxBuff[RX_BUFF_LEN] = {0};
UINT8 a_u8_comTxBuff[TX_BUFF_LEN];

#define MAX_DATA_LEN		100

typedef struct
{
    UINT8 head;
    UINT8 len;
    UINT8 cmd;
    UINT8 dat[MAX_DATA_LEN];
    UINT8 check;
    UINT8 end;
}COM_STD_STRUCT;

typedef struct
{
    UINT8 head;
    UINT8 cmd;
    UINT8 value;
    UINT8 end;
    COM_STD_STRUCT std_dat;
    UINT8 expand;
}COM_TX_STRUCT;

COM_STD_STRUCT s_comRxBuff;
COM_TX_STRUCT s_comTxBuff;


void hwa_uartInit(void)
{
    sys_uartInit();
}

void hwa_uartHandler1ms(void)
{
	UINT8 i, u8_comRxLen;
    UINT8* p;
	u8_comRxLen = sys_uartReadData(a_u8_comRxBuff);
	if(u8_comRxLen == 0)
	{
		return ;
	}
	p = a_u8_comRxBuff;
	s_comRxBuff.head = *p++;
	s_comRxBuff.len = *p++;
	s_comRxBuff.cmd = *p++;
	for(i=0; i<s_comRxBuff.len-2; i++)
	{
		if(i < MAX_DATA_LEN)
		{
			s_comRxBuff.dat[i] = *p++;
		}
		else
		{
			return;
		}
	}
	for(; i<MAX_DATA_LEN; i++)
	{
		s_comRxBuff.dat[i] = 0x00;
	}
	s_comRxBuff.check = a_u8_comRxBuff[s_comRxBuff.len+1];
	s_comRxBuff.end = a_u8_comRxBuff[s_comRxBuff.len+2];
	b_comRxFalg = TRUE;
}

#define CE_OK                   0
#define CE_CHECK_ERROR          1
#define CE_CMD_ERROR			2
#define CE_BAD_PARAM            3
#define CE_DATA_ERROR           4

typedef struct
{
	UINT8 Addr[4];
	UINT8 Length[2]; 
}REQ_STRUCT;

UINT8 comfun_0xFF(void)
{
	UINT8 *readDataAddr;
	data UINT8 xdata *readXdataAddr;
    UINT8 bitIndex;
	UINT8 readLen;
	UINT8 index = 0;
	UINT8 raedNum;
	data REQ_STRUCT *p;
	
	p = (REQ_STRUCT*)&s_comRxBuff.dat;
	
	raedNum = (s_comRxBuff.len-2)/sizeof(REQ_STRUCT);
	while(raedNum--)
	{
        bitIndex = p->Addr[0] >> 4;
		
		readLen = (UINT16)p->Length[0]<<8
					| (UINT16)p->Length[1]<<0;
		s_comTxBuff.expand = TRUE;
		s_comTxBuff.std_dat.len += readLen;
		if(s_comTxBuff.std_dat.len >= MAX_DATA_LEN+2)		//BUFF溢出
		{
			return CE_DATA_ERROR;
		}
        if(bitIndex != 0)                                   //bit变量
        {
            bitIndex -= 1;
            readDataAddr = (UINT8*)((UINT16)p->Addr[2]<<8
                        | (UINT16)p->Addr[3]<<0);
			s_comTxBuff.std_dat.dat[index++] = (*readDataAddr & (1<<bitIndex))>0;
        }
        else
        {
            if((p->Addr[0] & 0x0F) == 0x00)                 //data变量
            {
                readDataAddr = (UINT8 data*)((UINT16)p->Addr[2]<<8
                        | (UINT16)p->Addr[3]<<0);
                while(readLen--)
                {
                    s_comTxBuff.std_dat.dat[index++] = *readDataAddr++;
                }
            }
            else                                            //xdata变量
            {
                readXdataAddr = (UINT8 xdata*)((UINT16)p->Addr[2]<<8
                            | (UINT16)p->Addr[3]<<0);
                while(readLen--)
                {
                    s_comTxBuff.std_dat.dat[index++] = *readXdataAddr++;
                }
            }
        }
		p++;
	}
	return CE_OK;
}

UINT8 comfun_0xFE(void)
{
	data UINT8 *writeDataAddr;
	data UINT8 xdata *writeXdataAddr;
	UINT8 writeLen;
	UINT8 index = 0;
    UINT8 bitIndex;
    bitIndex = s_comRxBuff.dat[0] >> 4;
    if(bitIndex != 0)                                   //bit变量
    {
        bitIndex -= 1;
        writeDataAddr = (UINT8 *)((UINT16)s_comRxBuff.dat[2]<<8
                    | (UINT16)s_comRxBuff.dat[3]<<0);
        *writeDataAddr &= ~(1<<bitIndex);
        *writeDataAddr |= (s_comRxBuff.dat[4] & 0x01)<<bitIndex;
    }
    else
    {
        writeLen = s_comRxBuff.len - 2;
        if((s_comRxBuff.dat[0] & 0x0F) == 0x00)         //data变量
        {
            writeDataAddr = (UINT8 data*)((UINT16)s_comRxBuff.dat[2]<<8
                    | (UINT16)s_comRxBuff.dat[3]<<0);
            while(writeLen--)
            {
                *writeDataAddr++ = s_comRxBuff.dat[4+index++];
            }
        }
        else                                            //xdata变量
        {
            writeXdataAddr = (UINT8 xdata*)((UINT16)s_comRxBuff.dat[2]<<8
                        | (UINT16)s_comRxBuff.dat[3]<<0);
            while(writeLen--)
            {
                *writeXdataAddr++ = s_comRxBuff.dat[4+index++];
            }
        }
    }
	return CE_OK;
}

UINT8 comCheck(COM_STD_STRUCT * std)
{
	UINT8 i, check = 0;
	check ^= std->len;
	check ^= std->cmd;
	for(i=0; i<std->len-2; i++)
	{
		check ^= std->dat[i];
	}
	return check;
}

void hwa_uartHandler10ms(void)
{
	UINT8 i;
    UINT8 error = CE_OK;
    if(b_comRxFalg)
    {
        b_comRxFalg = FALSE;
        if(s_comRxBuff.head != 0xAA)
        {
            return;
        }
        b_comTxFlag = TRUE;
		s_comTxBuff.head  = 0xBB;
		s_comTxBuff.cmd   = 0x00;
		s_comTxBuff.value = 0x00;
		s_comTxBuff.end   = 0xCC;
		s_comTxBuff.expand = FALSE;
		if(s_comRxBuff.len > MAX_DATA_LEN
			|| s_comRxBuff.len < 2)
		{
            error = CE_BAD_PARAM;
		}
        if(s_comRxBuff.head == 0xAA && s_comRxBuff.end == 0xCC)
        {
        	if(comCheck(&s_comRxBuff) == s_comRxBuff.check)
        	{
        		s_comTxBuff.cmd = s_comRxBuff.cmd;
				s_comTxBuff.std_dat.head = 0xAA;
				s_comTxBuff.std_dat.len = 2;
				s_comTxBuff.std_dat.end = 0xCC;
                switch(s_comRxBuff.cmd)
                {
                    case 0xFF:
                        error = comfun_0xFF();
                        break;
						
                    case 0xFE:
                        error = comfun_0xFE();
                        break;
						
                    default:        //Cmd Error
                        error = CE_CMD_ERROR;
                        break;
                }
        	}
            else
            {
                error = CE_CHECK_ERROR;
            }
        }
        else
        {
            error = CE_BAD_PARAM;
        }
        s_comTxBuff.value = error;
    }
    
    if(b_comTxFlag)
    {
        b_comTxFlag = FALSE;
        a_u8_comTxBuff[0] = s_comTxBuff.head;
        a_u8_comTxBuff[1] = s_comTxBuff.cmd;
        a_u8_comTxBuff[2] = s_comTxBuff.value;
        a_u8_comTxBuff[3] = s_comTxBuff.end;
        if(s_comTxBuff.expand)
        {
        	s_comTxBuff.std_dat.cmd = s_comTxBuff.cmd;
	        a_u8_comTxBuff[4] = s_comTxBuff.std_dat.head;
	        a_u8_comTxBuff[5] = s_comTxBuff.std_dat.len;
	        a_u8_comTxBuff[6] = s_comTxBuff.std_dat.cmd;
	        for(i=0; i<s_comTxBuff.std_dat.len-2; i++)
	        {
	        	a_u8_comTxBuff[7+i] = s_comTxBuff.std_dat.dat[i];
	        }
	        i += 7;
        	s_comTxBuff.std_dat.check = comCheck(&s_comTxBuff.std_dat);
	        a_u8_comTxBuff[i++] = s_comTxBuff.std_dat.check;
	        a_u8_comTxBuff[i++] = s_comTxBuff.std_dat.end;
	        
            sys_uartSendData(a_u8_comTxBuff, 4+3+s_comTxBuff.std_dat.len);
        }
        else
        {
            sys_uartSendData(a_u8_comTxBuff, 4);
        }
    }
}

