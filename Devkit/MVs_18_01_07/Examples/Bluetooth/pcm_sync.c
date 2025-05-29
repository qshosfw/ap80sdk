/*
 *****************************************************************************
 *					Mountain View Silicon Tech. Inc.
 *	Copyright 2013, Mountain View Silicon Tech. Inc., ShangHai, China
 *					All rights reserved.
 *
 * Filename:		pcm_sync.c
 * Description:		PCM MODULE DRIVER FOR BLUETOOTH
 *
 * Change History:
 *        Justin	- 05/30/2013 - V0.1
 *					- created
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "type.h"
#include "i2s.h"
#include "rst.h"
#include "adc.h"
#include "dac.h"
#include "clk.h"
#include "gpio.h"
#include "irqs.h"
#include "delay.h"
//#include "debug.h"
#include "timeout.h"
#include "blue_aec.h"
#include "pcm_sync.h"
#include "pcm_fifo.h"
#include "chip_info.h"
#include "audio_adc.h"
#include "audio_path.h"
#include "pcm_transfer.h"
#include "app_config.h"

#define ADC_DATA_BUFFER_SIZE   (BLK_LEN*2)
#define I2S_DATA_BUFFER_SIZE   (BLK_LEN*2)

uint8_t AdcDataBuffer[ADC_DATA_BUFFER_SIZE]         __ATTRIBUTE__(AT(VMEM_ADDR));
uint8_t I2sInDataBuffer[I2S_DATA_BUFFER_SIZE]       __ATTRIBUTE__(AT(VMEM_ADDR + ADC_DATA_BUFFER_SIZE));
uint8_t I2sOutDataBuffer[I2S_DATA_BUFFER_SIZE * 2]  __ATTRIBUTE__(AT(VMEM_ADDR + ADC_DATA_BUFFER_SIZE + I2S_DATA_BUFFER_SIZE));

BlueAECContext* gBlueAecContext = (BlueAECContext*)(VMEM_ADDR + (BLK_LEN * 2) * 4);

static bool PcmSyncInitFlag = FALSE;

void SetPcmSyncInitFlag(bool flag)
{
    PcmSyncInitFlag = flag;
}

//
// AecStatus
// 0: disable phone and mv aec;
// 1: disable phone ace, enable mv ace;
// 2: enable phone aec, disable mv aec
// 3: enable phone aec, enable mv aec
//
//default value, app-callback 函数内部enable开关可以打开
static uint8_t AecStatus = 0;

void PcmSyncAecSetStatus(uint8_t Status)
{
    AecStatus = Status;
}

uint8_t PcmSyncAecGetStatus(void)
{
    return  AecStatus;
}

void PcmSyncPlayInit(void)
{
    memset(((uint8_t*)I2sInDataBuffer),0,BLK_LEN*6);

    NVIC_DisableIRQ(DECODER_IRQn);

    // Transfer PCM data to FIFO
    PcmTxInitWithDefaultPcmFifoSize(DATA_MODE_MONO_LOW);  

    PcmFifoInit(1024 * 11, 2048);

    PcmTxTransferData((void*)I2sOutDataBuffer, (void*)I2sOutDataBuffer, BLK_LEN);
}

void PcmSyncInit(uint8_t I2sClkType)
{    
    if(!PcmSyncInitFlag)
    {
        GpioPcmSyncIoConfig(1); //0:B24/25/26/27
        RstDecModule();
        PhubPathClose(ALL_PATH_CLOSE);

        if(!PhubPathSel(ADCIN2PMEM_IISIN2DACOUT_PCMFIFO2IISOUT))
        {
            DBG("phub path sel err!\n");
            return;
        }

        //config ADC
        CodecAdcAnaInit();
        CodecAdcChannelSel(ADC_CH_MIC_L | ADC_CH_MIC_R);
        CodecAdcMicGainConfig(0x0, 0x0);
        AdcVolumeSet(0xFFF, 0xFFF);

        AdcPmemWriteDis();
		AdcToPmem(USB_MODE, 0x3400, 0x800);
        AdcPmemWriteEn();

        //config I2s to dac
        I2sWritePmemDis();
    	I2sInToPmemConfig(0x3C00, 0x800);
        I2sWritePmemEn();

        if(MV_MCU_AS_PCMSYNC_CLK_MASTER == I2sClkType)
        {
            I2sSampleRateSet(8000, USB_MODE);
            I2sBluetoothMasterModeSet(I2S_DATA_FROM_DEC, USB_MODE, I2S_DSP_MODEA);
        }
        else
        {
            I2sSampleRateSet(8000, USB_MODE);
            I2sBluetoothSlaveModeSet(I2S_DATA_FROM_DEC, USB_MODE, I2S_DSP_MODEA);
        }

        CodecDacChannelSel(DAC_CH_DECD_L | DAC_CH_DECD_R); // select dac channel
        DacAdcSampleRateSet(8000, USB_MODE);	 //九种采样率
        DacConfig(MDAC_DATA_FROM_I2S, USB_MODE); 

        CodecDacMuteSet(FALSE, FALSE);
        PcmSyncPlayInit(); 

        blue_aec_init(gBlueAecContext, 1, 0);
    }

    WaitMs(8);
    
    PcmSyncInitFlag = TRUE;
}

//
// Process Pcm Data
//
void PcmSyncDataProcess(void)
{
    if(PcmSyncInitFlag)
    {
        if(PcmTxIsTransferDone())
		{
			uint32_t Size = 0;
            uint32_t i = 0, pos = 0;
            TIMER timer;    
            TimeOutSet(&timer, 20);

			PcmTxDoneSignalClear();

            while((Size < BLK_LEN) && (!IsTimeOut(&timer)))
            {
                Size += AdcPcmDataRead(AdcDataBuffer + Size * 2, BLK_LEN - Size, 1);
            }

            Size = 0;
            pos  = 0;
            TimeOutSet(&timer, 20);
            while((Size < BLK_LEN)&& (!IsTimeOut(&timer)))
            {
                Size += I2sPcmDataRead(I2sInDataBuffer + Size * 2, BLK_LEN - Size, 1);
            }

            if((AecStatus == 3) || (AecStatus == 1))
            {
                blue_aec_run(gBlueAecContext, (int16_t*)I2sInDataBuffer, (int16_t*)AdcDataBuffer, (int16_t*)I2sOutDataBuffer);
            }
            else
            {
                memcpy(I2sOutDataBuffer,AdcDataBuffer,BLK_LEN * 2);
            }

            pos = I2S_DATA_BUFFER_SIZE - 2;

            for(i = I2S_DATA_BUFFER_SIZE*2 - 4; i > 0; i -= 4,pos -= 2)
            {
                *((uint16_t*)&I2sOutDataBuffer[i]) = *((uint16_t*)&I2sOutDataBuffer[pos]);
            }

            PcmTxTransferData((void*)I2sOutDataBuffer, (void*)I2sOutDataBuffer, BLK_LEN);
		}
    }
}

//
// Uninitialize I2s data in/out、adc
//
void PcmSyncUnInit(void)
{
    if(PcmSyncInitFlag)
    {
        NVIC_EnableIRQ(DECODER_IRQn);

        PcmSyncInitFlag = FALSE;

        CodecDacMuteSet(TRUE, TRUE);
		CodecAdcAnaDeInit();
        AdcClose(); 
    }
}
