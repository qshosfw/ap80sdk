#include <stdio.h>
#include "type.h"
#include "uart.h"
#include "clk.h"
#include "spi_flash.h"
#include "cache.h"
#include "gpio.h"
#include "dac.h"
#include "audio_adc.h"
#include "wakeup.h"
#include "timer.h"
#include "rtc.h"
#include "adc.h"
#include "host_hcd.h"
#include "watchdog.h"
#include "mixer.h"
#include "lcd_seg.h"
#include "fat_file.h" 
#include "sd_card.h"
#include "sdio.h"
#include "timeout.h"


#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)

uint8_t Sec[1024];
//extern void SysTickInit(void);
#define	SD_PORT_NUM                 	1		// SD卡端口定义
	#define CARD_DETECT_PORT_IN			GPIO_A_IN	
	#define CARD_DETECT_PORT_OE			GPIO_A_OE	
	#define CARD_DETECT_PORT_PU			GPIO_A_PU	
	#define CARD_DETECT_PORT_PD			GPIO_A_PD	
	#define CARD_DETECT_PORT_IE  		GPIO_A_IE
	#define CARD_DETECT_BIT_MASK		(1 << 20)
	
uint8_t UartRec;
#define 	SetKeyValue(x)  (UartRec = (x)) 
#define 	GetKeyValue() 	(UartRec)
#define 	ClrKeyValue() 	(UartRec = 0) 


//extern bool SdioIsDatTransDone(void);
//extern SD_CARD_ERR_CODE SdCardWriteBlockStart(uint32_t block);
//extern SD_CARD_ERR_CODE SdCardWriteBlockNext(uint8_t* buffer);
//extern SD_CARD_ERR_CODE SdCardWriteBlockEnd(void);
//extern SD_CARD_ERR_CODE SdCardReadBlockStart(uint32_t block, uint8_t* buffer);
//extern SD_CARD_ERR_CODE SdCardReadBlockNext(uint8_t* buffer);
//extern SD_CARD_ERR_CODE SdCardReadBlockEnd(void);

__attribute__((section(".driver.isr")))void SdInterrupt(void)
{
	SdioDataInterruptClear();
	DBG("SDIO Data Transfer Done!\n");
}

static __INLINE void CardDetectDelay(void)
{
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
}

void LockSdClk(void)
{
}
void UnLockSdClk(void)
{
}

bool IsCardLink(void)
{
	bool TempFlag;
	LockSdClk();
	GpioSdIoConfig(RESTORE_TO_GENERAL_IO);

	GpioClrRegBits(CARD_DETECT_PORT_PU, CARD_DETECT_BIT_MASK);
	GpioClrRegBits(CARD_DETECT_PORT_PD, CARD_DETECT_BIT_MASK);
	GpioClrRegBits(CARD_DETECT_PORT_OE, CARD_DETECT_BIT_MASK);

	GpioSetRegBits(CARD_DETECT_PORT_IE, CARD_DETECT_BIT_MASK);

	CardDetectDelay(); // ??? Disable????IE ?,??????

	if(GpioGetReg(CARD_DETECT_PORT_IN) & CARD_DETECT_BIT_MASK)
	{
		TempFlag = FALSE;
	}
	else
	{
		TempFlag = TRUE;
	}
	
	if(TempFlag)
	{
		GpioSdIoConfig(SD_PORT_NUM);
	}

	UnLockSdClk();

	return TempFlag;
}
static bool HardwareInit()
{
	if(!IsCardLink())
	{
		return FALSE;
	}
	if(SdCardInit())	
	{
		return FALSE;
	}
	NVIC_EnableIRQ(SD_IRQn);
	SdioDataInterruptEn();
    return TRUE;
}

int32_t main(void)
{
	int32_t i = 0;
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating
	
	ClkPorRcToDpll(0); 		//clock src is 32768hz OSC
	CacheInit();	
	SysTickInit();
	
	WdgDis();
	GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);
	FuartInit(115200, 8, 0, 1);

    DBG("/==========================================================================\\\n");
    DBG("|                      SdCard Read\\Write Data TESTBENCH                    |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");
	DBG("SdCard detecting ...\n");
	while(1)
	{
		if(HardwareInit())
		{
			DBG("Hardware initialize success.\n");
			break;
		}
		CardDetectDelay();
	}
	DBG("sd card speed:%d\n", SdCardGetTransSpeed());
	SdCardSetTransSpeed(3);
	DBG("sd card speed:%d\n", SdCardGetTransSpeed());
	
	memset(Sec, 0, 1024);
	DBG("please input command as the following\n");
	DBG("Command   Discription\n");
	//SD卡接口
	DBG("r   		阻塞式读取2个扇区(0,1)\n");
	DBG("w   		阻塞式写2个扇区(2,3)\n");
	DBG("R   		非阻塞式读扇区(0,1)\n");
	DBG("W   		非阻塞式写扇区(2,3)\n");
	//SDIO接口
	DBG("d   		SDIO接口读扇区(2,3)\n");
	DBG("x   		SDIO接口写扇区(2,3)\n");
	DBG("\n");
	while(1)
	{
		while(!GetKeyValue());
		switch(GetKeyValue())
		{
			case 'r'://阻塞式读取2个扇区
				SdReadBlock(0, Sec, 2);
				for(i = 0; i < 1024; i++)
				{
					if(i == 512)
						DBG("\n");
					DBG("%02X ", Sec[i]);
				}
				break;
			case 'w'://阻塞式写2个扇区
				i = SdWriteBlock(2, Sec, 2);
				if(i == 0)
				{
					DBG("write success\n");
				}
				break;
			case 'R'://非阻塞式读2个扇区
				SdCardReadBlockStart(0, Sec);
				for(i = 0; i < 512; i++)
				{
					DBG("%02X ", Sec[i]);
				}
				SdCardReadBlockNext(Sec);
				while(!SdioIsDatTransDone())
				{
					DBG("-");//can do some other thing
				}
				DBG("\n");
				for(i = 0; i < 512; i++)
				{
					DBG("%02X ", Sec[i]);
				}
				SdCardReadBlockEnd();
				break;
			case 'W'://非阻塞式写2个扇区
				i = 0;
				i += SdCardWriteBlockStart(1);
				i += SdCardWriteBlockNext(Sec);//write data to sector 1
				while(!SdioIsDatTransDone())
				{
					DBG("~");//can do some other thing
				}
				i += SdCardWriteBlockNext(Sec);//write data to sector 2
				while(!SdioIsDatTransDone())
				{
					DBG("-");//can do some other thing
				}
				i += SdCardWriteBlockEnd();	
				if(i == 0)
				{
					DBG("write success\n");
				}
				break;
			case 'd':
				SdioSendCommand(18/*CMD18_READ_MULTIPLE_BLOCK*/, 0, 20);
				SdioStartReciveData(Sec, 512/*SD_BLOCK_SIZE*/);
				while(!SdioIsDatTransDone());
				SdioStartReciveData(&Sec[512], 512/*SD_BLOCK_SIZE*/);
				while(!SdioIsDatTransDone());
				SdioEndDatTrans();
				SdioSendCommand(12/*CMD12_STOP_TRANSMISSION*/, 0, 20);
				for(i = 0; i < 1024; i++)
				{
					if(i == 512)
						DBG("\n");
					DBG("%02X ", Sec[i]);
				}
				break;
			case 'x':
				i = 0;
				i += SdioSendCommand(25/*CMD25_WRITE_MULTIPLE_BLOCK*/, 1, 20);
				SdioStartSendData(Sec, 512/*SD_BLOCK_SIZE*/);
				while(!SdioIsDatTransDone());
				SdioStartSendData(&Sec[512], 512/*SD_BLOCK_SIZE*/);
				while(!SdioIsDatTransDone());
				SdioEndDatTrans();
				i += SdioSendCommand(12/*CMD12_STOP_TRANSMISSION*/, 0, 20);
				if(i == 0)
				{
					DBG("write success\n");
				}
				break;
			default:
				break;
		}
		ClrKeyValue();
		DBG("\n");
		DBG("please input command as the following\n");
		DBG("Command   Discription\n");
		//SD卡接口
		DBG("r   		阻塞式读取2个扇区(0,1)\n");
		DBG("w   		阻塞式写2个扇区(2,3)\n");
		DBG("R   		非阻塞式读扇区(0,1)\n");
		DBG("W   		非阻塞式写扇区(2,3)\n");
		//SDIO接口
		DBG("d   		SDIO接口读扇区(2,3)\n");
		DBG("x   		SDIO接口写扇区(2,3)\n");
		DBG("\n");
	}
}

void FuartInterrupt(void)
{
    if(IsFuartRxIntSrc())
    {
        uint8_t c;
        FuartRecvByte(&c);
        FuartClrRxInt(); 
        SetKeyValue(c);
	}
	if(IsFuartTxIntSrc())
    {
        FuartClrTxInt();
	}
}

