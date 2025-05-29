///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2014, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: SpimDma.c
///////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include "type.h"
#include "clk.h"
#include "rst.h"
#include "gpio.h"
#include "uart.h"
#include "cache.h"
#include "watchdog.h"
#include "spim.h"
#include "spis.h"

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

uint8_t SpimSendBuf[100] =  {
                                0xA5, 0x5A, 0x41, 0xE8, 0x29, 0x48, 0x08, 0x88, 0xCE, 0xE8, 0xBF, 0x0C, 0xC3, 0xF3, 0x62, 0xEE, 
                                0x0F, 0x0D, 0xA7, 0xB1, 0xFF, 0x03, 0x01, 0x41, 0xA2, 0xFC, 0xC3, 0x26, 0x3C, 0x11, 0x27, 0x1D, 
                                0x91, 0xA5, 0x22, 0x44, 0x05, 0x19, 0xC1, 0x21, 0x18, 0xAC, 0x56, 0x4F, 0x21, 0x59, 0x3A, 0x34, 
                                0x65, 0x05, 0x02, 0x83, 0x2D, 0x3C, 0x90, 0xB8, 0xAC, 0x9C, 0xA1, 0x21, 0x77, 0xB7, 0x02, 0x32, 
                                0x74, 0x6D, 0xE2, 0xE8, 0xC5, 0x02, 0x86, 0x10, 0x39, 0x72, 0x32, 0x7A, 0x46, 0xD9, 0x03, 0x85, 
                                0x6D, 0x05, 0x0C, 0xB6, 0xDC, 0x08, 0xCD, 0x88, 0xC4, 0xE4, 0x0A, 0x93, 0xA0, 0x04, 0x0F, 0x0A, 
                                0xEA, 0xDB, 0x5D, 0xE4       
                            } ;


uint8_t SpisSendBuf[100] = {

                                0xA5, 0x5A, 0x18, 0x63, 0x17, 0x47, 0x24, 0x19, 0xD2, 0x04, 0x09, 0x3A, 0x89, 0xA3, 0xA0, 0x00, 
                                0x55, 0x66, 0x77, 0x86, 0x9F, 0xEB, 0x6C, 0x4F, 0xFF, 0xF7, 0x3D, 0x11, 0x0B, 0xCE, 0x80, 0x14, 
                                0x44, 0xE2, 0x7B, 0x87, 0x03, 0x3D, 0xDC, 0xDC, 0xDD, 0xDD, 0xE0, 0x00, 0x83, 0xE2, 0x26, 0x61, 
                                0x04, 0x3B, 0xC0, 0x00, 0x02, 0xEB, 0xA7, 0xB8, 0x1B, 0xC4, 0x2F, 0x82, 0x11, 0x11, 0x00, 0x13, 
                                0xBB, 0x7B, 0xBB, 0xDC, 0x87, 0x3C, 0x9A, 0x65, 0x10, 0x8F, 0x1F, 0x59, 0x07, 0xBF, 0x9F, 0x58, 
                                0x82, 0x1C, 0x10, 0x84, 0x59, 0xE0, 0x60, 0x6E, 0xE2, 0xE2, 0x1E, 0xE6, 0x88, 0x89, 0xA5, 0x74, 
                                0x44, 0xD0, 0x43, 0xab, 
                          };
uint8_t SpisRecvBuf[200];
uint8_t SpimRecvBuf[200];
uint8_t DmaTxRxFlg = 0;



int32_t main(void) 
{    
    uint8_t Buf[10];
    
    ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating

	ClkPorRcToDpll(0);                                  //clock src is 32768hz OSC 
    CacheInit();    
	/*
	 * Now, uart has been initialized automatically by OS.
	 * if you want to customize debug uart setting(defaut 115200-8-N-1),
	 * you can invoke the function at any place you want(for example here).
	 */
	GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);
	FuartInit(115200, 8, 0, 1);
    
    /* Config spim pin: spim_mosi b[5], spim_clk  b[4], spim_miso b[3] */
    GpioSpimIoConfig(2);
    /* Config spis pin: spis_mosi b[20], spis_clk  b[21], spis_miso b[22], spis_cs b[23] */
    GpioSpisIoConfig(0);
    GpioSetRegOneBit(GPIO_A_OE, GPIOA9);	  //a9-->slave cs
	GpioClrRegOneBit(GPIO_A_IE, GPIOA9);
	GpioSetRegOneBit(GPIO_A_OUT, GPIOA9);

    APP_DBG("\n");
    APP_DBG("/==========================================================================\\\n");
    APP_DBG("|                   SPIM DMA EXAMPLE TESTBENCH                                   |\n");
    APP_DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    APP_DBG("\\==========================================================================/\n");
    APP_DBG("\n");
    APP_DBG(" Please send num to sel test item\n");
    APP_DBG(" 1: Spi master dma --> spi slave\n");
    APP_DBG(" 2: Spi slave      --> spi master dma\n");
    APP_DBG("\n");
   
    SpiMasterInit(3, 2);                   //Init spim para:  CPOL = 1 & CPHA = 1, spim clk = 96M/2^2 = 24Mhz
    SpiSlaveInit(3, 0, 0x200, 0x100);      //Init spis para:   
    NVIC_EnableIRQ(SPIM_IRQn);

    while(1)
    {
        WdgFeed();
        if(-1 != FuartRecvByte(Buf))
        { 
            switch(Buf[0])
            {
                case '1':
                    APP_DBG("Test spi master dma send\n");
                    GpioClrRegOneBit(GPIO_A_OUT, GPIOA9);
                    SpisClrRxBuff();
                    SpiMasterStartData(SpimSendBuf, 100, 0); 
                    DmaTxRxFlg = 1;
                    break;
                    
                case '2':
                    APP_DBG("Test spi master dma receive\n");
                    GpioClrRegOneBit(GPIO_A_OUT, GPIOA9);
                    SpiSlaveSend(SpisSendBuf, 100, 200);
                    memset(SpimRecvBuf, 0, sizeof(SpimRecvBuf));
                    SpiMasterStartData(SpimRecvBuf, 110, 1);
                    DmaTxRxFlg = 2;
                    break;
                    
                default:
                    break;    
            }
                       
        } 
    } 
}
void SpimInterrupt(void)
{
    uint32_t i;
    uint32_t RevLen = 0;
    
    SpiMasterIntClr();

    if(1 == DmaTxRxFlg)
    {
        DmaTxRxFlg = 0;
        RevLen = SpiSlaveRxDataLenGet();
        memset(SpisRecvBuf, 0, sizeof(SpisRecvBuf));
        SpiSlaveReceive(SpisRecvBuf, RevLen, 200); 
        for(i=0; i<100; i++)
        {
            if(i%8 == 0)
            {
                APP_DBG("\n");
            }               
            APP_DBG("%4x ", SpisRecvBuf[i]);
            
        } 
        APP_DBG("\n");  
    }
    if(2 == DmaTxRxFlg)
    {
        DmaTxRxFlg  =  0;
        for(i=2; i<102; i++)
        {
            if(i%8 == 0)
            {
                APP_DBG("\n");
            }               
            APP_DBG("%4x ", SpimRecvBuf[i]);  
        } 
        APP_DBG("\n"); 
    }   
}


//
