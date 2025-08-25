///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2014, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: AudioPath.c
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
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
#include "codec_effects.h"
#include "codec_mixer.h"
#include "chip_info.h"
#include "spi_flash.h"
#include "communication.h"
#include "delay.h"
#include "rst.h"

int16_t PcmBuf[USER_EFFECT_FRAME_SIZE*2];
uint8_t TmpSendBuf[1024]__ATTRIBUTE__(AT(0x20010000));

int32_t main(void)
{
    uint32_t    ReadLen = 128,i;
		uint8_t     RecvData;
    bool        PcmTxFlg = FALSE;
	
    
		ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
		ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating
	
		ClkPorRcToDpll(0);                                  //clock src is 32768hz OSC 
    CacheInit();  

		WdgFeed();
		
		RstCodecModule();
		DacVolumeSet(0,0);
		DacConfig(MDAC_DATA_FROM_DEC, USB_MODE);
		memset(TmpSendBuf, 0, 256*4);
		PcmTxDoneSignalClear();
		PcmTxTransferData(TmpSendBuf, TmpSendBuf, 256);
		while(!PcmTxIsTransferDone());
		PcmTxDoneSignalClear();
		RstCodecModule();
		DacVolumeSet(0,0);
		
		WaitMs(200);
		WdgDis();
		
		
		#if AUDIO_EFFECT_ONLINE_CONFIG_EN
		AudioEffectOnlineConfigInit();
		#endif
		

	
    CodecAdcAnaInit();					                    //adc init
    CodecDacInit(1);                            		//dac init 
		CodecMicBiasEn(1);
    CodecAdcChannelSel(ADC_CH_MIC_L | ADC_CH_MIC_R);  //mic
		CodecAdcMicGainConfig(15, 15);
	
    //CodecAdcLineInGainConfig(10, 10); 
    AdcConfig(USB_MODE);
    
    CodecDacChannelSel(DAC_CH_DECD_L | DAC_CH_DECD_R);
    DacConfig(MDAC_DATA_FROM_DEC, USB_MODE);
    DacAdcSampleRateSet(USER_EFFECT_SAMPLE_RATE, USB_MODE);
    DacVolumeSet(0xef0, 0xef0); 
    
  
    AdcPmemWriteDis();
    AdcToPmem(USB_MODE, (uint16_t)(AUDIO_ADC_PMEM_ADDR), USER_ADC_DMA_FIFO);
    //AdcPmemWriteEn();
	
		PcmTxInitWithDefaultPcmFifoSize(DATA_MODE_STEREO_LR);// i2sµÄ×óÓÒÉùµÀ·­µô
		PcmFifoInitialize(AUDIO_DAC_PMEM_ADDR, USER_DAC_DMA_FIFO, 0, 0);
		Codec_MixerInit((void *)(AUDIO_DAC_PMEM_ADDR + PMEM_ADDR), USER_DAC_DMA_FIFO);
		Codec_MixerConfigFormat(MIXER_FORMAT_STERO);
			
		PhubPathClose(ALL_PATH_CLOSE);
		PhubPathSel(ADCIN2PMEM_PCMFIFO2DACOUT);
		DacSoftMuteSet(FALSE, FALSE); 
			
		AudioEffectReadParamFromFlash();
		AudioEffectInit(USER_EFFECT_CHANNEL);
		
		AdcPmemWriteEn();
		
		#if AUDIO_EFFECT_ONLINE_CONFIG_EN
		#ifdef FUART_PORT_EN
		GpioFuartRxIoConfig(1);
		GpioFuartTxIoConfig(1);
		FuartInit(115200, 8, 0, 1);
		FuartIOctl(UART_IOCTL_RXFIFO_CLR, 1);
		FuartIOctl(UART_IOCTL_RXINT_CLR, 1);
		#else
		BuartExFifoInit(AUDIO_BUART_RX_FIFO_ADDR, USER_BUART_RX_FIFO, USER_BUART_TX_FIFO, 1);
		GpioBuartRxIoConfig(2);
		GpioBuartTxIoConfig(2);
		BuartInit(115200, 8, 0, 1);
		BuartIOctl(UART_IOCTL_RXFIFO_CLR, 1);
		BuartIOctl(UART_IOCTL_RXINT_CLR, 1);
		BuartIOctl(UART_IOCTL_RXINT_SET, 1);	
		#endif
		#endif
			
		while(1)
		{
			#if AUDIO_EFFECT_ONLINE_CONFIG_EN
			AudioEffectOnlineDataProcess();
			#endif
			
			if(Codec_MixerIsDone())
			{    
				if(AdcPmemPcmRemainLenGet() >= ReadLen)
				{
					 CycleCntStart();
					 
					 AdcPcmDataRead((uint8_t *)PcmBuf, USER_EFFECT_FRAME_SIZE, 0x3);				
					 AudioEffectProcess(PcmBuf, PcmBuf, USER_EFFECT_FRAME_SIZE);
					 Codec_MixerSetData(PcmBuf, USER_EFFECT_FRAME_SIZE); 
					 
					 CPUMcpsValue[CPUMcpsCnt] = CycleCntEnd(USER_EFFECT_FRAME_SIZE, USER_EFFECT_SAMPLE_RATE);
					 if(++CPUMcpsCnt >= CPU_MCPS_BUF_SIZE)
					 {
						CPUMcpsCnt = 0;
					 }
				}
			}
			//WdgFeed();
		} 
}

#if AUDIO_EFFECT_ONLINE_CONFIG_EN
#ifdef FUART_PORT_EN
void FuartInterrupt(void)
{
	uint8_t RecvData;
	
	RecvData = 0;
	if(-1 != FuartRecvByte(&RecvData))
	{
		AudioEffectOnlineDataFeed(RecvData);
	}
	if(-1 != FuartRecvByte(&RecvData))
	{
		AudioEffectOnlineDataFeed(RecvData);
	}
	if(-1 != FuartRecvByte(&RecvData))
	{
		AudioEffectOnlineDataFeed(RecvData);
	}
	if(-1 != FuartRecvByte(&RecvData))
	{
		AudioEffectOnlineDataFeed(RecvData);
	}
	FuartIOctl(UART_IOCTL_RXINT_CLR, 1);
}
#else
void BuartInterrupt(void)
{
	uint8_t RecvData[100];
	int32_t RecvLen;
	
	RecvLen = BuartRecv(RecvData, 100, 0);
	if(RecvLen > 0)
	{
			AudioEffectOnlineDatasFeed(RecvData, RecvLen);
	}
	BuartIOctl(UART_IOCTL_RXINT_CLR, 1);
}
#endif
#endif

