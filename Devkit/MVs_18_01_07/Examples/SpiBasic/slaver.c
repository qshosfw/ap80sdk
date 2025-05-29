/**
 **************************************************************************************
 * @file    slaver.c
 * @brief   SPI (Slaver)
 *
 * @author  Lilu
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "type.h"
#include "crc.h"
#include "spi_protocol.h"
#include "rst.h"
#include "clk.h"
#include "uart.h"
#include "gpio.h"
#include "spis.h"
#include "cache.h"
#include "delay.h"
#include "pcm_fifo.h"
#include "watchdog.h"
#include "timeout.h"

#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)

SpiMessContext RecvHeader, SendHeader;
uint8_t SpisSendBuf[100] = {

                                0xA5, 0x5A, 0x18, 0x63, 0x17, 0x47, 0x24, 0x19, 0xD2, 0x04, 0x09, 0x3A, 0x89, 0xA3, 0xA0, 0x00, 
                                0x55, 0x66, 0x77, 0x86, 0x9F, 0xEB, 0x6C, 0x4F, 0xFF, 0xF7, 0x3D, 0x11, 0x0B, 0xCE, 0x80, 0x14, 
                                0x44, 0xE2, 0x7B, 0x87, 0x03, 0x3D, 0xDC, 0xDC, 0xDD, 0xDD, 0xE0, 0x00, 0x83, 0xE2, 0x26, 0x61, 
                                0x04, 0x3B, 0xC0, 0x00, 0x02, 0xEB, 0xA7, 0xB8, 0x1B, 0xC4, 0x2F, 0x82, 0x11, 0x11, 0x00, 0x13, 
                                0xBB, 0x7B, 0xBB, 0xDC, 0x87, 0x3C, 0x9A, 0x65, 0x10, 0x8F, 0x1F, 0x59, 0x07, 0xBF, 0x9F, 0x58, 
                                0x82, 0x1C, 0x10, 0x84, 0x59, 0xE0, 0x60, 0x6E, 0xE2, 0xE2, 0x1E, 0xE6, 0x88, 0x89, 0xA5, 0x74, 
                                0x44, 0xD0, 0x43, 0xab, 
                          };

uint8_t SpisRecvBuf[100];

//extern void SysTickInit(void);
void SpisLookForSync()
{
	uint8_t c = 0;
	while(1)
	{
		SpiSlaveReceive(&c, 1, 30);
		if(c == SPI_SYNC_BYTE)
		{
			break;
		}
	}
}
int32_t main(void)
{
	uint32_t i;
    ClkPorRcToDpll(0);
    CacheInit();
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);

    SysTickInit();

	//Disable Watchdog
    WaitMs(200);
	WdgDis();

    GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);

	FuartInit(115200, 8, 0, 1);

    WaitMs(100);
    DBG("/==========================================================================\\\n");
    DBG("|             		SPI BASIC TRANSFER APPLICATION (SLAVER)                  |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");

    //SPIS initialization
	/* Config spis pin: spis_mosi b[20], spis_clk  b[21], spis_miso b[22], spis_cs b[23] */
    GpioSpisIoConfig(0);
    SpiSlaveInit(0, 0x0, 0x400, 0x400);

	while(1)
	{
		//Ω” ’master«Î«Û
		SpisLookForSync();
		SpiSlaveReceive((uint8_t *)&RecvHeader.Message, 5, 100);
		DBG("SpisHeader is %02x,%04x,%04x\n",  RecvHeader.Message, RecvHeader.Param, RecvHeader.CrcValue);
		switch(RecvHeader.Message)
		{
			case SPIM_READ:
				SendHeader.SyncWord =SPI_SYNC_BYTE;
				SendHeader.Message = SPIS_OK;//ok slaver will send 100 byte
				SendHeader.Param = RecvHeader.Param;
				SendHeader.CrcValue = 0x1111;
				SpiSlaveSend((uint8_t *)&SendHeader, 6, 100);
				
				SendHeader.SyncWord = SPI_SYNC_BYTE;
				SpiSlaveSend((uint8_t *)&SendHeader, 1, 100);
				SpiSlaveSend(SpisSendBuf, RecvHeader.Param, 100);
				SpisClrRxBuff();
				break;
			case SPIM_WRITE:
				SendHeader.SyncWord =SPI_SYNC_BYTE;
				SendHeader.Message = SPIS_OK;//ok slaver will recv 100 byte
				SendHeader.Param = RecvHeader.Param;
				SendHeader.CrcValue = 0x1111;
				SpiSlaveSend((uint8_t *)&SendHeader, 6, 100);
			
				SpisLookForSync();
				SpiSlaveReceive(SpisRecvBuf, RecvHeader.Param, 1000);
				for(i = 0; i < RecvHeader.Param; i++)
				{
					DBG("%02x ",SpisRecvBuf[i]);
				}
				DBG("\n");
				break;
			default:
				break;
		}
	}
}
