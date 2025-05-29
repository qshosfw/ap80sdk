/**
 **************************************************************************************
 * @file    master.c
 * @brief   SPI (Master)
 *
 * @author  Lilu
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include "type.h"
#include "crc.h"
#include "spi_protocol.h"
#include "clk.h"
#include "uart.h"
#include "gpio.h"
#include "spim.h"
#include "cache.h"
#include "delay.h"
#include "watchdog.h"
#include "fsinfo.h"
#include "fat_file.h"
#include "fs_mount.h"
#include "audio_decoder.h"
#include "timeout.h"

#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define  SPIM_CONTROL_SPIS_GPIO_BANK            ('B')
#define  SPIM_CONTROL_SPIS_GPIO_PIN             (23)
#define  SPIM_CONTROL_SPIS_GPIO_BANK_PIN        ((uint32_t)1 << SPIM_CONTROL_SPIS_GPIO_PIN)
#define  SPIM_CONTROL_SPIS_GPIO_BANK_PIN_IE     ((SPIM_CONTROL_SPIS_GPIO_BANK - 'A') * 10 + GPIO_A_IE)
#define  SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OE     ((SPIM_CONTROL_SPIS_GPIO_BANK - 'A') * 10 + GPIO_A_OE)
#define  SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OUT    ((SPIM_CONTROL_SPIS_GPIO_BANK - 'A') * 10 + GPIO_A_OUT)

#define  SPIM_CONTROL_SPIS_INIT()               \
         GpioSetRegOneBit(SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OUT, SPIM_CONTROL_SPIS_GPIO_BANK_PIN);      \
         GpioSetRegOneBit(SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OE,  SPIM_CONTROL_SPIS_GPIO_BANK_PIN);      \
         GpioClrRegOneBit(SPIM_CONTROL_SPIS_GPIO_BANK_PIN_IE,  SPIM_CONTROL_SPIS_GPIO_BANK_PIN);
         
#define  SPIM_ENABLE_SPIS()                     \
         GpioClrRegOneBit(SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OUT, SPIM_CONTROL_SPIS_GPIO_BANK_PIN);

#define  SPIM_DISABLE_SPIS()                    \
         GpioSetRegOneBit(SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OUT, SPIM_CONTROL_SPIS_GPIO_BANK_PIN);

SpiMessContext RecvHeader, SendHeader;
uint8_t SpimSendBuf[100] =  {
                                0x01, 0x02, 0x03, 0xE8, 0x29, 0x48, 0x08, 0x88, 0xCE, 0xE8, 0xBF, 0x0C, 0xC3, 0xF3, 0x62, 0xEE, 
                                0x0F, 0x0D, 0xA7, 0xB1, 0xFF, 0x03, 0x01, 0x41, 0xA2, 0xFC, 0xC3, 0x26, 0x3C, 0x11, 0x27, 0x1D, 
                                0x91, 0xA5, 0x22, 0x44, 0x05, 0x19, 0xC1, 0x21, 0x18, 0xAC, 0x56, 0x4F, 0x21, 0x59, 0x3A, 0x34, 
                                0x65, 0x05, 0x02, 0x83, 0x2D, 0x3C, 0x90, 0xB8, 0xAC, 0x9C, 0xA1, 0x21, 0x77, 0xB7, 0x02, 0x32, 
                                0x74, 0x6D, 0xE2, 0xE8, 0xC5, 0x02, 0x86, 0x10, 0x39, 0x72, 0x32, 0x7A, 0x46, 0xD9, 0x03, 0x85, 
                                0x6D, 0x05, 0x0C, 0xB6, 0xDC, 0x08, 0xCD, 0x88, 0xC4, 0xE4, 0x0A, 0x93, 0xA0, 0x04, 0x0F, 0x0A, 
                                0xEA, 0xDB, 0x5D, 0xE4       
                            } ;
uint8_t SpimRecvBuf[100];
//extern void SysTickInit(void);
void SpimLookForSync()
{
	uint8_t c;
	while(1)
	{
		SPIM_ENABLE_SPIS();
		c = SpiMasterRecvByte();
		SPIM_DISABLE_SPIS();
		if(c == SPI_SYNC_BYTE)
		{
			break;
		}
	}
}

void SpimWrite()
{
	//发出写100byte请求
	SendHeader.SyncWord = SPI_SYNC_BYTE;
	SendHeader.Message = SPIM_WRITE;
	SendHeader.Param = 100;//length
	SendHeader.CrcValue = 0x1111;
	SPIM_ENABLE_SPIS();
	SpiMasterSendData((uint8_t *)&SendHeader, 6);
	SPIM_DISABLE_SPIS();
	
	//接收回复
	SpimLookForSync();
	SPIM_ENABLE_SPIS();
	SpiMasterRecvData((uint8_t *)&RecvHeader.Message, 5);
	SPIM_DISABLE_SPIS();
	DBG("SpimHeader is %02x,%04x,%04x\n", RecvHeader.Message, RecvHeader.Param, RecvHeader.CrcValue);
	switch(RecvHeader.Message)
	{
		case SPIS_OK:
			SendHeader.SyncWord =SPI_SYNC_BYTE;
			SPIM_ENABLE_SPIS();
			SpiMasterSendData((uint8_t *)&SendHeader, 1);
			SPIM_DISABLE_SPIS();
			SPIM_ENABLE_SPIS();
			SpiMasterSendData(SpimSendBuf, 100);
			SPIM_DISABLE_SPIS();

			break;
		case SPIS_NO_ROOM:
			DBG("no data!\n");
			break;
		default:
			break;
	}
}

void SpimRead()
{	
	uint8_t i;
	//发出读100byte请求
	SendHeader.SyncWord = SPI_SYNC_BYTE;
	SendHeader.Message = SPIM_READ;
	SendHeader.Param = 100;//length
	SendHeader.CrcValue = 0x1111;
	SPIM_ENABLE_SPIS();
	SpiMasterSendData((uint8_t *)&SendHeader, 6);
	SPIM_DISABLE_SPIS();
	
	//接收回复
	SpimLookForSync();
	SPIM_ENABLE_SPIS();
	SpiMasterRecvData((uint8_t *)&RecvHeader.Message, 5);
	SPIM_DISABLE_SPIS();
	DBG("SpimHeader is %02x,%04x,%04x \n", RecvHeader.Message, RecvHeader.Param, RecvHeader.CrcValue);
	switch(RecvHeader.Message)
	{
		case SPIS_OK:
			SpimLookForSync();
			SPIM_ENABLE_SPIS();
			SpiMasterRecvData(SpimRecvBuf, 100);
			SPIM_DISABLE_SPIS();
			for(i = 0; i < 100; i++)
			{
				DBG("%02x ",SpimRecvBuf[i]);
			}
			DBG("\n");
			break;
		case SPIS_NO_DATA:
			DBG("no data!\n");
			break;
		default:
			break;
	}
}

int32_t main(void)
{

    ClkPorRcToDpll(0);
    CacheInit();
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);	
	CacheInit();
    SysTickInit();

	//Disable Watchdog
    WaitMs(200);
	WdgDis();

    GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);

	FuartInit(115200, 8, 0, 1);

    WaitMs(100);
    DBG("/==========================================================================\\\n");
    DBG("|            		 SPI BASIC TRANSFER APPLICATION (MASTER)                 |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");

	//SPI master initialization
    SPIM_CONTROL_SPIS_INIT();
    GpioSpimIoConfig(3);
    SpiMasterInit(0, SPIM_CLK_DIV_24M);
	
	SpimWrite();
	WaitMs(100);//Slaver端在打印，需等待其完成，否则下一条读命令slaver可能会漏过
	//本demo假设slaver始终处于接收状态，实际使用时可以加入slaver-master的流控线，或加入命令一定时间无回复重发机制
	SpimRead();
	
	while(1);

}
