#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "type.h"
#include "clk.h"
#include "uart.h"
#include "gpio.h"
#include "cache.h"
#include "delay.h"
#include "timer.h"
#include "watchdog.h"
#include "sdram.h"
#include "timeout.h"

#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define	CFG_SDRAM_BASE	(0x60000000)
static uint8_t Sram[4 * 1024];

//Capacity(2MB/8MB/16MB),Vendor(Samsung,EtronTech,Hynix,Micron,ESMT)
//Type X Cap X ROW X COL X BANKS
//Micron 	MT48LC1M16A2 X 2MB X 2048 X 256 X 2
//Micron 	MT48LC4M16A2 X 8MB X 4096 X 256 X 4
//Micron 	MT48LC8M16A2 X 16MB X 4096 X 512 X 4

//Hynix 	HY57V281620 X 16MB X 4096 X 512 X 4
//Hynix 	HY57V641620 X 8MB X 4096 X 256 X 4
//Hynix 	HY57V161610D X 2MB X 2048 X 256 X 2

//Samsung 	K4S281632K X 16MB X 4096 X 512 X 4
//Samsung 	K4S641632H X 8MB X 4096 X 256 X 4
//Samsung 	K4S161622D X 2MB X 2048 X 256 X 2

//ESMT 		EM639165TS X 16MB X 4096 X 512 X 4
//ESMT 		EM638165TS X 8MB X 4096 X 256 X 4

//ESMT 		M5S2128188 X 16MB X 4096 X 512 X 4

//extern void SysTickInit(void);

int main(void)
{
	int i;
	long TmCnt;
	TIMER Tmr;
	unsigned char* pByte = (uint8_t*)CFG_SDRAM_BASE;
	unsigned short* pWord = (unsigned short*)CFG_SDRAM_BASE;
	unsigned int* pDword = (unsigned int *)CFG_SDRAM_BASE;
	unsigned long Size,Loops,Random;
	
	
	ClkPorRcToDpll(0);
    CacheInit();
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);	

//	GpioFuartRxIoConfig(1);
	//NOTE:
	//connect the GPIO GPA0 to UART_TXD pad to output the debug information
	//connect the USB cable from PC to J15 for USB2UART socket
	GpioFuartTxIoConfig(0);
	FuartInit(115200, 8, 0, 1);

	SysTickInit();
	//Disable Watchdog
    WaitMs(200);
	WdgDis();
	
	//NOTE:
	//GPB0---GPB31,GPA26---GPA31 used by SDRAM,you cannot occupy this gpio pin
	//all the GPIOS above jumpers must be removed
	GpioSdramIoConfig(0);
	//SdramInit(0);
    SdramInit();
	Size = 0x800000;//EM6381 capacity is 8 Mega byte,see capacity list above
	
	DBG("Start to test sdram\n");
	do
	{
		/*
		 * case 3.1.1.1
		 */
		TimeOutSet(&Tmr, 0);
	
		pByte = (uint8_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size;i ++){
			*pByte ++ = 0xA5;
		}
	
		pByte = (uint8_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size;i ++){
			if(pByte[i] != 0xA5){
				DBG("SDRAM test case 3.1.1.1 failed\n");
				break;
			}
		}
	
		TmCnt = PastTimeGet(&Tmr);
		DBG("SDRAM test case 3.1.1.1 during time %d\n",TmCnt);
	
		/*
		 * case 3.1.1.2
		 */
		TimeOutSet(&Tmr, 0);
		pByte = (uint8_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size;i ++){
			*pByte ++ = 0x5A;
		}
		pByte = (uint8_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size;i ++){
			if(pByte[i] != 0x5A){
				DBG("SDRAM test case 3.1.1.2 failed\n");
				break;
			}
		}
		TmCnt = PastTimeGet(&Tmr);
		DBG("SDRAM test case 3.1.1.2 during time %d\n",TmCnt);
	
		/*
		 * case 3.1.1.3
		 */
		TimeOutSet(&Tmr, 0);
		pWord = (uint16_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size / 2;i ++){
			*pWord ++ = 0xA5A5;
		}
		pWord = (uint16_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size / 2;i ++){
			if(pWord[i] != 0xA5A5){
				DBG("SDRAM test case 3.1.1.3 failed\n");
				break;
			}
		}
		TmCnt = PastTimeGet(&Tmr);
		DBG("SDRAM test case 3.1.1.3 during time %d\n",TmCnt);
	
		/*
		 * case 3.1.1.4
		 */
		TimeOutSet(&Tmr, 0);
		pWord = (uint16_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size / 2;i ++){
			*pWord ++ = 0x5A5A;
		}
		pWord = (uint16_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size / 2;i ++){
			if(pWord[i] != 0x5A5A){
				DBG("SDRAM test case 3.1.1.4 failed\n");
				break;
			}
		}
		TmCnt = PastTimeGet(&Tmr);
		DBG("SDRAM test case 3.1.1.4 during time %d\n",TmCnt);
	
		/*
		 * case 3.1.1.5
		 */
		TimeOutSet(&Tmr, 0);
		pDword = (uint32_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size / 4;i ++){
			*pDword ++ = 0xAAAAAAAA;
		}
		pDword = (uint32_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size / 4;i ++){
			if(pDword[i] != 0xAAAAAAAA){
				DBG("SDRAM test case 3.1.1.5 failed\n");
				break;
			}
		}
		TmCnt = PastTimeGet(&Tmr);
		DBG("SDRAM test case 3.1.1.5 during time %d\n",TmCnt);
	
		/*
		 * case 3.1.1.6
		 */
		TimeOutSet(&Tmr, 0);
		pDword = (uint32_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size / 4;i ++){
			*pDword ++ = 0x55555555;
		}
		pDword = (uint32_t*)CFG_SDRAM_BASE;
		for(i = 0;i < Size / 4;i ++){
			if(pDword[i] != 0x55555555){
				DBG("SDRAM test case 3.1.1.6 failed\n");
				break;
			}
		}
		TmCnt = PastTimeGet(&Tmr);
		DBG("SDRAM test case 3.1.1.6 during time %d\n",TmCnt);
	
		/*
		 * case 3.1.1.7
		 */
		TimeOutSet(&Tmr, 0);
		pWord = (uint16_t*)(CFG_SDRAM_BASE + 1);
		for(i = 1;i + 2 < Size;i += 2){
			*pWord ++ = 0xA5A5;
		}
		pWord = (uint16_t*)(CFG_SDRAM_BASE + 1);
		for(i = 1;i + 2 < Size;i += 2){
			if(*pWord ++ != 0xA5A5){
				DBG("SDRAM test case 3.1.1.7.1 failed\n");
				break;
			}
		}
	
		pWord = (uint16_t*)(CFG_SDRAM_BASE + 1);
		for(i = 1;i + 2 < Size;i += 2){
			*pWord ++ = 0x5A5A;
		}
	
		pWord = (uint16_t*)(CFG_SDRAM_BASE + 1);
		for(i = 1;i + 2 < Size;i += 2){
			if(*pWord ++ != 0x5A5A){
				DBG("SDRAM test case 3.1.1.7.2 failed\n");
				break;
			}
		}
		TmCnt = PastTimeGet(&Tmr);
		DBG("SDRAM test case 3.1.1.7 during time %d\n",TmCnt);
	
//		/*
//		 * case 3.1.1.8
//		 */
//		TimeOutSet(&Tmr, 0);
//		for(j = 1;j < 4;j ++){
//			pDword = (uint32_t*)(CFG_SDRAM_BASE + j);
//			for(i = j;i + 4 < Size;i += 4){
//				*pDword ++ = 0xAAAAAAAA;
//			}
//			pDword = (uint32_t*)(CFG_SDRAM_BASE + j);
//			for(i = j;i + 4 < Size;i += 4){
//				if(*pDword != 0xAAAAAAAA){
//					DBG("SDRAM test case 3.1.1.8 failed\n");
//					break;
//				}
//				pDword ++;
//			}
//		}
//		TmCnt = PastTimeGet(&Tmr);
//		DBG("SDRAM test case 3.1.1.8 during time %d\n",TmCnt);
		
	
		/*
		 * case 3.1.1.9
		 * random read & write for SDRAM solely
		 */
		Loops = 100000;
		TimeOutSet(&Tmr, 0);
		while(Loops --){
			Random = (rand() << 16 | rand()) % Size;
			Sram[0]  = rand() % 0xFF;Sram[1] = rand() % 0xFF;Sram[2] = rand() % 0xFF;Sram[3] = 0x11;
			Sram[4]  = rand() % 0xFF;Sram[5] = rand() % 0xFF;Sram[6] = rand() % 0xFF;Sram[7] = 0xFC;
			Sram[8]  = rand() % 0xFF;Sram[9] = rand() % 0xFF;Sram[10] = rand() % 0xFF;Sram[11]=0xD8;
			Sram[12] = rand() % 0xFF;Sram[13] = rand() % 0xFF;Sram[14]=rand() % 0xFF;Sram[15]=0xFF;
			Random &= Size - 1;
			Random |= CFG_SDRAM_BASE;
			Random &= ~0xF;
	    
			for(i = 0,pByte = (uint8_t*)Random;i < 16;i ++){
				pByte[i] = Sram[i];
			}
			for(i = 0,pByte = (uint8_t*)Random;i < 16 && Loops;i ++){
				if(pByte[i] != Sram[i]){
					DBG("SDRAM test case 3.1.1.9.1 failed<0x%02X!=0x%02X\n",pByte[i],Sram[i]);
					Loops = 0;
					break;
				}
			}
	
			memset(pByte,0,16);
			for(i = 0,pWord = (uint16_t*)Random;i < 8 && Loops;i ++){
				pWord[i] = ((uint16_t*)Sram)[i];
			}
			for(i = 0,pWord = (uint16_t*)Random;i < 8 && Loops;i ++){
				if(pWord[i] != ((uint16_t*)Sram)[i]){
					DBG("SDRAM test case 3.1.1.9.2 failed<0x%04X!=0x%04X\n",pWord[i],Sram[i]);
					Loops = 0;
					break;
				}
			}
	
			memset(pWord,0,16);
			for(i = 0,pDword = (uint32_t*)Random;i < 4 && Loops;i ++){
				pDword[i] = ((uint32_t*)Sram)[i];
			}
			for(i = 0,pDword = (uint32_t*)Random;i < 4 && Loops;i ++){
				if(pDword[i] != ((uint32_t*)Sram)[i]){
					DBG("SDRAM test case 3.1.1.9.3 failed<0x%08X!=0x%08X\n",pDword[i],Sram[i]);
					Loops = 0;
					break;
				}
			}
		}
		
		TmCnt = PastTimeGet(&Tmr);
		DBG("SDRAM test case 3.1.1.9 during time %d\n",TmCnt);
	}while(1);
	
	
//	return 0;
}

