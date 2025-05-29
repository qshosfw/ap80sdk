/**
 **************************************************************************************
 * @file    audio_encoders_test.c
 * @brief   Audio Encoders Testbench
 * 
 * @author  Aissen Li
 * @version V1.0.0
 * 
 * $Created: 2014-04-22 17:31:10$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "type.h"
#include "app_config.h"
#include "string_convert.h"
#include "enc_out_buff.h"
#include "clk.h"
#include "dac.h"
#include "gpio.h"
#include "uart.h"
#include "cache.h"
#include "delay.h"
#include "audio_adc.h"
#include "audio_path.h"
#include "debug.h"
#include "watchdog.h"
#include "pcm_fifo.h"
#include "pcm_transfer.h"
#include "audio_utility.h"
#include "audio_encoder.h"
#include "audio_decoder.h"
#include "fsinfo.h"
#include "fat_file.h"
#include "fs_mount.h"
#include "os.h"
#include "timeout.h"
#include "sd_card.h"

#define  PCM_IFIFO_START_ADDR       (0) //offset address
#define  PCM_IFIFO_LENGTH           (4096)
#define  PCM_OFIFO_START_ADDR       (PCM_IFIFO_START_ADDR + PCM_IFIFO_LENGTH)
#define  PCM_OFIFO_LENGTH           (8192)

DECLARE_SEMAPHORE(AEEStart, 1);

static bool      IsAudioEncoderStartFlag;
static FAT_FILE  gFatFile;
static FAT_FILE* FileHandle = &gFatFile;

extern int32_t start_kernel(void);

//extern bool SdCardInit(void);
//extern void SysTickInit(void);
//extern void CodecDacMuteAll(uint16_t);

//extern const uint8_t* GetLibVersionAudioUtility(void);
//extern const uint8_t* GetLibVersionAudioEncoder(void);
//extern const uint8_t* GetLibVersionAudioDecoder(void);

static const uint16_t SAMPLE_RATE_TABLE[] =
{
    8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
};

static const uint32_t BIT_RATE_TABLE[] =
{
    96, 112, 128, 192, 320, 384
};

bool IsCardLink(void)
{
    return TRUE;
}

bool CheckAllDiskLinkFlag(void)
{
    return TRUE;
}

bool SdInitAndFsMount(void)
{
    //Deinit File System first.
    FSDeInit(DEV_ID_SD);

    GpioSdIoConfig(1);

    if(SdCardInit())
    {
        DBG("[INFO]: SD initialize fail.\n");
        return FALSE;
    }

    if(!FSInit(DEV_ID_SD))
    {
        DBG("[INFO]: FS mount fail.\n");
        return FALSE;
    }

    DBG("[INFO]: SD initialize and FS mount success.\n\n");

    return TRUE;
}

uint32_t gCycleCnt = 0;

void tic(void)
{
    gCycleCnt = *(uint32_t*)0xE0001004;
}

uint32_t toc(void)
{
    uint32_t cnt = *(uint32_t*)0xE0001004;

    if(cnt >= gCycleCnt)
    {
        return cnt - gCycleCnt;
    }
    else
    {
        return cnt + ((uint32_t)0xFFFFFFFF - gCycleCnt);
    }
}

static uint8_t gKey = 0;

#define GetKeyValue()    gKey
#define SetKeyValue(v)   gKey = v

/**
 * @brief Fuart Interrupt Service Routine
 * @param[in]  NONE
 * @param[out] NONE
 */
void FuartInterrupt(void)
{
    //If data received
    if(IsFuartRxIntSrc())
    {
        uint8_t c;

        //Get received byte
        FuartRecvByte(&c);

		//clear interrupt
        FuartClrRxInt();

        SetKeyValue(c);
	}

    //If data sent
	if(IsFuartTxIntSrc())
    {
		//clear interrupt
        FuartClrTxInt();
	}
}

static uint8_t WaitForKeyInput(uint8_t min, uint8_t max)
{
    uint8_t KeyValue;

    while(1)
    {
        __WFI();

        KeyValue = GetKeyValue();

        SetKeyValue(0);
        
        if(KeyValue && KeyValue >= min && KeyValue <= max)
        {
            return KeyValue;
        }
    }
}

uint32_t AdcPcmDataReceive(int16_t* Buffer, uint32_t Samples)
{
    if(audio_encoder)
    {
        return AdcPcmDataRead((uint8_t*)Buffer, Samples, audio_encoder->num_channels == 1 ? 1 : 3);
    }
    else
    {
        return 0;
    }
}

void AudioEncoderWriteTask(void)
{
    DBG("[INFO]: Audio encoder write task entried.\n");

    OSDOWN(AEEStart);

    while(IsAudioEncoderStartFlag)
    {
        //EncoderOutBufferPop(FileHandle);
        EncoderOutBufferPopByBlock(FileHandle);
    }

    DBG("\n[INFO]: Audio encoder write task exited.\n");

    OSUP(AEEStart);

    while(1)
    {
        OSRescheduleTimeout(10);
    }
}

int32_t main(void)
{
    uint32_t  FileCount = 0;

    uint8_t   KeyValue;

    bool IsSdInitAndFsMount;

    ClkPorRcToDpll(0);

    CacheInit();

	ClkModuleEn(ALL_MODULE_CLK_SWITCH);
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);	
    
    SysTickInit();

	// Disable Watchdog
    WaitMs(200);
	WdgDis();

    GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);

	FuartInit(115200, 8, 0, 1);

    WaitMs(100);
    DBG("/==========================================================================\\\n");
    DBG("|                   AUDIO CODEC (ENCODERS) TESTBENCH                       |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");

    DBG("\n");
    DBG("Audio utility library version : %s\n", GetLibVersionAudioUtility());
    DBG("Audio encoder library version : %s\n", GetLibVersionAudioEncoder());
    DBG("Audio decoder library version : %s\n", GetLibVersionAudioDecoder());
    DBG("\n");

    //Initialize OS
    OSStartKernel();

    CodecAdcAnaInit();
    CodecAdcMicGainConfig(0x0, 0x0);
    CodecAdcLineInGainConfig(0x0, 0x0);
    CodecDacInit(TRUE);
    CodecDacMuteSet(TRUE, TRUE);
    CodecDacChannelSel(DAC_CH_DECD_L | DAC_CH_DECD_R);
    AdcVolumeSet(0xFB0, 0xFB0);
    DacVolumeSet(0xED8, 0xED8);
    DacNoUseCapacityToPlay();
    DacConfig(MDAC_DATA_FROM_DEC, USB_MODE);
    PhubPathSel(ADCIN2PMEM_PCMFIFO2DACOUT);
    CodecDacMuteSet(FALSE, FALSE);

    NVIC_EnableIRQ(DECODER_IRQn);

    IsSdInitAndFsMount = SdInitAndFsMount();

	OSStartSchedule();
	OSTaskPrioSet(NULL, 4);    

    while(1)
    {
        if(IsSdInitAndFsMount)
        {
            uint8_t  FileName[11];

            uint32_t EncoderType = MP2_ENCODER;
            uint32_t DecoderType = MP3_DECODER;
            uint32_t NumChannels = 2;
            uint32_t SampleRate  = 44100;
            uint32_t BitRate     = 192;

            uint8_t* OutBuffer;
            uint32_t len;

            DBG("[INFO]: Please select input audio source, [0]: MIC, [1]: LINEIN ...... ");
            EncoderType = WaitForKeyInput('0', '1') - '0';
            if(EncoderType == 0)
            {
                CodecAdcChannelSel(ADC_CH_NONE);
                CodecAdcChannelSel(ADC_CH_MIC_L | ADC_CH_MIC_R);   
            }
            else
            {
                CodecAdcChannelSel(ADC_CH_NONE);
                CodecAdcChannelSel(ADC_CH_LINE_L | ADC_CH_LINE_R);
            }
            DBG("%d\n\n", EncoderType);

            DBG("[INFO]: Please select encoder type, [1]: ADPCM, [2]: MP2 ...... ");
            EncoderType = WaitForKeyInput('1', '2') - '0';
            DecoderType = EncoderType == MP2_ENCODER ? MP3_DECODER : WAV_DECODER;
            DBG("%d\n\n", EncoderType);

            DBG("[INFO]: Please select encode channels, [1]: MONO, [2]: STEREO ...... ");
            NumChannels = WaitForKeyInput('1', '2') - '0';
            DBG("%d\n\n", NumChannels);

            if(EncoderType == MP2_ENCODER)
            {
                DBG("[INFO]: Please select encode sample rate, [3]: 16K, [4]: 22K, [5]: 24K, [6]: 32K, [7]:44K, [8]:48K ...... ");
                SampleRate = SAMPLE_RATE_TABLE[WaitForKeyInput('3', '8') - '0'];
            }
            else
            {
                DBG("[INFO]: Please select encode sample rate, [0]: 8K, [1]: 11K, [2]: 12K, [3]: 16K, [4]: 22K, [5]: 24K, [6]: 32K, [7]:44K, [8]:48K ...... ");
                SampleRate = SAMPLE_RATE_TABLE[WaitForKeyInput('0', '8') - '0'];
            }

            DBG("%d Hz\n\n", SampleRate);

            if(EncoderType == MP2_ENCODER)
            {
                DBG("[INFO]: Please select encode bitrate, [0]: 96K, [1]: 112K, [2]: 128K, [3]: 192K, [4]: 320K, [5]:384K ...... ");
                BitRate = BIT_RATE_TABLE[WaitForKeyInput('0', '5') - '0'];
                DBG("%d Kbps\n\n", BitRate);
            }

            memset(FileName, 0, sizeof(FileName));
            sprintf((char*)FileName, "REC%04d.%s", ++FileCount, EncoderType == MP2_ENCODER ? "mp3" : "wav");

            if(FileOpen(FileHandle, FileName, FA_WRITE|FA_CREATE_ALWAYS))
            {
                if(RT_SUCCESS == audio_encoder_initialize((uint8_t*)VMEM_ADDR, AdcPmemPcmRemainLenGet, AdcPcmDataReceive, EncoderType, NumChannels, SampleRate, BitRate, 0))
                {
                    xTaskHandle AudioEncoderWriteTaskHandle;

                    OSDOWN(AEEStart);
                    OSRescheduleTimeout(10);
                    OSTaskCreate(AudioEncoderWriteTask, "AEW", CFG_TASK_STACK_SIZE, NULL, 3, &AudioEncoderWriteTaskHandle);

                    EncoderOutBufferInit((uint8_t*)VMEM_ADDR + audio_encoder->encoder_size, VMEM_SIZE - audio_encoder->encoder_size);

                    DacAdcSampleRateSet(SampleRate, USB_MODE);
                    AdcPmemWriteDis();
                    AdcToPmem(USB_MODE, PCM_IFIFO_START_ADDR, PCM_IFIFO_LENGTH);

                    if(audio_encoder_update_stream_header(&OutBuffer, &len))
                    {
                        if(OutBuffer && len)
                        {
                            EncoderOutBufferPush(OutBuffer, len);
                        }
                    }

                    DBG("[INFO]: Start recording as file %s, press [q] to quit recording.\n", FileName);

                    AdcPmemWriteEn();

                    IsAudioEncoderStartFlag = TRUE;

                    OSUP(AEEStart);

                    while(1)
                    {
                        if('q' == (KeyValue = GetKeyValue()))
                        {
                            SetKeyValue(0);
                            break;
                        }

                        if(AdcPmemPcmRemainLenGet() < 32)
                        {
                            OSRescheduleTimeout(10);
                        }

                        if(RT_SUCCESS == audio_encoder_encode(&OutBuffer, &len))
                        {
                            if(EncoderOutBufferPush(OutBuffer, len))
                            {
                                OSRescheduleTimeout(5);
                            }
                            else
                            {
                                OSRescheduleTimeout(5);
                                EncoderOutBufferPush(OutBuffer, len);
                            }

                            DBG(".");
                        }
                    }

                    IsAudioEncoderStartFlag = FALSE;

                    OSDOWN(AEEStart);
                    OSTaskExit(AudioEncoderWriteTaskHandle);
                    OSUP(AEEStart);

                    // Pop the last encoded data.
                    EncoderOutBufferPop(FileHandle);

                    // Update header if existed.
                    if(audio_encoder_update_stream_header(&OutBuffer, &len))
                    {
                        if(OutBuffer && len)
                        {                    
                            FileSeek(FileHandle, 0, SEEK_SET);                   
                            FileWrite(OutBuffer, 1, len, FileHandle);
                        }
                    }

                    audio_encoder_close();
                    
                    FileClose(FileHandle);

                    DBG("\n[INFO]: Recording done.\n");
                    DBG("[INFO]: Playing back.\n");

                    // Play back
                    if(FileOpen(FileHandle, FileName, FA_READ))
                    {
                        if(RT_SUCCESS == audio_decoder_initialize((uint8_t*)VMEM_ADDR, (void*)FileHandle, IO_TYPE_FILE, DecoderType))
                        {
                            PcmTxInitWithDefaultPcmFifoSize((PCM_DATA_MODE)audio_decoder->song_info->pcm_data_mode);
                            PcmFifoInitialize(PCM_OFIFO_START_ADDR, PCM_OFIFO_LENGTH, 0, 0);
                            DacAdcSampleRateSet(audio_decoder->song_info->sampling_rate, USB_MODE);

                            while(RT_YES == audio_decoder_can_continue())
                            {
                                if('q' == (KeyValue = GetKeyValue()))
                                {
                                    SetKeyValue(0);
                                    break;
                                }

                                if(RT_NO == audio_decoder_check_pcm_transfer_done())
                                {
                                    continue;
                                }

                                if(RT_SUCCESS == audio_decoder_decode())
                                {
                                    if(RT_YES == is_audio_decoder_with_hardware())
                                    {
                                        audio_decoder_wait_for_hardware_decoder_done();
                                    }

                                    audio_decoder_clear_pcm_transfer_done();
                                    audio_decoder_start_pcm_transfer();
                                }
                            }
                        }
                        
                        FileClose(FileHandle);
                    }
                }
                else
                {
                    DBG("[INFO]: Encoder initialization fail.\n");
                }

                FileClose(FileHandle);
            }
            else
            {
                DBG("[INFO]: Creating record file %s fail.\n", FileName);
                FileCount--;
            }
        }

        DBG("\nPress the follow keys to continue:\n");
        DBG("[r]: RESTART\n");
        DBG("[R]: RESET\n");
        DBG("\n");

        while(1)
        {
            bool BreakFlag = TRUE;

            __WFI();
            
            KeyValue = GetKeyValue();

            SetKeyValue(0);
            
            if(!KeyValue)
            {
                continue;
            }

            switch(KeyValue)
            {
            case 'r':
                DBG("[KEY__INFO]: RESTART\n");
                break;

            case 'R'://reset
                IsSdInitAndFsMount = SdInitAndFsMount();
                DBG("[KEY__INFO]: RESET\n");
                break;

            default:
                BreakFlag = FALSE;
                DBG("[KEY__INFO]: UNKOWN\n");
                break;
            }

            if(BreakFlag)
            {
                break;
            }
        }
    }

    //return 0;
}


void HIDReceive(uint8_t *pBuffer)
{
	return;
}

void HIDSendData(uint8_t *pBuffer,uint32_t RxLength)
{
	return;
}
