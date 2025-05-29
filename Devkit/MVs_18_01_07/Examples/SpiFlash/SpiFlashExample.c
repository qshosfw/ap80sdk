/**
 **************************************************************************************
 * @file    SpiFlashExample.c
 * @brief   Spi Flash operation example
 * 
 * @author  Lujiangang
 * @version V1.0.0 	initial release
 * 
 * $Id$
 * $Created: 2014-09-02 17:31:10$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

 //�ⲿ�ִ�������ʾ��Flash ��д���������ӽ����Ȳ�����
 //ʹ�������н����ķ�ʽ�����в���, ����ʹ��ASCII�뷽ʽ���з���
 
 //ע��: Flash д�����У�д���ݵ�buf �ĵ�ַ���ܴ���code�ռ䣬�������������code �ռ�
 //��Ҫ�Ƚ������ݴ�code�ռ临�Ƶ�data�ռ�ı����У��ٽ���Flashд������

 //CONST DATA ADDR 0x100000
//USER	DATA ADDR 0x1A0000
//0x1A0000 ~ 0x1A0000+64K Ϊ��������
//0x1A0000+64k ~ 0x1A0000+64K+16K ΪBreakpoint ����
//ʾ��������Flash д/�����Ĳ�����ַ�� 0x1A0000+64K+16K = 0x1B4000 ��ʼ

#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "uart.h"
#include "clk.h"
#include "gpio.h"
#include "wakeup.h"
#include "timeout.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "timeout.h"
#include "cache.h"
#include "stdio.h"

//extern void SysTickInit(void);
#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define TEST_FLASH_WR_ADDR_BEGIN	0x1B4000	//0x1A0000+64*1024+16*1024
#define MAX_RDBUF_LEN	1024
#define MAX_CMD_LEN	1024
#define TIMEOUT_200MS 200
#define TIMEOUT_100MS 100

//FLASH_NONE_ERROR Ϊ0�� Flash �����д����붼Ϊ����
#define ERR_ILLEGAL_PARA	1
#define ERR_VERIFY_FAIL		2


char * test_string = "hello world!\r\n";
uint8_t rd_buf[MAX_RDBUF_LEN];
SPI_FLASH_INFO  FlashInfo;


#define SKIP_SPACE(p) do{\
	while((*p)== ' ' )	\
		{p++;}			\
}while(0)

bool verify(uint8_t* buf1, uint8_t* buf2, uint32_t len)
{
	uint32_t i;
	for(i=0; i<len; i++)
	{
		if(buf1[i] != buf2[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}


	
int32_t do_cmd_erase(uint8_t* buf, uint32_t len)
{
	int32_t ret;
	int32_t offset;
	char *p = (char*)buf;
	char * tmp;
	
	SKIP_SPACE(p);
	offset = strtol((const char *)p,&tmp,16);
	p = tmp;
	SKIP_SPACE(p);
	ret = strtol((const char *)p,&tmp,10);
	
	if(offset < TEST_FLASH_WR_ADDR_BEGIN || offset > FlashInfo.Capacity)
	{
		printf("Bad parameter, ");
		return ERR_ILLEGAL_PARA;
	}
	ret = SpiFlashErase(offset, ret);
	return ret;
}

int32_t do_cmd_write(uint8_t* buf, uint32_t len)
{
	int32_t ret;
	int32_t offset,w_len;
	char *p = (char*)buf;
	char * tmp;
	
	SKIP_SPACE(p);
	offset = strtol((const char *)p,&tmp,16);
	p = tmp+1;
	
	w_len = strlen(p);

	if(offset<TEST_FLASH_WR_ADDR_BEGIN || offset + w_len > FlashInfo.Capacity)
	{
		printf("Bad parameter, ");
		return ERR_ILLEGAL_PARA;
	}
	if((ret = SpiFlashWrite(offset, (uint8_t*)p, w_len)) != FLASH_NONE_ERR)
	{
		printf("Write Failed, erase Flash and retry!\r\n");
		return ret;
	}
	
	memset(rd_buf, 0, MAX_RDBUF_LEN);
	SpiFlashRead(offset, rd_buf, w_len);
	
	if(verify((uint8_t*)p, rd_buf, w_len))
	{
		ret = FLASH_NONE_ERR;
	}
	else 
	{
		ret = ERR_VERIFY_FAIL;
	}
	
	return ret;
}

int32_t do_cmd_read(uint8_t* buf, uint32_t len)	//(uint32_t offset, uint8_t* buf, uint32_t buf_len)
{
	int32_t ret, i;
	int32_t offset;
	uint32_t rd_len=0;
	char *p = (char*)buf;
	char * tmp;

	SKIP_SPACE(p);
	offset = strtol((const char *)p,&tmp,16);
	p = tmp;
	SKIP_SPACE(p);
	rd_len = strtol((const char *)p,&tmp,10);

	if(offset+rd_len > FlashInfo.Capacity || rd_len > MAX_RDBUF_LEN || rd_len < 1)
	{
		printf("Bad parameter, ");
		return ERR_ILLEGAL_PARA;
	}
	if((ret = SpiFlashRead(offset, rd_buf, rd_len)) != FLASH_NONE_ERR)
	{
		return ret;
	}
	
	printf("Flash read at address 0x%08x, length=%d :\r\n", offset,  rd_len);
	for(i=0;i<rd_len;i++)
	{
		printf("%02x ",rd_buf[i]);
		
	}
	printf("\r\n");
	return ret;
}

int32_t do_cmd_unlock(void)
{
	int32_t ret;
	char cmd[3] = "\x35\xBA\x69";
	
	// unlock flash
	ret = SpiFlashIOCtl(IOCTL_FLASH_UNPROTECT, cmd, sizeof(cmd));
	return ret;
}

int32_t do_cmd_lock(uint8_t* buf, uint32_t len)
{
	int32_t ret;
	char *tmp;
	SPI_FLASH_LOCK_RANGE range;
	if(len>=2)
	{
		SKIP_SPACE(buf);
		range = (SPI_FLASH_LOCK_RANGE)strtol((const char *)buf,&tmp,10);
	}
	else
	{
		printf("Bad parameter, ");
		return ERR_ILLEGAL_PARA;
	}

	if(range<1 || range>4)
	{
		printf("Bad parameter, ");
		return ERR_ILLEGAL_PARA;
	}
	
	ret = SpiFlashIOCtl(IOCTL_FLASH_PROTECT, range);
	
	return ret;
}

void do_cmd_help()
{
	printf("\r\nComand line syntax:\r\n");
	printf("Flash Erase:	E address(hex) length(dec) address & length will be 4KB aligned\r\n	 \
					eg. E 0x1B4000 4096\r\n");
	printf("Flash Unlock:	U				eg. U\r\n");
	printf("Flash Lock:	L lock_range[1-4]		eg. L 2\r\n");
	printf("Flash Write:	W address data(string)		eg. W 0x1B4000 abcdefg(total command line is 1024 bytes)\r\n");
	printf("Flash Read:	R address(hex) len(dec)		eg. R 0x1B4000 200(Maximum read length is 1024 bytes)\r\n");
	printf("Help:		?				print this help\r\n");
}


void GetFlashGD(int32_t protect)
{    
	uint8_t  str[20];
	int32_t FlashCapacity = 0;
		
	switch(FlashInfo.Did)
	{
		case 0x1340:	
			strcpy((char *)str,"GD25Q40(GD25Q40B)");
			FlashCapacity = 0x00080000;
			break;

		case 0x1240:
        	strcpy((char *)str,"GD25Q20(GD25Q20B)");
			FlashCapacity = 0x00040000;
			break;       

		case 0x1540:
			strcpy((char *)str,"GD25Q16(GD25Q16B)");
			FlashCapacity = 0x00200000;			
			break; 

		case 0x1640:
        	strcpy((char *)str,"GD25Q32(GD25Q32B)");
			FlashCapacity = 0x00400000;        
			break;

		case 0x1740:
        	strcpy((char *)str,"GD25Q64B");
			FlashCapacity = 0x00800000;          
			break;

		case 0x1440:
        	strcpy((char *)str,"GD25Q80(GD25Q80B)");
			FlashCapacity = 0x00100000;         
			break;

		case 0x1840:
            strcpy((char *)str,"GD25Q128B");
            FlashCapacity = 0x01000000;         
            break;

		default:
			break;
	}
        
    if(FlashCapacity > 0)
    {
        APP_DBG("�ͺ�:                         ");
        APP_DBG("%s\r\n",str);
        APP_DBG("����:                         ");
        APP_DBG("0x%08X\r\n", FlashCapacity);
    }  
    else
    {
        APP_DBG("����ʧ��\r\n");
    }
}

void GetFlashWinBound(int32_t protect)
{	
    uint8_t  str[20];
	int32_t FlashCapacity = 0;
    
	switch(FlashInfo.Did)
    {
        case 0x1440:
            strcpy((char *)str,"W25Q80BV");
            FlashCapacity = 0x00100000;             
            break;
        
        case 0x1760:
            strcpy((char *)str,"W25Q64DW");
            FlashCapacity = 0x00800000;             
            break;
        				
        case 0x1740:
            strcpy((char *)str,"W25Q64CV");
            FlashCapacity = 0x00800000; 
            break;
        
        default:
            break;
    }
    
    if(FlashCapacity > 0)
    {
        APP_DBG("�ͺ�:                         ");
        APP_DBG("%s\r\n",str);
        APP_DBG("����:                         ");
        APP_DBG("0x%08X\r\n",FlashCapacity);
    }  
    else
    {
        APP_DBG("����ʧ��\r\n");
    }    
}

void GetFlashPct(void)
{	
    uint8_t  str[20];
	int32_t FlashCapacity = 0;
    
	switch(FlashInfo.Did)
    {
        case 0x0126:
            strcpy((char *)str,"PCT26VF016");
            FlashCapacity = 0x00200000;        
			break;

        case 0x0226:       
            strcpy((char *)str,"PCT26VF032");
            FlashCapacity = 0x00400000;
            break;

        default:            
			break;
    }
      
    if(FlashCapacity > 0)
    {
        APP_DBG("�ͺ�:                         ");
        APP_DBG("%s\r\n",str);
        APP_DBG("����:                         ");
        APP_DBG("0x%08X\r\n",FlashCapacity);
    }  
    else
    {
        APP_DBG("����ʧ��\r\n");
    }  
}

void GetFlashEon(int32_t protect)
{
    uint8_t  str[20];
	int32_t FlashCapacity = 0;
    
	switch(FlashInfo.Did)
    {
        case 0x1430:
            strcpy((char *)str,"EN25Q80A");
            FlashCapacity = 0x00100000; 
            break;

        case 0x1530:
            strcpy((char *)str,"EN25Q16A");
            FlashCapacity = 0x00200000; 
            break;

        case 0x1830:
            strcpy((char *)str,"EN25Q128");
            FlashCapacity = 0x01000000; 
            break;

        case 0x1630:
            strcpy((char *)str,"EN25Q32A");
            FlashCapacity = 0x00400000; 
            break;

        case 0x1330:
            strcpy((char *)str,"EN25Q40");
            FlashCapacity = 0x00080000; 
            break;

        case 0x1730:
            strcpy((char *)str,"EN25Q64");
            FlashCapacity = 0x00800000; 
            break;

        case 0x1570:
            strcpy((char *)str,"EN25QH16");
            FlashCapacity = 0x00200000; 
            break;

        case 0x1670: 
            strcpy((char *)str,"EN25QH32");
            FlashCapacity = 0x00400000; 
            break;

        default:
            break;
    }
    
    if(FlashCapacity > 0)
    {
        APP_DBG("�ͺ�:                         ");
        APP_DBG("%s\r\n",str);
        APP_DBG("����:                         ");
        APP_DBG("0x%08X\r\n",FlashCapacity);
    }  
    else
    {
        APP_DBG("����ʧ��\r\n");
    }
}

void GetFlashBg(int32_t protect)
{	
    uint8_t  str[20];
	int32_t FlashCapacity = 0;
    
    switch(FlashInfo.Did)
    {
        case 0x1540:
            strcpy((char *)str,"BG25Q16A");
            FlashCapacity = 0x00200000; 
            break;

        case 0x1340:
            strcpy((char *)str,"BG25Q40A");
            FlashCapacity = 0x00080000; 
            break;
        
        case 0x1440:
            strcpy((char *)str,"BG25Q80A");
            FlashCapacity = 0x00100000; 
            break;
        
        default:
			break;         
    }
    
    if(FlashCapacity > 0)
    {
        APP_DBG("�ͺ�:                         ");
        APP_DBG("%s\r\n",str);
        APP_DBG("����:                         ");
        APP_DBG("0x%08X\r\n",FlashCapacity);
    }  
    else
    {
        APP_DBG("����ʧ��\r\n");
    }
}

void GetFlashEsmt(int32_t protect)
{
    uint8_t  str[20];
	int32_t FlashCapacity = 0;
       
	switch(FlashInfo.Did)
    {
        case 0x1440:
            strcpy((char *)str,"F25L08QA");
            FlashCapacity = 0x00100000; 
            break;

        case 0x1540:
            strcpy((char *)str,"F25L16QA");
            FlashCapacity = 0x00200000; 
            break;

        case 0x1641:
            strcpy((char *)str,"F25L32QA");
            FlashCapacity = 0x00400000; 
            break;

        case 0x1741:
            strcpy((char *)str,"F25L64QA");
            FlashCapacity = 0x00800000; 
            break;
              
        default:
            break;
    }    
    
    if(FlashCapacity > 0)
    {
        APP_DBG("�ͺ�:                         ");
        APP_DBG("%s\r\n",str);
        APP_DBG("����:                         ");
        APP_DBG("0x%08X\r\n",FlashCapacity);
    }  
    else
    {
        APP_DBG("����ʧ��\r\n");
    }
}

void GetDidInfo(int32_t protect)
{
	APP_DBG("%-30s","Did:");
	APP_DBG("0x%08X\r\n",FlashInfo.Did);
	APP_DBG("%-30s","������Χ(BP4~BP0:Bit4~Bit0):");
	APP_DBG("0x%08X\r\n",protect);
}

void GetFlashInfo(void)
{
	int32_t protect = 0;
   
	APP_DBG("\r\n\r\n****************************************************************\n");
        APP_DBG("%-30s\r\n","Flash��Ϣ");
	
	if(FlashInfo.Mid != FLASH_PCT)
	{
		protect = SpiFlashIOCtl(3,0);
		protect = (protect >> 2) & 0x1F;
	}
	
    switch(FlashInfo.Mid)
    {
        case FLASH_GD:
            APP_DBG("����:                         GD\r\n");
			GetDidInfo(protect);
            GetFlashGD(protect);
            break;
        
        case FLASH_WINBOUND:
            APP_DBG("����:                         WINBOUND\r\n");
			GetDidInfo(protect);
            GetFlashWinBound(protect);
            break;
        
        case FLASH_PCT:
            APP_DBG("����:                         PCT\r\n");
            GetFlashPct();
            break;
        
        case FLASH_EON:            
            APP_DBG("����:                         EN\r\n");
			GetDidInfo(protect);
            GetFlashEon(protect);
            break;
        
        case FLASH_BG:
            APP_DBG("����:                         BG\r\n");
			GetDidInfo(protect);
            GetFlashBg(protect);
            break;
        
        case FLASH_ESMT:
            APP_DBG("����:                         ESMT\r\n");
			GetDidInfo(protect);
            GetFlashEsmt(protect);
            break;
        
        default:            
            APP_DBG("����:                         not found\r\n");
            break;
    }
	APP_DBG("\r\n");
	APP_DBG("****************************************************************\n");
}

//Flash ����
//����: ��
//����ֵ: �����ɹ�����TRUE�����򷵻�FALSE
bool FlashUnlock(void)
{
	char cmd[3] = "\x35\xBA\x69";
	
	if(SpiFlashIOCtl(IOCTL_FLASH_UNPROTECT, cmd, sizeof(cmd)) != FLASH_NONE_ERR)
	{
		return FALSE;
	}

	return TRUE;
}


//Flash����
//����:lock_range ��Flash������Χ: 
//		 FLASH_LOCK_RANGE_HALF : 			����1/2 Flash �ռ�(��0��ʼ����ͬ)
//		FLASH_LOCK_RANGE_THREE_QUARTERS: 	����3/4 Flash �ռ�
//		FLASH_LOCK_RANGE_SEVENTH_EIGHT:	����7/8 Flash �ռ�
//		FLASH_LOCK_RANGE_ALL:				����ȫ��Flash �ռ�
//ע��: ��Ҫ���ݶ����USER BANK �ĵ�ַ�����������ķ�Χ��������ܽ�USER bank�ռ�������޷�д����
//����ֵ: �����ɹ�����TRUE�����򷵻�FALSE
bool FlashLock(SPI_FLASH_LOCK_RANGE lock_range)
{
	if(SpiFlashIOCtl(IOCTL_FLASH_PROTECT, lock_range) != FLASH_NONE_ERR)
	{
		return FALSE;
	}

	return TRUE;
}





int32_t main(void)
{	
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating
	
	WdgDis();		//�رտ��Ź�
	SysGetWakeUpFlag();             //get wake up flag, DO NOT remove this!!
	

#ifdef USE_POWERKEY_SLIDE_SWITCH
	SysPowerKeyInit(POWERKEY_MODE_SLIDE_SWITCH, 500);	//Ӳ����ģʽ
#endif

#ifdef USE_POWERKEY_SOFT_PUSH_BUTTON
	SysPowerKeyInit(POWERKEY_MODE_PUSH_BUTTON, 2000); //����ģʽ 2s
#endif
	
	
	ClkPorRcToDpll(0);              //clock src is 32768hz OSC
	CacheInit();
#ifdef FUNC_RADIO_DIV_CLK_EN
	ClkDpllClkGatingEn(1);
#endif
	
	//�ڽ���Flash����ǰ���������ȵ��øó�ʼ������
	SpiFlashInfoInit();
	//ϵͳ��ʱ��������ʱ
	SysTickInit();
	GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);
	FuartInit(115200, 8, 0, 1);

	//��ȡFlash��Ϣ
	SpiFlashGetInfo(&FlashInfo);
	FlashUnlock();

	
	
	APP_DBG("------------------------------------------------------\n");
	APP_DBG("             SPI FLASH Operation Example                      \n");
	APP_DBG("       Mountain View Silicon Technology Co.,Ltd.               \n"); 
	APP_DBG("------------------------------------------------------\r\n\r\n");

	//��ӡFlash��Ϣ
	GetFlashInfo();
	
	

#if 0	
	//�ô�������ʾ��SpiFlashWrite()�����Ĳ���buf��������code�ռ�
	ret = SpiFlashErase(TEST_FLASH_WR_ADDR_BEGIN, 4 * 1024);
	if(ret != 0 )
		APP_DBG("Flash erase error!\r\n");
	
	{
			uint8_t buf[100];
			len = strlen(test_string);
			memset(buf, 0, 100);
			//SpiFlashWrite()������buf����code�ռ䣬test_string��code �ռ䣬���Ի�ʧ��
			ret = SpiFlashWrite(TEST_FLASH_WR_ADDR_BEGIN, (uint8_t*)test_string, len);
			if(ret)
			{
				//��test_string ���Ƶ�data�ռ�ı���buf�У�Ȼ����д��Flash��
				memcpy(buf,test_string,strlen(test_string));
				ret = SpiFlashWrite(TEST_FLASH_WR_ADDR_BEGIN, buf, len);
				memset(buf, 0, 100);
			}
			
			if(!ret)
				ret = SpiFlashRead(TEST_FLASH_WR_ADDR_BEGIN, buf, len);
			if(!ret)
				FuartSend(buf, len);
			
	}
	while(1);
#endif



#if 1
	//�ⲿ�ִ�������ʾ�������н���ʽFlash����, ����ʹ��ASCII�뷽ʽ���з���
	
	/*
	ret = SpiFlashErase(TEST_FLASH_WR_ADDR_BEGIN, 4 * 1024);
	if(ret != 0 )
		APP_DBG("Flash erase error!\r\n");
		*/
	do_cmd_help();
	WdgDis();
	while(1)
	{
		uint8_t cmd_buf[MAX_CMD_LEN] = {0};
		uint16_t cmd_len;
		int32_t ret;

		cmd_len =  FuartRecv(cmd_buf,MAX_CMD_LEN, TIMEOUT_200MS);
		if(cmd_len>0)
		{
			unsigned char cmd_head = cmd_buf[0];
			switch(cmd_head)
			{
				case 'E':
				case 'e':
					ret = do_cmd_erase(&cmd_buf[1],cmd_len-1);
					break;
				case 'W':
				case 'w':
					ret = do_cmd_write(&cmd_buf[1],cmd_len-1);
					break;
				case 'R':
				case 'r':
					ret = do_cmd_read(&cmd_buf[1],cmd_len-1);
					break;
				case 'U':
				case 'u':
					ret = do_cmd_unlock();
					break;
				case 'L':
				case 'l':
					ret = do_cmd_lock(&cmd_buf[1],cmd_len-1);
					break;
				case '?':
					do_cmd_help();
					continue;
				case 'h':
				case 'H':
					if(cmd_len == 1)
					{
						do_cmd_help();	
						continue;
					}
					else if(cmd_len == 4)	//"help" or "Help"
					{
						if(cmd_buf[1] == 'e' && cmd_buf[2] == 'l' && cmd_buf[3] == 'p')
						{
							do_cmd_help();	
							continue;
						}
					}
				default:
					printf("Unknown command!\r\n");
					continue;
			
			}



			if(ret == FLASH_NONE_ERR)
				printf("command executed successful!\r\n");
			else if(ret == ERR_ILLEGAL_PARA)
				printf("Illegal command\r\n");
			else if(ret == ERR_VERIFY_FAIL)
				printf("Verify failed, erase Flash and retry!\r\n");
			else
				printf("Command executed failed!\r\n");
		
		}
				
		
	}
}


#endif

//
