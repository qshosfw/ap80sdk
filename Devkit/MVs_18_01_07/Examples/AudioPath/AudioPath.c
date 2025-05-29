///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2014, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: AudioPath.c
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "type.h"
#include "clk.h"
#include "gpio.h"
#include "uart.h"
#include "cache.h"
#include "i2s.h"
#include "audio_path.h"
#include "audio_adc.h"
#include "dac.h"
#include "pcm_fifo.h"
#include "pcm_transfer.h"
#include "watchdog.h"
#include "AudioPath.h"

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

int8_t gPathState = STATUS_IDLE;
uint8_t *SendBuf = (uint8_t *)DEC_MEM_ADDR;

int32_t main(void)
{
    uint8_t     Buf[10];
    uint32_t    ReadLen = 192;
    bool        PcmTxFlg = FALSE;
    
    ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating

	ClkPorRcToDpll(0);                                  //clock src is 32768hz OSC 
    CacheInit();    
	/*
	 * Now, uart has been initialized automatically by OS.
	 * if you want to customize debug uart setting(defaut 115200-8-N-1),
	 * you can invoke the function at any place you want(for example here).
	 */
	GpioFuartRxIoConfig(0);
	GpioFuartTxIoConfig(0);
	FuartInit(115200, 8, 0, 1);
    APP_DBG("\n");
    APP_DBG("/==========================================================================\\\n");
    APP_DBG("|                   AUDIO PATH TESTBENCH                                   |\n");
    APP_DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    APP_DBG("\\==========================================================================/\n");
    APP_DBG("\n\n");
    APP_DBG(" ==============================================\n");
    APP_DBG(" Please send num to sel audio path\n");
    APP_DBG(" Path 0: Line in to Line out\n");
    APP_DBG(" Path 1: Line in to I2s out\n");
    APP_DBG(" Path 2: Line in to Line out and I2s out\n");
    APP_DBG(" Path 3: I2s in to I2s out\n");
    APP_DBG(" Path 4: I2s in to Line out\n");
    APP_DBG(" Path 5: Line in to I2s out and I2s in to Line out\n");
    APP_DBG(" ==============================================\n");
    
    GpioI2sIoConfig(0);                                 //i2s port sel
    GpioMclkIoConfig(0);                                //mclk port sel
    
    CodecAdcAnaInit();					                //adc init
    CodecDacInit(1);                                    //dac init 
    CodecAdcChannelSel(ADC_CH_LINE_L | ADC_CH_LINE_R);  //line in 
    CodecAdcLineInGainConfig(10, 10); 
    AdcConfig(USB_MODE);
    
    CodecDacChannelSel(DAC_CH_DECD_L | DAC_CH_DECD_R);
    DacConfig(MDAC_DATA_FROM_DEC, USB_MODE);
    DacAdcSampleRateSet(44100, USB_MODE);
    DacVolumeSet(0xef0, 0xef0); 
    
    PcmFifoPause();
    PcmFifoInitialize(PCM_FIFO_ADDR-PMEM_ADDR, PCM_FIFO_LEN, 0, 0);
    PcmTxSetPcmDataMode(DATA_MODE_STEREO_LR);
    PcmFifoPlay();
    
    AdcPmemWriteDis();
    AdcToPmem(USB_MODE, (uint16_t)(ADC_FIFO_ADDR - PMEM_ADDR), ADC_FIFO_LEN);
    AdcPmemWriteEn();
    
    I2sSetMasterMode(I2S_DATA_FROM_DEC, USB_MODE, I2S_FORMAT_I2S);
    I2sSampleRateSet(44100, USB_MODE);
    I2sWritePmemDis();
    I2sInToPmemConfig((uint16_t)(I2S_FIFO_ADDR-PMEM_ADDR),I2S_FIFO_LEN);
    I2sWritePmemEn();
    
    while(1)
    {
        if(-1 != FuartRecvByte(Buf))
        {
            DacSoftMuteSet(TRUE, TRUE); 
            switch(Buf[0])
            {
                case '0':   
                    APP_DBG("\n Current status: line in --> line out\n");
                    PhubPathClose(ALL_PATH_CLOSE);
                    PhubPathSel(ADCIN2PMEM_PCMFIFO2DACOUT);
                    DacSoftMuteSet(FALSE, FALSE); 
                    gPathState = STATUS_ADCIN_BUSY;                
                    break;
                    
                case '1': 
                    APP_DBG("\n Current status: line in --> i2s out\n");                    
                    PhubPathClose(ALL_PATH_CLOSE);
                    PhubPathSel(ADCIN2PMEM_PCMFIFO2IISOUT); 
                    gPathState = STATUS_ADCIN_BUSY;                
                    break;
                
                case '2':  
                    APP_DBG("\n Current status: line in --> line out + i2s out\n");                       
                    PhubPathClose(ALL_PATH_CLOSE);
                    PhubPathSel(ADCIN2PMEM_PCMFIFO2DACOUTI2SOUT);
                    DacSoftMuteSet(FALSE, FALSE); 
                    gPathState = STATUS_ADCIN_BUSY;
                    break;
                
                case '3':
                    APP_DBG("\n Current status: i2s in --> i2s out\n");   
                    PhubPathClose(ALL_PATH_CLOSE);
                    PhubPathSel(PCMFIFO_TO_IISOUT);
                    gPathState = STATUS_I2SIN_BUSY;
                    break;
                
                case '4':  
                    APP_DBG("\n Current status: i2s in --> line out\n");                     
                    PhubPathClose(ALL_PATH_CLOSE);
                    PhubPathSel(PCMFIFO_MIX_DACOUT);
                    DacSoftMuteSet(FALSE, FALSE);
                    gPathState = STATUS_I2SIN_BUSY;
                    break;
                
                case '5':
                    APP_DBG("\n Current status: i2s in --> line out; line in --> i2s out\n"); 
                    PhubPathClose(ALL_PATH_CLOSE);
                    PhubPathSel(ADCIN2PMEM_IISIN2DACOUT_PCMFIFO2IISOUT);
                    DacSoftMuteSet(FALSE, FALSE); 
                    gPathState = STATUS_ADCIN_BUSY;
                    break;

                default:
                    gPathState = STATUS_IDLE;
                    break;
            }
            PcmTxFlg = FALSE;
        }
        else
        {          
            switch(gPathState)
            {
                case STATUS_IDLE:
                    break;
                
                case STATUS_ADCIN_BUSY:                    
                    if(PcmTxIsTransferDone() || !PcmTxFlg)
                    {                                                                      
                        if(AdcPmemPcmRemainLenGet() >= ReadLen)
                        {
                            AdcPcmDataRead(SendBuf, ReadLen, 0x3);
                            PcmTxTransferData(SendBuf, SendBuf, ReadLen); 
                            PcmTxFlg = TRUE;
                        }
                    }
                    break;
                
                case STATUS_I2SIN_BUSY:
                    if(PcmTxIsTransferDone() || !PcmTxFlg)
                    {
                        if(I2sPmemPcmRemainLenGet() >= ReadLen)
                        {
                            I2sPcmDataRead(SendBuf, ReadLen, 0x3);
                            PcmTxTransferData(SendBuf, SendBuf, ReadLen); 
                            PcmTxFlg = TRUE;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        WdgFeed();
    }
}
//
