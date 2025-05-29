/**
 **************************************************************************************
 * @file    mp3_encoder_test.c
 * @brief   MP3 Encoder Testbench
 * 
 * @author  Aissen Li
 * @version V1.1.0
 * 
 * $Id$
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
#include "audio_adc.h"
#include "audio_path.h"
#include "cache.h"
#include "debug.h"
#include "delay.h"
#include "watchdog.h"
#include "pcm_fifo.h"
#include "pcm_transfer.h"
#include "mp3enc_api.h"
#include "audio_decoder.h"
#include "fsinfo.h"
#include "fat_file.h"
#include "fs_mount.h"
#include "os.h"
#include "timeout.h"
#include "sd_card.h"

#define  PCM_IFIFO_START_ADDR       (0) //offset address
#define  PCM_IFIFO_LENGTH           (8192)
#define  PCM_OFIFO_START_ADDR       (PCM_IFIFO_START_ADDR + PCM_IFIFO_LENGTH)
#define  PCM_OFIFO_LENGTH           (8192)

#define  PCM_FRAME_IBUFFER          (PMEM_ADDR + PCM_OFIFO_START_ADDR + PCM_OFIFO_LENGTH)
#define  PCM_FRAME_IBUFFER_LENGTH   (SAMPLES_PER_FRAME * 4)
#define  PCM_FRAME_OBUFFER          (PCM_FRAME_IBUFFER + PCM_FRAME_IBUFFER_LENGTH)
#define  PCM_FRAME_OBUFFER_LENGTH   (MP3_ENCODER_OUTBUF_CAPACITY)

#define  PART_SAMPLES_PER_FRAME     (SAMPLES_PER_FRAME)

DECLARE_SEMAPHORE(Mp3EncoderStart, 1);

static bool      IsMp3EncoderStartFlag;
static FAT_FILE  gFatFile;
static FAT_FILE* FileHandle = &gFatFile;

//extern bool SdCardInit(void);
//extern void SysTickInit(void);
//extern void CodecDacMuteAll(uint16_t);

extern int32_t start_kernel(void);

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

void Mp3EncoderWriteTask(void)
{
    DBG("[INFO]: MP3 encoder write task entried.\n");

    OSDOWN(Mp3EncoderStart);

    while(IsMp3EncoderStartFlag)
    {
        //EncoderOutBufferPop(FileHandle);
        EncoderOutBufferPopByBlock(FileHandle);
    }

    DBG("\n[INFO]: MP3 encoder write task exited.\n");

    OSUP(Mp3EncoderStart);

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
    DBG("|                   AUDIO CODEC (MP3 ENCODER) TESTBENCH                    |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");

    DBG("\n");
    DBG("Audio utility library version : %s\n", GetLibVersionAudioUtility());
    //DBG("Audio encoder library version : %s\n", GetLibVersionAudioEncoder());
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

            uint32_t EncoderSrc  = 0;
            uint32_t DecoderType = MP3_DECODER;
            uint32_t NumChannels = 2;
            uint32_t SampleRate  = 44100;
            uint32_t BitRate     = 192;

            uint8_t* OutBuffer;
            uint32_t len;

            DBG("[INFO]: Please select input audio source, [0]: MIC, [1]: LINEIN ...... ");
            EncoderSrc = WaitForKeyInput('0', '1') - '0';
            if(EncoderSrc == 0)
            {
                CodecAdcChannelSel(ADC_CH_NONE);
                CodecAdcChannelSel(ADC_CH_MIC_L | ADC_CH_MIC_R);   
            }
            else
            {
                CodecAdcChannelSel(ADC_CH_NONE);
                CodecAdcChannelSel(ADC_CH_LINE_L | ADC_CH_LINE_R);
            }
            DBG("%d\n\n", EncoderSrc);

            DBG("[INFO]: Please select encode channels, [1]: MONO, [2]: STEREO ...... ");
            NumChannels = WaitForKeyInput('1', '2') - '0';
            DBG("%d\n\n", NumChannels);

            DBG("[INFO]: Please select encode sample rate, [6]: 32K, [7]:44K, [8]:48K ...... ");
            SampleRate = SAMPLE_RATE_TABLE[WaitForKeyInput('6', '8') - '0'];
            DBG("%d Hz\n\n", SampleRate);

            DBG("[INFO]: Please select encode bitrate, [0]: 96K, [1]: 112K, [2]: 128K, [3]: 192K, [4]: 320K ...... ");
            BitRate = BIT_RATE_TABLE[WaitForKeyInput('0', '4') - '0'];
            DBG("%d Kbps\n\n", BitRate);

            memset(FileName, 0, sizeof(FileName));
            sprintf((char*)FileName, "REC%04d.mp3", ++FileCount);

            if(FileOpen(FileHandle, FileName, FA_WRITE | FA_CREATE_ALWAYS))
            {
                uint8_t* Mp3EncoderFrameBuffer = (uint8_t*)PCM_FRAME_OBUFFER;

                MP3EncoderContext* Mp3Encoder  = (MP3EncoderContext*)(VMEM_ADDR);

                if(RT_SUCCESS == mp3_encoder_initialize(Mp3Encoder, NumChannels, SampleRate, BitRate, Mp3EncoderFrameBuffer))
                {
                    uint32_t SampleCount = 0;
                    uint32_t SampleStep  = NumChannels == 1 ? 2 : 4;

                    xTaskHandle Mp3EncoderWriteTaskHandle;

                    OSDOWN(Mp3EncoderStart);
                    OSRescheduleTimeout(10);
                    OSTaskCreate(Mp3EncoderWriteTask, "MEW", CFG_TASK_STACK_SIZE, NULL, 3, &Mp3EncoderWriteTaskHandle);

                    EncoderOutBufferInit(((uint8_t*)Mp3Encoder) + sizeof(MP3EncoderContext), VMEM_SIZE - sizeof(MP3EncoderContext));

                    DacAdcSampleRateSet(SampleRate, USB_MODE);
                    AdcPmemWriteDis();
                    AdcToPmem(USB_MODE, PCM_IFIFO_START_ADDR, PCM_IFIFO_LENGTH);

                    DBG("[INFO]: Start recording as file %s, press [q] to quit recording.\n", FileName);

                    AdcPmemWriteEn();

                    IsMp3EncoderStartFlag = TRUE;

                    OSUP(Mp3EncoderStart);

                    while(1)
                    {
                        if('q' == (KeyValue = GetKeyValue()))
                        {
                            SetKeyValue(0);
                            break;
                        }

                        if(AdcPmemPcmRemainLenGet() < PART_SAMPLES_PER_FRAME)
                        {
                            OSRescheduleTimeout(2);
                        }
                        else
                        {
                            AdcPcmDataRead((uint8_t*)PCM_FRAME_IBUFFER + SampleCount * SampleStep, PART_SAMPLES_PER_FRAME, SampleStep - 1);
                            SampleCount += PART_SAMPLES_PER_FRAME;
                        }

                        if(SampleCount >= SAMPLES_PER_FRAME)
                        {
                            SampleCount = 0;

                            if(RT_SUCCESS == mp3_encoder_encode(Mp3Encoder, (int16_t*)PCM_FRAME_IBUFFER, &OutBuffer, &len))
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
                    }

                    IsMp3EncoderStartFlag = FALSE;

                    OSDOWN(Mp3EncoderStart);
                    OSTaskExit(Mp3EncoderWriteTaskHandle);
                    OSUP(Mp3EncoderStart);

                    // Pop the last encoded data.
                    EncoderOutBufferPop(FileHandle);

                    mp3_encoder_close();

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
