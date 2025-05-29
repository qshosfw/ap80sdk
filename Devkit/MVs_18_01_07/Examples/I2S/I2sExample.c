///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2014, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: AudioPath.c
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "type.h"
#include "clk.h"
#include "rst.h"
#include "gpio.h"
#include "uart.h"
#include "cache.h"
#include "i2s.h"
#include "pmem.h"
#include "audio_adc.h"
#include "dac.h"
#include "watchdog.h"
#include "chip_info.h"
#include "audio_path.h"
#include "pcm_fifo.h"
#include "pcm_transfer.h"

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define ADC_FIFO_ADDR  PMEM_ADDR
#define ADC_FIFO_LEN   (4 * 1024)

#define I2S_FIFO_ADDR  (ADC_FIFO_ADDR + ADC_FIFO_LEN)
#define I2S_FIFO_LEN   (4 * 1024)

#define PCM_FIFO_ADDR  (I2S_FIFO_ADDR + I2S_FIFO_LEN)
#define PCM_FIFO_LEN   (10 * 1024)


#define DEC_MEM_ADDR    VMEM_ADDR
#define DEC_MEM_MAX_SIZE (20 * 1024)

int32_t main(void)
{
    uint8_t     Buf[10];
    uint32_t    uSampleRate = 12000;
    uint32_t    ReadLen = 192;
    
    uint8_t gPathState = 0;
    uint8_t PcmTxFlg =0;    
    uint8_t *SendBuf = (uint8_t *)DEC_MEM_ADDR;
    
    
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
    
    /*
     * Config i2s pin: 
     * i2s_lrck(io)----b[3], i2s_bclk(io)--- b[4], 
     * i2s_din(i)----- b[6], i2s_do(o)-------b[5]
     */
    GpioI2sIoConfig(0); 
    
    APP_DBG("\n");
    APP_DBG("/==========================================================================\\\n");
    APP_DBG("|                   I2S EXAMPLE TESTBENCH                                   |\n");
    APP_DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    APP_DBG("\\==========================================================================/\n");
    APP_DBG("\n\n");
    APP_DBG(" Please send num to sel i2s mode: \n");
    APP_DBG("\n 1: Line in --> I2s Out:\n");
    APP_DBG("    Master, Mclk = 12MHZ, SampleRate = %d, i2s justified format\n", uSampleRate);
    
    APP_DBG("\n 2: Line in --> I2s Out:\n\n");
    APP_DBG("    Slave,  Mclk=12.288/11.2896Mhz, SampleRate = %d, I2s justified format\n", uSampleRate);
    
    APP_DBG("\n 3: I2s in --> STA Classd Out:\n\n");
    APP_DBG("    Slave,  Mclk=12.288/11.2896Mhz, SampleRate = %d, I2s justified format\n", uSampleRate);
    
    APP_DBG("\n 4: I2s  in --> Dac Out:\n\n");
    APP_DBG("    Master,  Mclk=12.288/11.2896Mhz, SampleRate = %d, I2s justified format\n", uSampleRate);
    APP_DBG("\n--------------------------------------------------------------------------\n");
    /*line in --> i2s out*/
    CodecAdcAnaInit();					                    //adc init
    CodecDacInit(1);                                        //dac init 
    
   
    AdcPmemWriteDis();
    AdcToPmem(USB_MODE, (uint16_t)(ADC_FIFO_ADDR - PMEM_ADDR), ADC_FIFO_LEN);
    AdcPmemWriteEn(); 


    DacAdcSampleRateSet(uSampleRate, USB_MODE);
    DacVolumeSet(0xfff, 0xfff);

    I2sInToPmemConfig(I2S_FIFO_ADDR - PMEM_ADDR, I2S_FIFO_LEN);//i2s in -->i2s out
    I2sWritePmemEn();
    
    PcmFifoPause();
    PcmFifoInitialize(PCM_FIFO_ADDR-PMEM_ADDR, PCM_FIFO_LEN, 0, 0);
    PcmTxSetPcmDataMode(DATA_MODE_STEREO_LR);
    PcmFifoPlay();
    
    
    while(1)
    {
        if(-1 != FuartRecvByte(Buf))
        {
            switch(Buf[0])
            {
                case '1':
                    APP_DBG("\n  I2s mode: master, i2s justified format, mclk: 12Mhz\n");
                    CodecAdcChannelSel(ADC_CH_LINE_L | ADC_CH_LINE_R);      //line in -->i2s out
                    CodecAdcLineInGainConfig(10, 10); 
                    AdcConfig(USB_MODE);
                    GpioMclkIoConfig(0);                                //mclk port sel
                    DacConfig(MDAC_DATA_FROM_DEC, USB_MODE);
                    DacAdcSampleRateSet(uSampleRate, USB_MODE);
                    PhubPathClose(ALL_PATH_CLOSE);
                    PhubPathSel(ADCIN2PMEM_PCMFIFO2DACOUTI2SOUT);
                    I2sSetMasterMode(I2S_DATA_FROM_DEC, USB_MODE, I2S_FORMAT_I2S);
                    I2sSampleRateSet(uSampleRate, USB_MODE);   
                    Rst12p288GeneratorModule();
                    RstMclk();
                    gPathState = 2;
                    break;
                
                case '2':
                    APP_DBG("\n  I2s mode: slave, i2s justified format, mclk: 12.288/11.2896Mhz\n");
                    CodecAdcChannelSel(ADC_CH_LINE_L | ADC_CH_LINE_R);      //line in -->i2s out
                    CodecAdcLineInGainConfig(10, 10); 
                    AdcConfig(USB_MODE);                
                    GpioMclkIoConfig(2);                                //mclk port sel
                    DacConfig(MDAC_DATA_FROM_DEC, NORMAL_MODE);
                    DacAdcSampleRateSet(uSampleRate, NORMAL_MODE);
                    PhubPathClose(ALL_PATH_CLOSE);
                    PhubPathSel(ADCIN2PMEM_PCMFIFO2DACOUTI2SOUT);
                    I2sSetSlaveMode(I2S_DATA_FROM_DEC, NORMAL_MODE, I2S_FORMAT_I2S);
                    I2sSampleRateSet(uSampleRate, NORMAL_MODE);
                    Rst12p288GeneratorModule();
                    RstMclk();
                    gPathState = 2;
                    break;
                
                case '3':   
                    APP_DBG("\n  I2s mode: master, i2s justified format, mclk: sta mode\n");
                    GpioMclkIoConfig(0);                                //mclk port sel
                    DacConfig(MDAC_DATA_FROM_DEC, NORMAL_MODE);
                    PhubPathClose(ALL_PATH_CLOSE);
                    PhubPathSel(PCMFIFO_TO_IISOUT);
                    if(uSampleRate < 32000)
                    {
                       //由于O18B目前只有一组I2S，所以只能支持I2S输入和I2S输出相同的采样率
                       //本次以外接STA333BW CLASSD芯片为例，由于该数字功放芯片的采样率只支持
                       //到32000HZ，所以如果I2S in的采样率低于32000时，O18B是不支持的，如果
                       //您选择的CLASSD的采样率支持9种采样率，则直接进入else，不会存在采样率
                       //的判断。
                    }
                    else
                    {
                        PhubPathClose(ALL_PATH_CLOSE);
                        PhubPathSel(ADCIN2PMEM_PCMFIFOBEEP2_IISOUT);
                        DacAdcSampleRateSet(uSampleRate, NORMAL_MODE);                    
                        I2sClassdMasterModeSet(I2S_DATA_FROM_DEC, I2S_FORMAT_I2S);
                        I2sClassdSTASampleRateSet(uSampleRate, NORMAL_MODE, 0);  
                    }
                    Rst12p288GeneratorModule();
                    RstMclk();
                    gPathState = 1;
                    break;
                 
                case '4':
                    APP_DBG("\n  I2s mode: master, i2s justified format, mclk: 12.288/11.2896Mhz\n");
                    GpioMclkIoConfig(0);                                //mclk port sel
                    CodecDacChannelSel(DAC_CH_DECD_L | DAC_CH_DECD_R);
                    DacConfig(MDAC_DATA_FROM_DEC, NORMAL_MODE);
                    DacAdcSampleRateSet(uSampleRate, NORMAL_MODE);
                    PhubPathClose(ALL_PATH_CLOSE);
                    PhubPathSel(PCMFIFO_MIX_DACOUT);
                    gPathState = 1;
                    break;
                
                default:
                    gPathState = 0;
                    break;
            }
            PcmTxFlg = 0;
        }
        else
        {
            if(1 == gPathState)
            {
                if(PcmTxIsTransferDone() || !PcmTxFlg)
                {
                    if(I2sPmemPcmRemainLenGet() >= ReadLen)
                    {
                        I2sPcmDataRead(SendBuf, ReadLen, 0x3);
                        PcmTxTransferData(SendBuf, SendBuf, ReadLen); 
                        PcmTxFlg = TRUE;
                    }
                }
            }
            if(2 == gPathState)
            {
                if(PcmTxIsTransferDone() || !PcmTxFlg)
                {                                                                      
                    if(AdcPmemPcmRemainLenGet() >= ReadLen)
                    {
                        AdcPcmDataRead(SendBuf, ReadLen, 0x3);
                        PcmTxTransferData(SendBuf, SendBuf, ReadLen); 
                        PcmTxFlg = TRUE;
                    }
                }
            }
        }
        WdgFeed();
    }
  
}
//
