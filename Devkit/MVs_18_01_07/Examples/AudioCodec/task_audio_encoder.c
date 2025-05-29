/**
 **************************************************************************************
 * @file    task_audio_encoder.c
 * @brief   Audio Encoders Library Usage Demonstration
 *
 * @author  Aissen Li
 * @version V1.0.0
 *
 * $Created: 2012-11-21 17:07:10$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <string.h>

#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "chip_info.h"
#include "clk.h"
#include "i2s.h"
#include "adc.h"
#include "dac.h"
#include "audio_path.h"
#include "pmem.h"
#include "gpio.h"
#include "pcm_transfer.h"
#include "folder.h"
#include "fat_file.h"
#include "enc_out_buff.h"
//#include "audio_encoder.h"

#define ENCODE_PARAM_ENCODER_TYPE   MP2_ENCODER//ADPCM_ENCODER//MP2_ENCODER
#define ENCODE_PARAM_SAMPLE_RATE    48000
#define ENCODE_PARAM_CHANNEL_NUM    1
#define ENCODE_PARAM_BITRATE        384 //kbps

#define ENCODER_FIFO_START_ADDR     PMEM_OFIFO_START_ADDR
#define ENCODER_FIFO_LENGTH         8192

typedef enum _PcmDataSource
{
    ADC    = 0,
    I2SIN  = 1,
    LINEIN = 2
}PcmDataSource;

DECLARE_SEMAPHORE(AEEStart, 1);

static FIL_HANDLE pOutFile = NULL;
static BOOL       IsAudioEncoderOpenFlag = FALSE;

//extern WORD  I2SCurrWritePointGet(VOID);
//extern WORD  AdcMixCurrWritePointGet(VOID);

VOID  Adc2PmemInit(VOID);
VOID  I2sin2PmemInit(VOID);
VOID  Linein2PmemInit(VOID);
VOID  PcmRxInit(PcmDataSource src);
DWORD PcmRxGetSamplesAvailable(VOID);
DWORD PcmRxReceiveSamples(SWORD *PcmData, DWORD NumSamplesRequired);

VOID Adc2PmemInit(VOID)
{
    CodecAdcAnaInit();
    CodecAdcChannelSel(ADC_CH_MIC_L | ADC_CH_MIC_R);
    CodecAdcMicGainConfig(0x05, 0x05);
    WaitMs(400);
    PhubPathClose(ALL_PATH_CLOSE);
    if(!PhubPathSel(ADCIN_TO_PMEM))
    {
        DBG(("phub path sel err!\n")); 
        return;      
    }
    
    AdcPmemWriteDis();
    DacAdcSampleRateSet(ENCODE_PARAM_SAMPLE_RATE, USB_MODE);
    AdcToPmem(USB_MODE, ENCODER_FIFO_START_ADDR, ENCODER_FIFO_LENGTH);
    //AdcPmemWriteEn();
}

VOID I2sin2PmemInit(VOID)
{
    //config pmem
    PmemDataPathSel(PMEM_I2S_PATH);
    I2sWritePmemDis();
    I2sInToPmemConfig((WORD)ENCODER_FIFO_START_ADDR, ENCODER_FIFO_LENGTH);
    I2sWritePmemEn();

	I2sSampleRateSet(ENCODE_PARAM_SAMPLE_RATE, 0);
	GpioI2sIoConfig(0);
    I2sSetMasterMode(1, 1, 2);
}

VOID Linein2PmemInit(VOID)
{
    CodecAdcAnaInit();
    CodecAdcChannelSel(ADC_CH_LINE_L | ADC_CH_LINE_R);
    CodecAdcLineInGainConfig(0x05, 0x05);
    WaitMs(400);
    PhubPathClose(ALL_PATH_CLOSE);
    if(!PhubPathSel(ADCIN_TO_PMEM))
    {
        DBG(("phub path sel err!\n")); 
        return;      
    }
    
    AdcPmemWriteDis();
    DacAdcSampleRateSet(ENCODE_PARAM_SAMPLE_RATE, USB_MODE);
    AdcToPmem(USB_MODE, ENCODER_FIFO_START_ADDR, ENCODER_FIFO_LENGTH);
    //AdcPmemWriteEn();
}

BOOL SdInit(VOID)
{
	GpioSdIoConfig(1);

    SdControllerInit();
    
	//等待SD卡或U盘设备连接
	if(!IsCardLink())
	{
		return FALSE;
	}

	if(SdCardInit())	
	{
		return FALSE;
	}

	if(!FSInit(DEV_ID_SD))
	{
		return FALSE;
	}
    
	return TRUE;
}

#if 1 //Call driver written by Richard

typedef DWORD (*PmemPcmRemainLenGetFunc)(VOID);
typedef DWORD (*PcmDataReadFunc)(BYTE *Buffer, DWORD Len, DWORD PcmDataMode);
static  PmemPcmRemainLenGetFunc PmemPcmRemainLenGet;
static  PcmDataReadFunc         PcmDataRead;

VOID PcmRxInit(PcmDataSource src)
{
    switch(src)
    {
    case ADC:
        Adc2PmemInit();
        PmemPcmRemainLenGet = AdcPmemPcmRemainLenGet;
        PcmDataRead         = AdcPcmDataRead;
        break;
    case I2SIN:
        I2sin2PmemInit();
        PmemPcmRemainLenGet = I2sPmemPcmRemainLenGet;
        PcmDataRead         = I2sPcmDataRead;
        break;
    case LINEIN:
        Linein2PmemInit();
        PmemPcmRemainLenGet = AdcPmemPcmRemainLenGet;
        PcmDataRead         = AdcPcmDataRead;
        break;
    default:
        break;
    }
}

DWORD PcmRxGetSamplesAvailable(VOID)
{
    return PmemPcmRemainLenGet()/4;
}

DWORD PcmRxReceiveSamples(SWORD *PcmData, DWORD NumSamplesRequired)
{
    return PcmDataRead((BYTE*)PcmData, NumSamplesRequired*2*audio_encoder->num_channels, audio_encoder->num_channels*2-1)/(2*audio_encoder->num_channels);
}

#else //Call driver written by Aissen

#define MIN(A, B)           ((A) < (B) ? (A) : (B))
#define	MAX(A, B)           ((A) > (B) ? (A) : (B))

DWORD  gPcmReadPtr;
DWORD *gPcmInFifoAddr;
DWORD  gPcmInFifoCapacity;

WORD   (*GetCurrWritePtr)(VOID);

VOID PcmRxInit(PcmDataSource src)
{
    gPcmReadPtr        = 0;
    gPcmInFifoAddr     = (DWORD*)(ENCODER_FIFO_START_ADDR+PMEM_ADDR);
    gPcmInFifoCapacity = ENCODER_FIFO_LENGTH/4;
    GetCurrWritePtr    = AdcMixCurrWritePointGet;
    
    Linein2PmemInit();
}

DWORD PcmRxGetSamplesAvailable(VOID)
{
    DWORD PcmWritePtr = (PMEM_ADDR+GetCurrWritePtr()-(DWORD)gPcmInFifoAddr)/4;
    
    return (PcmWritePtr >= gPcmReadPtr ? (PcmWritePtr-gPcmReadPtr) : (PcmWritePtr+gPcmInFifoCapacity-gPcmReadPtr));
}

DWORD PcmRxReceiveSamples(SWORD *PcmData, DWORD NumSamplesRequired)
{
    DWORD PcmWritePtr = (PMEM_ADDR+GetCurrWritePtr()-(DWORD)gPcmInFifoAddr)/4;
    DWORD NumSamples;
    DWORD NumSamples1;
    
    DWORD *PcmData_ = (DWORD*)PcmData;

    if(PcmWritePtr >= gPcmReadPtr)
    {
        NumSamples = PcmWritePtr-gPcmReadPtr;
        NumSamples = MIN(NumSamples, NumSamplesRequired);
        
        memcpy(PcmData_, &gPcmInFifoAddr[gPcmReadPtr], NumSamples<<2);
        
        gPcmReadPtr += NumSamples;
    }
    else
    {
        NumSamples = PcmWritePtr+gPcmInFifoCapacity-gPcmReadPtr;
        NumSamples = MIN(NumSamples, NumSamplesRequired);
        NumSamples1 = gPcmInFifoCapacity - gPcmReadPtr;
        
        if(NumSamples1 >= NumSamples)
        {
            memcpy(PcmData_, &gPcmInFifoAddr[gPcmReadPtr], NumSamples<<2);
            gPcmReadPtr += NumSamples;
        }
        else
        {
            memcpy(PcmData, &gPcmInFifoAddr[gPcmReadPtr], NumSamples1<<2);
            gPcmReadPtr = NumSamples-NumSamples1;
            memcpy(&PcmData_[NumSamples1], &gPcmInFifoAddr[0], gPcmReadPtr<<2);            
        }
    }

    if(gPcmReadPtr >= gPcmInFifoCapacity)
    {
        gPcmReadPtr = 0;
    }
    
    return NumSamples;
}

#endif

VOID AudioEncoderEncodeTask(VOID)
{
    DWORD NumChannels = ENCODE_PARAM_CHANNEL_NUM;
    DWORD SampleRate  = ENCODE_PARAM_SAMPLE_RATE;
    DWORD BitRate     = ENCODE_PARAM_BITRATE;
    
    BYTE* OutBuffer;
    DWORD len;

    BYTE  FileName[16];

    EncoderType EncoderType = ENCODE_PARAM_ENCODER_TYPE;

    TIMER EncoderWatchTimer;

    OSDOWN(AEEStart);

    DBG(("Audio encoder encode task entried.\n"));
     
    sprintf(FileName, "%02dK_%1dCH.%s", SampleRate/1000, NumChannels, EncoderType == ADPCM_ENCODER ? "wav" : "mp2");

    if(pOutFile = MvFopen(FileName, "wb"))
    {
        if(RT_SUCCESS == audio_encoder_initialize((BYTE*)VMEM_ADDR, PcmRxGetSamplesAvailable, PcmRxReceiveSamples, EncoderType, NumChannels, SampleRate, BitRate, 0))
        {
            PcmRxInit(LINEIN);
            EncoderOutBufferInit((BYTE*)VMEM_ADDR+audio_encoder->encoder_size, VMEM_SIZE-audio_encoder->encoder_size);

            if(audio_encoder_update_stream_header(&OutBuffer, &len))
            {
                if(OutBuffer && len)
                {
                    EncoderOutBufferPush(OutBuffer, len);
                }
            }
            
            AdcPmemWriteEn();

            IsAudioEncoderOpenFlag = TRUE;

            OSUP(AEEStart);

            TimeOutSet(&EncoderWatchTimer, 60000);

            while(1)
            {
                if(IsTimeOut(&EncoderWatchTimer))
                {
                    break;
                }

                if(PcmRxGetSamplesAvailable()<32)
                {
                    // CPU 出让
                    OSRescheduleTimeout(10);
                }

                if(RT_SUCCESS == audio_encoder_encode(&OutBuffer, &len))
                {
                    if(!EncoderOutBufferPush(OutBuffer, len))
                    {
                        DBG(("*"));
                    }
                    
                    // CPU 出让
                    OSRescheduleTimeout(8);
                    DBG(("."));
                }
            }

            IsAudioEncoderOpenFlag = FALSE;
            
            // Pop the last encoded data.
            EncoderOutBufferPop(pOutFile);
            
            // Update header if existed.
            if(audio_encoder_update_stream_header(&OutBuffer, &len))
            {
                if(OutBuffer && len)
                {                    
                    MvFseek(pOutFile, 0, SEEK_SET);                   
                    MvFwrite(OutBuffer, 1, len, pOutFile);
                }
            }

            audio_encoder_close();
        }
        else
        {
            DBG(("Encoder initialize fail.\n"));
        }

        MvFclose(pOutFile);
    }
    else
    {
        DBG(("Open output file fail.\n"));
    }

    DBG(("Encode done.\n"));

    OSTaskExit(NULL);
}

VOID AudioEncoderWriteTask(VOID)
{
    DBG(("Audio encoder write task entried.\n"));

    OSDOWN(AEEStart);

    while(IsAudioEncoderOpenFlag)
    {
        //WaitMs(1);
        //EncoderOutBufferPop(pOutFile);
        EncoderOutBufferPopByBlock(pOutFile);
    }
    
    OSTaskExit(NULL);
}
