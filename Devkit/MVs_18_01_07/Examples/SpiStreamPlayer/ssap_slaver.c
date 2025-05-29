/**
 **************************************************************************************
 * @file    ssap_slaver.c
 * @brief   SPI Stream Audio Player (Slaver)
 *
 * @author  Aissen Li
 * @version V1.1.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "type.h"
#include "app_config.h"
#include "crc.h"
#include "ssap_protocol.h"
#include "rst.h"
#include "clk.h"
#include "i2s.h"
#include "dac.h"
#include "uart.h"
#include "gpio.h"
#include "spis.h"
#include "cache.h"
#include "delay.h"
#include "pcm_fifo.h"
#include "watchdog.h"
#include "pcm_transfer.h"
#include "fs_config.h"
#include "audio_path.h"
#include "audio_decoder.h"
#include "chip_info.h"

#define  SPIS_STREAM_BUFFER_CAPACITY            (20*1024)

#define  SPIS_STREAM_CONTROL_GPIO_BANK          ('B')
#define  SPIS_STREAM_CONTROL_GPIO_PIN           (5)
#define  SPIS_STREAM_CONTROL_GPIO_BANK_PIN      ((uint32_t)1 << SPIS_STREAM_CONTROL_GPIO_PIN)
#define  SPIS_STREAM_CONTROL_GPIO_BANK_PIN_IE   ((SPIS_STREAM_CONTROL_GPIO_BANK - 'A') * 10 + GPIO_A_IE)
#define  SPIS_STREAM_CONTROL_GPIO_BANK_PIN_OE   ((SPIS_STREAM_CONTROL_GPIO_BANK - 'A') * 10 + GPIO_A_OE)
#define  SPIS_STREAM_CONTROL_GPIO_BANK_PIN_OUT  ((SPIS_STREAM_CONTROL_GPIO_BANK - 'A') * 10 + GPIO_A_OUT)

#define  SPIS_STREAM_CONTROL_INIT()             \
         GpioSetRegOneBit(SPIS_STREAM_CONTROL_GPIO_BANK_PIN_OUT, SPIS_STREAM_CONTROL_GPIO_BANK_PIN);    \
         GpioSetRegOneBit(SPIS_STREAM_CONTROL_GPIO_BANK_PIN_OE,  SPIS_STREAM_CONTROL_GPIO_BANK_PIN);    \
         GpioClrRegOneBit(SPIS_STREAM_CONTROL_GPIO_BANK_PIN_IE,  SPIS_STREAM_CONTROL_GPIO_BANK_PIN);        
#define  SPIS_STREAM_CONTROL_OPEN()             \
         GpioClrRegOneBit(SPIS_STREAM_CONTROL_GPIO_BANK_PIN_OUT, SPIS_STREAM_CONTROL_GPIO_BANK_PIN);
#define  SPIS_STREAM_CONTROL_CLOSE()            \
         GpioSetRegOneBit(SPIS_STREAM_CONTROL_GPIO_BANK_PIN_OUT, SPIS_STREAM_CONTROL_GPIO_BANK_PIN);

static uint8_t SPIS_STREAM_BUFFER[SPIS_STREAM_BUFFER_CAPACITY];
static uint8_t SPIS_RECEIVE_BUFFER[SSAP_SEND_DATA_LENGTH];
static uint8_t gLongFileName[FAT_NAME_MAX];

static bool xr_start_flag;
static bool tx_start_flag;

static uint32_t gSampleRate;
static uint16_t gDacVolumeIdx;

extern void SysTickInit(void);
//extern void CodecDacMuteAll(uint16_t);
//extern void PrintSysInfo(void);

static bool AudioDecoderInit(void* StreamHandle, int32_t DecoderType);
static void SpisStreamAudioPlayerProcess(void);

#define  MIN_DAC_VOLUME_IDX     (0)
#define  MAX_DAC_VOLUME_IDX     (32)
#define  DAC_VOLUME_TABLE_SIZE  (MAX_DAC_VOLUME_IDX - MIN_DAC_VOLUME_IDX + 1)
static const uint16_t DAC_VOLUME_TABLE[DAC_VOLUME_TABLE_SIZE] =
{
	0,
	26,		31,		36,		42,		49,		58,		67,		78,
	91,		107,	125,	147,	173,	204,	240,	282,
	332,	391,	460,	541,	636,	748,	880,	1035,
	1218,	1433,	1686,	1984,	2334,	2746,	3230,	3800//最高音量约930mVrms
};

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
        //Get received byte
        //FuartRecvByte(&c);

		//clear interrupt
        FuartClrRxInt();
	}

    //If data sent
	if(IsFuartTxIntSrc())
    {
		//clear interrupt
        FuartClrTxInt();
	}
}

bool CheckAllDiskLinkFlag(void)
{
    return TRUE;
}

int32_t main(void)
{
    ClkPorRcToDpll(0);
    CacheInit();
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);

    SysTickInit();

	//Disable Watchdog
    WaitMs(200);
	WdgDis();

    GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);

	FuartInit(115200, 8, 0, 1);

    WaitMs(100);
    DBG("/==========================================================================\\\n");
    DBG("|             SPI STREAM AUDIO PLAYER APPLICATION (SLAVER)                 |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");

    //PrintSysInfo();

    gDacVolumeIdx = MAX_DAC_VOLUME_IDX;//%50 volume

    //I2S initialization
    GpioI2sIoConfig(2);
    GpioMclkIoConfig(1);
    I2sClassdMasterModeSet(I2S_DATA_FROM_DEC, I2S_FORMAT_I2S);

    //DAC initialization
    CodecDacInit(TRUE);
    CodecDacMuteSet(TRUE, TRUE);
    CodecDacChannelSel(DAC_CH_DECD_L | DAC_CH_DECD_R);
    DacVolumeSet(DAC_VOLUME_TABLE[gDacVolumeIdx], DAC_VOLUME_TABLE[gDacVolumeIdx]);
    DacNoUseCapacityToPlay();
    DacConfig(MDAC_DATA_FROM_DEC, NORMAL_MODE);
    PhubPathSel(PCMFIFO_TO_IISOUT_DACOUT);//PCMFIFO_MIX_DACOUT
    CodecDacMuteSet(FALSE, FALSE);

    //SPIS initialization
    GpioSpisIoConfig(0);
    SpiSlaveInit(0, 0x4000, 0x3C00, 0x200);

    SPIS_STREAM_CONTROL_INIT();

    SpisStreamAudioPlayerProcess();

	return 0;
}

static bool AudioDecoderInit(void* StreamHandle, int32_t DecoderType)
{
    NVIC_DisableIRQ(DECODER_IRQn); 

    RstDecModule();

    if(audio_decoder_initialize((uint8_t*)VMEM_ADDR, StreamHandle, IO_TYPE_MEMORY, DecoderType) != RT_SUCCESS)
    {
        return FALSE;
    }

    PcmTxInitWithDefaultPcmFifoSize((PCM_DATA_MODE)audio_decoder->song_info->pcm_data_mode);

    gSampleRate = audio_decoder->song_info->sampling_rate;

    DBG("[INFO]: Audio decoder initialize sample rate as %d Hz\n", gSampleRate);
    DacAdcSampleRateSet(gSampleRate, NORMAL_MODE);
    I2sClassdSTASampleRateSet(gSampleRate, NORMAL_MODE, TRUE);

    xr_start_flag = FALSE;
    tx_start_flag = FALSE;

    NVIC_EnableIRQ(DECODER_IRQn);

    return TRUE;
}

void audio_decoder_interrupt_callback(int32_t intterupt_type)
{
    if(intterupt_type == 0)//XR_DONE
    {
        xr_start_flag = FALSE;
        audio_decoder_start_pcm_transfer();
        tx_start_flag = TRUE;
    }

    if(intterupt_type == 1)//TX_DONE
    {
        tx_start_flag = FALSE;
    }
}

static void AudioDecoderDecodeOneFrame(bool flag)
{
    if(flag)
    {
        if(xr_start_flag)
        {
            if(is_audio_decoder_with_hardware())
            {
                return;
            }
            else
            {
                xr_start_flag = FALSE;
                audio_decoder_start_pcm_transfer();
                tx_start_flag = TRUE;
            }
        }

        if(tx_start_flag)
        {
            return;
        }

        if(RT_SUCCESS == audio_decoder_decode())
        {
            xr_start_flag = TRUE;
            
            if(gSampleRate != audio_decoder->song_info->sampling_rate)
            {
                DBG("[INFO]: Change sample rate from %d to %d Hz\n", gSampleRate, audio_decoder->song_info->sampling_rate);
                gSampleRate = audio_decoder->song_info->sampling_rate;

                PcmTxSetPcmDataMode((PCM_DATA_MODE)audio_decoder->song_info->pcm_data_mode);
                DacAdcSampleRateSet(gSampleRate, NORMAL_MODE);
                I2sClassdSTASampleRateSet(gSampleRate, NORMAL_MODE, TRUE);
            }
        }
        else
        {
            //DBG("[INFO]: ERROR%d\n", audio_decoder->error_code));
        }
    }
}

static bool FindSpiStreamDataSyncWord(bool PlayStatus)
{
	uint32_t SyncWord;
    uint32_t Cnt = 0;
    uint8_t* p = (uint8_t*)&SyncWord;

	while(1)
	{
		SyncWord = 0;

        if(SpiSlaveReceive(p, 1, 1))
        {
            if(p[0] == SSAP_SYNC_BYTE)
            {
                #if (SSAP_SYNC_WORD_LENGTH - 1)
                SpiSlaveReceive((uint8_t*)&p[1], (SSAP_SYNC_WORD_LENGTH - 1), 1);

                if(SyncWord == SSAP_SYNC_WORD_LE)
                #endif

                {
                    return TRUE;
                }
            }
        }

        if(Cnt++ > 100)
		{
            return FALSE;
		}
	}
}

static void SpisStreamAudioPlayerProcess(void)
{
    bool PlayStopFlag;   //0: Stop,   1: Play
    bool IsCmd;
 
    uint8_t* p;
    uint8_t  SSAPCmdData[sizeof(SSAPCmdContext)];

    uint32_t DataLen;
    uint32_t RecvLen;

    uint16_t LocalCrc;
    uint16_t RecvCrc;
 
    int32_t  DecoderType = MP3_DECODER;

    MemHandle StreamHandle;

    SSAPCmdContext*         CommandContext  = (SSAPCmdContext*)SSAPCmdData;
    SSAPCmdResponseContext* ResponseContext = (SSAPCmdResponseContext*)SSAPCmdData;

    StreamHandle.addr         = SPIS_STREAM_BUFFER;
    StreamHandle.mem_capacity = sizeof(SPIS_STREAM_BUFFER);
    StreamHandle.mem_len      = 0;
    StreamHandle.p            = 0;

    p = (uint8_t*)&CommandContext->SyncWord;

    PlayStopFlag   = FALSE;

    while(1)
	{
		IsCmd = FALSE;
        CommandContext->SyncWord = 0;

        SpisClrRxBuff();

        SPIS_STREAM_CONTROL_OPEN();
        SPIS_STREAM_CONTROL_CLOSE();

        if(SpiSlaveReceive(p, 1, 1))
        {
            if(p[0] == SSAP_SYNC_BYTE)
            {
                #if (SSAP_SYNC_WORD_LENGTH - 1)
                SpiSlaveReceive((uint8_t*)&p[1], (SSAP_SYNC_WORD_LENGTH - 1), 1);

                if(CommandContext->SyncWord == SSAP_SYNC_WORD_LE)
                #endif

                {
                    if(SpiSlaveReceive((uint8_t*)&CommandContext->Command, (sizeof(SSAPCmdContext) - SSAP_SYNC_WORD_LENGTH), (sizeof(SSAPCmdContext) - SSAP_SYNC_WORD_LENGTH)) == (sizeof(SSAPCmdContext) - SSAP_SYNC_WORD_LENGTH))
                    {
                        if(CommandContext->CrcValue == GetCRC16NBS((uint8_t*)CommandContext, sizeof(SSAPCmdContext) - SSAP_CRC_VALUE_LENGTH))
                        {
                            IsCmd = TRUE;
                        }
                    }
                }
            }
        }

        if(IsCmd)
        {
            switch(CommandContext->Command)
            {
            case SSAP_CMD_START:
                DBG("[RCMD]: SSAP_CMD_START\n");

                DataLen = CommandContext->Content;           

                if(FindSpiStreamDataSyncWord(PlayStopFlag))
                {
                    uint32_t RemainLen = DataLen;
                    uint8_t* RecvBuff  = SPIS_RECEIVE_BUFFER;

 					while(RemainLen > 0)
					{
						RecvLen = SpiSlaveReceive(RecvBuff, DataLen, 10);

						RemainLen -= RecvLen;
						RecvBuff  += RecvLen;

						if(RemainLen == 0)
						{
							SpiSlaveReceive((uint8_t*)&RecvCrc, SSAP_CRC_VALUE_LENGTH, 2);

							LocalCrc = GetCRC16NBS(SPIS_RECEIVE_BUFFER, DataLen);

                            ResponseContext->Response = RecvCrc == LocalCrc ? SSAP_CMD_RESP_OKSEND : SSAP_CMD_RESP_NEXTSEND;
                            ResponseContext->CrcValue = GetCRC16NBS((uint8_t*)ResponseContext, sizeof(SSAPCmdResponseContext) - SSAP_CRC_VALUE_LENGTH);

                            SpiSlaveSend((uint8_t*)ResponseContext, sizeof(SSAPCmdResponseContext), sizeof(SSAPCmdResponseContext));
 
							if(RecvCrc == LocalCrc)
                            {
                                DecoderType = *(int32_t*)SPIS_RECEIVE_BUFFER;
                                memset(gLongFileName, 0, sizeof(gLongFileName));
                                memcpy(gLongFileName, SPIS_RECEIVE_BUFFER + 4, DataLen - 4);

                                DBG("[INFO]: Start playing song %s.\n", gLongFileName);
                            }
                            else
                            {
                                //DBG("CRC ERROR\n"));
                            }
						}
					}
                }
                else
                {
                    DBG("[INFO]: LOST SONG INFO\n");
                }
                break;

            case SSAP_CMD_PAUSE:
                DBG("[RCMD]: SSAP_CMD_PAUSE\n");
                PcmFifoPause();
                break;

            case SSAP_CMD_RESUME:
                DBG("[RCMD]: SSAP_CMD_RESUME\n");
                PcmFifoPlay();
                break;

            case SSAP_CMD_STOP:
                DBG("[RCMD]: SSAP_CMD_STOP\n");

                DecoderType = MP3_DECODER;

                PlayStopFlag   = FALSE;
                StreamHandle.mem_len = 0;
                StreamHandle.p       = 0;

                break;

            case SSAP_CMD_MUTE:
                DBG("[RCMD]: SSAP_CMD_MUTE\n");
                CodecDacMuteSet(TRUE, TRUE);
                break;

            case SSAP_CMD_UNMUTE:
                DBG("[RCMD]: SSAP_CMD_UNMUTE\n");
                CodecDacMuteSet(FALSE, FALSE);
                break;

            case SSAP_CMD_DATA:
                //DBG("[RCMD]: SSAP_CMD_DATA\n");

                DataLen = CommandContext->Content;

                //1. receive data
                //DBG("<- Receive %d data\n", DataLen));
                if(FindSpiStreamDataSyncWord(PlayStopFlag))
                {
                    uint32_t RemainLen = DataLen;
                    uint8_t* RecvBuff  = SPIS_RECEIVE_BUFFER;

 					while(RemainLen > 0)
					{
						RecvLen = SpiSlaveReceive(RecvBuff, DataLen, 10);

						RemainLen -= RecvLen;
						RecvBuff  += RecvLen;

						if(RemainLen == 0)
						{
                            bool StreamFullFlag = mv_mremain(&StreamHandle) < DataLen;

							SpiSlaveReceive((uint8_t*)&RecvCrc, SSAP_CRC_VALUE_LENGTH, 2);

							LocalCrc = GetCRC16NBS(SPIS_RECEIVE_BUFFER, DataLen);

                            //Send response
                            //DBG("<- Send response\n"));
                            ResponseContext->Response = StreamFullFlag ? SSAP_CMD_RESP_NEXTSEND : (RecvCrc == LocalCrc ? SSAP_CMD_RESP_OKSEND : SSAP_CMD_RESP_RESEND);
                            ResponseContext->CrcValue = GetCRC16NBS((uint8_t*)ResponseContext, sizeof(SSAPCmdResponseContext) - SSAP_CRC_VALUE_LENGTH);

                            SpiSlaveSend((uint8_t*)ResponseContext, sizeof(SSAPCmdResponseContext), sizeof(SSAPCmdResponseContext));
 
							#if SSAP_RESEND_IF_CRC_ERROR
                            if((StreamFullFlag == FALSE) && (RecvCrc == LocalCrc))
                            #else
                            if((StreamFullFlag == FALSE))
                            #endif
                            {
                                mv_mwrite(SPIS_RECEIVE_BUFFER, 1, DataLen, &StreamHandle);
                            }
                            else
                            {
                                //DBG("FULL or CRC ERROR\n");
                            }
						}

                        if((PlayStopFlag == FALSE) && (StreamHandle.mem_len >= (SPIS_STREAM_BUFFER_CAPACITY - SSAP_SEND_DATA_LENGTH)))
                        {
                            if(AudioDecoderInit(&StreamHandle, DecoderType))
                            {
                                PlayStopFlag = TRUE;
                            }
                            else
                            {
                                StreamHandle.mem_len = 0;
                                StreamHandle.p       = 0;
                            }
                        }

                        AudioDecoderDecodeOneFrame(PlayStopFlag);
					}                    
                }
                else
                {
                    ResponseContext->Response = SSAP_CMD_RESP_NEXTSEND;
                    SpiSlaveSend((uint8_t*)ResponseContext, sizeof(SSAPCmdResponseContext), sizeof(SSAPCmdResponseContext));
                    DBG("[INFO]: LOST SONG DATA\n");
                }

                break;

            case SSAP_CMD_VOL_SET:
                DBG("[RCMD]: SSAP_CMD_VOL_SET\n");                
                gDacVolumeIdx = CommandContext->Content * DAC_VOLUME_TABLE_SIZE / 100;
                gDacVolumeIdx = gDacVolumeIdx <= MAX_DAC_VOLUME_IDX ? gDacVolumeIdx : MAX_DAC_VOLUME_IDX;
                DacVolumeSet(DAC_VOLUME_TABLE[gDacVolumeIdx], DAC_VOLUME_TABLE[gDacVolumeIdx]);
                break;

            case SSAP_CMD_VOL_ADD:
                DBG("[RCMD]: SSAP_CMD_VOL_ADD\n");
                if(gDacVolumeIdx < MAX_DAC_VOLUME_IDX)
                {
                    gDacVolumeIdx++;
                }
                DacVolumeSet(DAC_VOLUME_TABLE[gDacVolumeIdx], DAC_VOLUME_TABLE[gDacVolumeIdx]);
                break;

            case SSAP_CMD_VOL_SUB:
                DBG("[RCMD]: SSAP_CMD_VOL_SUB\n");
                if(gDacVolumeIdx > MIN_DAC_VOLUME_IDX)
                {
                    gDacVolumeIdx--;
                }
                DacVolumeSet(DAC_VOLUME_TABLE[gDacVolumeIdx], DAC_VOLUME_TABLE[gDacVolumeIdx]);
                break;

            default:
                break;
            }
        }
        else
        {
            AudioDecoderDecodeOneFrame(PlayStopFlag);
        }
	}
}
