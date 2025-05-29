/**
 **************************************************************************************
 * @file    pitch_shifter_test.c
 * @brief   Pitch Shifter Effect Testbench
 * 
 * @author  Aissen Li
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "app_config.h"
#include "rst.h"
#include "clk.h"
#include "dac.h"
#include "adc.h"
#include "dac.h"
#include "gpio.h"
#include "uart.h"
//#include "debug.h"
#include "delay.h"
#include "cache.h"
#include "fsinfo.h"
#include "sd_card.h"
#include "timeout.h"
#include "fat_file.h"
#include "fs_mount.h"
#include "watchdog.h"
#include "pcm_fifo.h"
#include "pcm_transfer.h"
#include "audio_path.h"
#include "audio_utility.h"
#include "audio_decoder.h"
#include "string_convert.h"
#include "pitch_shifter.h"

#define  SHOW_SONG_INFO_FLAG    1

#define  MIN_DAC_VOLUME         (0)
#define  MAX_DAC_VOLUME         (32)

static const uint16_t DAC_VOL_TABLE[] =
{
	0,
	26,		31,		36,		42,		49,		58,		67,		78,
	91,		107,	125,	147,	173,	204,	240,	282,
	332,	391,	460,	541,	636,	748,	880,	1035,
	1218,	1433,	1686,	1984,	2334,	2746,	3230,	3800//最高音量约930mVrms
};

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
        DBG(("SD initialize fail.\n"));
        return FALSE;
    }

    if(!FSInit(DEV_ID_SD))
    {
        DBG(("FS mount fail.\n"));
        return FALSE;
    }

    DBG(("SD initialize and FS mount success.\n\n"));

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

void help(void)
{
    DBG("[SHORT_KEY]: AS FOLLOWING\n");
    DBG("    [H] ............... SHOW HELP MESSAGE\n");
    DBG("    [Space] ........... PLAY / PAUSE\n");
    DBG("    [h] ............... PLAY PREVIOUS SONG\n");
    DBG("    [j] ............... VOLUME-\n");
    DBG("    [k] ............... VOLUME+\n");
    DBG("    [l] ............... PLAY NEXT SONG\n");
    DBG("    [s] ............... SEEK\n");
    DBG("    [e] ............... SHOW ERROR MESSAGE\n");
    DBG("    [p] ............... Pitch shifter semitone-\n");
    DBG("    [P] ............... Pitch shifter semitone+\n");
    DBG("    [q] ............... QUIT\n");
    DBG("\n");
}

PSContext gPitchShifter;

int16_t PcmBuffer[2048] __attribute__((at(VMEM_ADDR + 28 * 1024)));

int32_t GetPitchShiftFrameSize(int32_t SampleRate)
{
    switch(SampleRate)
    {
    case 48000:
    case 44100:
    case 32000:
        return 576;
    case 24000:
    case 22050:
    case 16000:
        return 288;
    case 12000:
    case 11025:
    case  8000:
        return 144;
    default:
        return 576;
    }
}

int32_t main(void)
{
    uint32_t  FileCount;
    uint32_t  FileIndex;

    FAT_FILE  FileHandle;

    uint16_t  DacVolume = MAX_DAC_VOLUME / 2;
    
    uint8_t   KeyValue;
    uint8_t   LongFileName[128];

    int32_t   i;

    uint32_t  FrameCount;

    uint32_t  SeekTime;

    uint8_t   ShowErrorCode = FALSE;

    uint32_t  SampleRate;
    
    bool      IsSdInitAndFsMount;

    int32_t   PitchShiftStep = 0;

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
    DBG("|                   MP3 Decoder & Pitch Shifter Demonstration              |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");

    DBG("\n");
    DBG("Audio utility library version : %s\n", GetLibVersionAudioUtility());
    DBG("Audio decoder library version : %s\n", GetLibVersionAudioDecoder());
    DBG("\n");

    help();

    CodecDacInit(TRUE);
    CodecDacMuteSet(TRUE, TRUE);
    CodecDacChannelSel(DAC_CH_DECD_L | DAC_CH_DECD_R);
    DacAdcSampleRateSet(8000, NORMAL_MODE);
    DacVolumeSet(DAC_VOL_TABLE[DacVolume], DAC_VOL_TABLE[DacVolume]);
    DacNoUseCapacityToPlay();
    DacConfig(MDAC_DATA_FROM_DEC, NORMAL_MODE);
    PhubPathSel(PCMFIFO_MIX_DACOUT);
    CodecDacMuteSet(FALSE, FALSE);

    FileIndex = 1;

    IsSdInitAndFsMount = SdInitAndFsMount();

    while(1)
    {
        if(IsSdInitAndFsMount)
        {
            bool QuitFlag = FALSE;
            
            FileCount = FsGetCurContext()->gFsInfo.FileSum;
            
            if(FileIndex > FileCount)
            {
                FileIndex = 1;
            }

            while(FileOpenByNum(&FileHandle, NULL, FileIndex++))
            {
                int32_t DecoderType = UNKOWN_DECODER;

                if(QuitFlag)
                {
                    QuitFlag  = FALSE;
                    FileIndex = 1;
                    break;
                }

                if((FileHandle.ShortName[8] == 'M' && FileHandle.ShortName[9] == 'P' && FileHandle.ShortName[10] == '2') ||
                   (FileHandle.ShortName[8] == 'M' && FileHandle.ShortName[9] == 'P' && FileHandle.ShortName[10] == '3'))
                {
                    DecoderType = MP3_DECODER;
                }

                if(DecoderType != UNKOWN_DECODER)
                {
                    memset((void*)LongFileName, 0, 128);
                    if(FileGetLongName(&FileHandle, LongFileName, 128))
                    {
                        StrUnicode2Gbk((uint16_t*)LongFileName, 128/2);                
                    }
                    else
                    {
                        memcpy(LongFileName, FileHandle.ShortName, 8);
                        for(i = 7; i >= 0; i--)
                        {
                            if(LongFileName[i] == 0x20)
                            {
                                LongFileName[i] = 0x00;
                            }
                            else
                            {
                                break;
                            }
                        }
                        i++;
                        LongFileName[i++] = '.';
                        LongFileName[i++] = FileHandle.ShortName[8];
                        LongFileName[i++] = FileHandle.ShortName[9];
                        LongFileName[i++] = FileHandle.ShortName[10];                    
                    }
                    
                    DBG("\n\n[SONG_NAME]: %s ", LongFileName);
                    for(i = strlen((const char*)LongFileName); i < 64; i++)
                    {
                        DBG(("."));
                    }
                    DBG((" "));
                }
                else
                {
                    continue;
                }

                NVIC_DisableIRQ(DECODER_IRQn);
                RstDecModule();

                if(RT_SUCCESS == audio_decoder_initialize((uint8_t*)VMEM_ADDR, (void*)&FileHandle, IO_TYPE_FILE, DecoderType))
                //if(RT_SUCCESS == audio_decoder_initialize((uint8_t*)AAC_DECODER_RAM, (void*)FileHandle, IO_TYPE_FILE, DecoderType))
                {
                    NVIC_EnableIRQ(DECODER_IRQn);

                    FrameCount   = 0;
                    SeekTime     = 0;

                    SampleRate = audio_decoder->song_info->sampling_rate;

                    DBG("[SUCCESS]\n");
                    DBG("[SONG_INFO]: Current file position: %d bytes\n", FileTell(&FileHandle));
                    #if SHOW_SONG_INFO_FLAG               
                    DBG("[SONG_INFO]: ChannelCnt : %6d\n",        audio_decoder->song_info->num_channels);
                    DBG("[SONG_INFO]: SampleRate : %6d Hz\n",     audio_decoder->song_info->sampling_rate);
                    DBG("[SONG_INFO]: BitRate    : %6d Kbps\n",   audio_decoder->song_info->bitrate/1000);
                    DBG("[SONG_INFO]: DecoderSize: %6d Bytes \n", audio_decoder->decoder_size);
                    DBG("[SONG_INFO]: Duration   : %6d S\n",      audio_decoder->song_info->duration/1000);
                    DBG("[SONG_INFO]: IsVBR      : %s\n",         audio_decoder->song_info->vbr_flag ? "   YES" : "    NO");

                    if(audio_decoder->song_info->char_set == CHAR_SET_UTF_8 || audio_decoder->song_info->char_set == CHAR_SET_UTF_16)
                    {
                        StrUnicode2Gbk((uint16_t*)audio_decoder->song_info->title,     MAX_TAG_LEN/2);
                        StrUnicode2Gbk((uint16_t*)audio_decoder->song_info->artist,    MAX_TAG_LEN/2);
                        StrUnicode2Gbk((uint16_t*)audio_decoder->song_info->album,     MAX_TAG_LEN/2);
                        StrUnicode2Gbk((uint16_t*)audio_decoder->song_info->comment,   MAX_TAG_LEN/2);
                        StrUnicode2Gbk((uint16_t*)audio_decoder->song_info->genre_str, MAX_TAG_LEN/2);
                    }
                    DBG("[SONG_INFO]: CHARSET    : %6d\n", audio_decoder->song_info->char_set);
                    DBG("[SONG_INFO]: TITLE      : %s\n", audio_decoder->song_info->title);
                    DBG("[SONG_INFO]: ARTIST     : %s\n", audio_decoder->song_info->artist);
                    DBG("[SONG_INFO]: ALBUM      : %s\n", audio_decoder->song_info->album);
                    DBG("[SONG_INFO]: COMMT      : %s\n", audio_decoder->song_info->comment);
                    DBG("[SONG_INFO]: GENRE      : %s\n", audio_decoder->song_info->genre_str);
                    DBG("[SONG_INFO]: YEAR       : %s\n", audio_decoder->song_info->year);
                    DBG("[SONG_INFO]: TRACK      : %6d\n", audio_decoder->song_info->track);
                    #endif

                    PcmTxInitWithDefaultPcmFifoSize(audio_decoder->song_info->num_channels == 2 ? DATA_MODE_STEREO_LR : DATA_MODE_MONO_S);
                    PcmFifoInitialize(20*1024, 8*1024, 0, 0);
                    DacAdcSampleRateSet(audio_decoder->song_info->sampling_rate, NORMAL_MODE);
                    DacVolumeSet(DAC_VOL_TABLE[DacVolume], DAC_VOL_TABLE[DacVolume]);
                    CodecDacMuteSet(FALSE, FALSE);

                    init_pitch_shifter(&gPitchShifter, audio_decoder->song_info->num_channels, SampleRate, PitchShiftStep * 10, GetPitchShiftFrameSize(SampleRate));

                    while(RT_YES == audio_decoder_can_continue())
                    {
                        if(0 != (KeyValue = GetKeyValue()))
                        {
                            bool BreakFlag = TRUE;

                            switch(KeyValue)
                            {
                            case 'H' :
                                BreakFlag = FALSE;
                                DBG("[KEY__INFO]: SHOW HELP MESSAGE\n");
                                help();
                                break;

                            case 0x20://space: play/pause
                                BreakFlag = FALSE;
                                PcmFifoSetStatus(!PcmFifoGetStatus());
                                DBG("[KEY__INFO]: %s\n", PcmFifoGetStatus() ? "PAUSE" : "PLAY");
                                break;

                            case 0x93://up
                            case 'k' :
                                DacVolume = DacVolume < MAX_DAC_VOLUME ? DacVolume + 1 : MAX_DAC_VOLUME;
                                DacVolumeSet(DAC_VOL_TABLE[DacVolume], DAC_VOL_TABLE[DacVolume]);
                                BreakFlag = FALSE;
                                DBG("[KEY__INFO]: VOLUME+ -> %d\n", DacVolume);
                                break;
                                
                            case 0x95://down
                            case 'j' :
                                DacVolume = DacVolume > MIN_DAC_VOLUME ? DacVolume - 1 : MIN_DAC_VOLUME;
                                DacVolumeSet(DAC_VOL_TABLE[DacVolume], DAC_VOL_TABLE[DacVolume]);
                                BreakFlag = FALSE;
                                DBG("[KEY__INFO]: VOLUME- -> %d\n", DacVolume);
                                break;

                            case 0x92://left
                            case 'h' :
                                FileIndex = FileIndex > 2 ? FileIndex - 2 : 1;
                                DBG("[KEY__INFO]: PREVIOUS\n");
                                break;

                            case 0x94://right
                            case 'l' :
                                DBG("[KEY__INFO]: NEXT\n");
                                break;
                            
                            case 's' :
                                audio_decoder_seek(SeekTime);
                                DBG("[KEY__INFO]: SEEK -> %d s\n", SeekTime / 1000 / 2);
                                SeekTime += 10000;
                                BreakFlag = FALSE;
                                break;

                            case 'e' :
                                ShowErrorCode = !ShowErrorCode;
                                BreakFlag = FALSE;
                                DBG("[KEY__INFO]: SHOW ERROR CODE %s\n", ShowErrorCode ? "ON" : "OFF");
                                break;

                            case 'q' :
                                DBG("[KEY__INFO]: QUIT\n");
                                QuitFlag = TRUE;
                                break;

                            case 'p' :
                                BreakFlag = FALSE;
                                if(--PitchShiftStep < -12)
                                {
                                    PitchShiftStep = -12;
                                }
                                init_pitch_shifter(&gPitchShifter, audio_decoder->song_info->num_channels, SampleRate, PitchShiftStep * 10, GetPitchShiftFrameSize(SampleRate));
                                DBG("[KEY__INFO]: Pitch shift -> %d\n", PitchShiftStep * 100);
                                break;

                            case 'P' :
                                BreakFlag = FALSE;
                                if(++PitchShiftStep > 12)
                                {
                                    PitchShiftStep = 12;
                                }
                                init_pitch_shifter(&gPitchShifter, audio_decoder->song_info->num_channels, SampleRate, PitchShiftStep * 10, GetPitchShiftFrameSize(SampleRate));
                                DBG("[KEY__INFO]: Pitch shift -> %d\n", PitchShiftStep * 100);
                                break;

                            default  :
                                BreakFlag = FALSE;
                                DBG("[KEY__INFO]: UNKNOW\n");
                                break;
                            }

                            SetKeyValue(0);

                            if(BreakFlag)
                            {
                                break;
                            }
                        }

                        if(PcmFifoGetStatus())
                        {
                            continue;
                        }

                        if(RT_SUCCESS == audio_decoder_decode())
                        {
                            int32_t  b;
                            int16_t* pcm = PcmBuffer;

                            FrameCount++;

                            //DO pitch shifter here.
                            for(b = 0; b < audio_decoder->song_info->pcm_data_length * 2 / gPitchShifter.w; b++)
                            {
                                apply_pitch_shifter(&gPitchShifter, pcm, pcm);
                                pcm += gPitchShifter.w / 2 * audio_decoder->song_info->num_channels;
                            }

                            if(RT_YES == is_audio_decoder_with_hardware())
                            {
                                audio_decoder_wait_for_hardware_decoder_done();
                            }
                            
                            if(SampleRate != audio_decoder->song_info->sampling_rate)
                            {
                                DBG("[SONG_INFO]: Sample rate changed : %d Hz --> %d Hz\n", SampleRate, audio_decoder->song_info->sampling_rate);
                                SampleRate = audio_decoder->song_info->sampling_rate;
                                PcmFifoClear();
                                DacAdcSampleRateSet(SampleRate, NORMAL_MODE);

                                init_pitch_shifter(&gPitchShifter, audio_decoder->song_info->num_channels, SampleRate, PitchShiftStep * 10, GetPitchShiftFrameSize(SampleRate));
                            }

                            if(PcmFifoIsEmpty() && (FrameCount != 1))
                            {
                                DBG("E\n");
                            }

                            audio_decoder_clear_pcm_transfer_done();
                            PcmTxTransferData(PcmBuffer, PcmBuffer, audio_decoder->song_info->pcm_data_length);
                            audio_decoder_convert_pcm_data_layout();
                            while(!audio_decoder_check_pcm_transfer_done());

                            memcpy(PcmBuffer, audio_decoder->song_info->pcm0_addr, audio_decoder->song_info->pcm_data_length * audio_decoder->song_info->num_channels * 2);
                        }
                        else
                        {
                            if(ShowErrorCode)
                            {
                                DBG("[ERROR]: %d\n", audio_decoder->error_code);
                            }
                        }
                    }
                }
                else
                {
                    DBG("[FAILURE(%d)]\n", audio_decoder_get_error_code());
                    DBG("[SONG_INFO]: Current file position: %d bytes\n", FileTell(&FileHandle));
                }

                FileClose(&FileHandle);
                
                if(!PcmFifoGetStatus())
                {
                    DacVolumeSet(0, 0);
                    while(!PcmFifoIsEmpty());
                }

                CodecDacMuteSet(TRUE, TRUE);
            }
        }

        DBG("\n[END]\n");
        DBG("All songs had been played, press [p] to re-playing.\n");

        DBG("\nPress the follow keys to continue:\n");
        DBG("[h]: PREVIOUS\n");
        DBG("[l]: NEXT\n");
        DBG("[p]: REPLAY\n");
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
            case 0x92://left
            case 'h' :
                FileIndex = FileIndex - 2;
                DBG("[KEY__INFO]: PREVIOUS\n");
                break;

            case 0x94://right
            case 'l' :
                FileIndex = 1;
                DBG("[KEY__INFO]: NEXT\n");
                break;

            case 'p' :
                FileIndex = 1;
                DBG("[KEY__INFO]: REPLAY\n");
                break;

            case 'R'://reset
                FileIndex = 1;
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
