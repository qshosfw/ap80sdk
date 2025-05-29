/**
 **************************************************************************************
 * @file    ssap_master.c
 * @brief   SPI Stream Audio Player (Master)
 *
 * @author  Aissen Li
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include "type.h"
#include "app_config.h"
#include "crc.h"
#include "ssap_protocol.h"
#include "string_convert.h"
#include "clk.h"
#include "uart.h"
#include "gpio.h"
#include "spim.h"
#include "cache.h"
#include "delay.h"
#include "watchdog.h"
#include "fsinfo.h"
#include "fat_file.h"
#include "fs_mount.h"
#include "audio_decoder.h"
#include "timeout.h"
#include "sd_card.h"

#define  SPIM_STREAM_CONTROL_GPIO_BANK          ('B')
#define  SPIM_STREAM_CONTROL_GPIO_PIN           (5)
#define  SPIM_STREAM_CONTROL_GPIO_BANK_PIN      ((uint32_t)1 << SPIM_STREAM_CONTROL_GPIO_PIN)
#define  SPIM_STREAM_CONTROL_GPIO_BANK_PIN_IN   ((SPIM_STREAM_CONTROL_GPIO_BANK - 'A') * 10 + GPIO_A_IN)
#define  SPIM_STREAM_CONTROL_GPIO_BANK_PIN_IE   ((SPIM_STREAM_CONTROL_GPIO_BANK - 'A') * 10 + GPIO_A_IE)
#define  SPIM_STREAM_CONTROL_GPIO_BANK_PIN_OE   ((SPIM_STREAM_CONTROL_GPIO_BANK - 'A') * 10 + GPIO_A_OE)

#define  SPIM_STREAM_CONTROL_INIT()             \
         GpioSetRegOneBit(SPIM_STREAM_CONTROL_GPIO_BANK_PIN_IE, SPIM_STREAM_CONTROL_GPIO_BANK_PIN);    \
         GpioClrRegOneBit(SPIM_STREAM_CONTROL_GPIO_BANK_PIN_OE, SPIM_STREAM_CONTROL_GPIO_BANK_PIN);

#define  SPIM_STREAM_CONTROL_GET_STATUS()       \
         (GpioGetReg(SPIM_STREAM_CONTROL_GPIO_BANK_PIN_IN) & SPIM_STREAM_CONTROL_GPIO_BANK_PIN)

#define  SPIM_CONTROL_SPIS_GPIO_BANK            ('B')
#define  SPIM_CONTROL_SPIS_GPIO_PIN             (23)
#define  SPIM_CONTROL_SPIS_GPIO_BANK_PIN        ((uint32_t)1 << SPIM_CONTROL_SPIS_GPIO_PIN)
#define  SPIM_CONTROL_SPIS_GPIO_BANK_PIN_IE     ((SPIM_CONTROL_SPIS_GPIO_BANK - 'A') * 10 + GPIO_A_IE)
#define  SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OE     ((SPIM_CONTROL_SPIS_GPIO_BANK - 'A') * 10 + GPIO_A_OE)
#define  SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OUT    ((SPIM_CONTROL_SPIS_GPIO_BANK - 'A') * 10 + GPIO_A_OUT)

#define  SPIM_CONTROL_SPIS_INIT()               \
         GpioSetRegOneBit(SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OUT, SPIM_CONTROL_SPIS_GPIO_BANK_PIN);      \
         GpioSetRegOneBit(SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OE,  SPIM_CONTROL_SPIS_GPIO_BANK_PIN);      \
         GpioClrRegOneBit(SPIM_CONTROL_SPIS_GPIO_BANK_PIN_IE,  SPIM_CONTROL_SPIS_GPIO_BANK_PIN);
         
#define  SPIM_ENABLE_SPIS()                     \
         GpioClrRegOneBit(SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OUT, SPIM_CONTROL_SPIS_GPIO_BANK_PIN);

#define  SPIM_DISABLE_SPIS()                    \
         GpioSetRegOneBit(SPIM_CONTROL_SPIS_GPIO_BANK_PIN_OUT, SPIM_CONTROL_SPIS_GPIO_BANK_PIN);

static uint8_t SPIM_SEND_BUFFER[sizeof(SSAPCmdContext) + SSAP_SYNC_WORD_LENGTH + SSAP_SEND_DATA_LENGTH + SSAP_CRC_VALUE_LENGTH];

static FAT_FILE gFatFile;
static uint8_t  gLongFileName[FAT_NAME_MAX];

//extern void SysTickInit(void);
//extern bool SdCardInit(void);

static bool SdInitAndFsMount(void)
{
    //Deinit File System first.
    FSDeInit(DEV_ID_SD);

    GpioSdIoConfig(1);

    if(SdCardInit())
    {
        DBG("SD initialize fail.\n");
        return FALSE;
    }

    if(!FSInit(DEV_ID_SD))
    {
        DBG("FS mount fail.\n");
        return FALSE;
    }

    DBG("SD initialize and FS mount success.\n\n");

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

void SpimStreamAudioPlayerProcess(void* FileHandle, uint32_t* FileIndex, int32_t DecoderType)
{
    int32_t   FileLen   = FileSof(FileHandle);
    uint8_t*  pBuffer   = (uint8_t*)&SPIM_SEND_BUFFER[sizeof(SSAPCmdContext) + SSAP_SYNC_WORD_LENGTH];
    uint16_t* pCrcValue = (uint16_t*)&SPIM_SEND_BUFFER[sizeof(SSAPCmdContext) + SSAP_SYNC_WORD_LENGTH + SSAP_SEND_DATA_LENGTH];

    uint8_t*  p;
    uint32_t  Cnt;
    uint32_t  Flag;

    bool    PauseFlag     = FALSE;
    bool    MuteFlag      = FALSE;
    bool    ReturnFlag    = FALSE;
    bool    FileStartFlag = TRUE;
    bool    FileEndFlag   = TRUE;

    uint8_t KeyValue;

    SSAPCmdContext*         CommandContext  = (SSAPCmdContext*)SPIM_SEND_BUFFER;
    SSAPCmdResponseContext* ResponseContext = (SSAPCmdResponseContext*)SPIM_SEND_BUFFER;

    DBG("[INFO]: Begin of file\n");
    DBG("[SCMD]: START\n");
    
    CommandContext->SyncWord = SSAP_SYNC_WORD_LE;
    CommandContext->Command  = SSAP_CMD_START;
    CommandContext->Content  = 4 + strlen((const char*)gLongFileName);
    CommandContext->CrcValue = GetCRC16NBS((uint8_t*)CommandContext, sizeof(SSAPCmdContext) - SSAP_CRC_VALUE_LENGTH);

    //Set decoder type
    *(int32_t*)pBuffer = DecoderType;
    //Set song name
    memcpy(pBuffer + 4, gLongFileName, strlen((const char*)gLongFileName));
    //Calculate CRC value
    *(uint16_t*)(pBuffer + CommandContext->Content) = GetCRC16NBS(pBuffer, CommandContext->Content);

    while(FileStartFlag)
    {
        while(SPIM_STREAM_CONTROL_GET_STATUS());

        //Send UART comand to slaver
        SPIM_ENABLE_SPIS();
        SpiMasterSendData((uint8_t*)CommandContext, sizeof(SSAPCmdContext) + SSAP_SYNC_WORD_LENGTH + CommandContext->Content + SSAP_CRC_VALUE_LENGTH);
        SPIM_DISABLE_SPIS();
        
        //Receive response
        p = (uint8_t*)&ResponseContext->SyncWord;

        Cnt  = 0;
        Flag = FALSE;
        while(1)
        {
            SPIM_ENABLE_SPIS();
            p[0] = SpiMasterRecvByte();
            SPIM_DISABLE_SPIS();

            if(p[0] == SSAP_SYNC_BYTE)
            {
                #if (SSAP_SYNC_WORD_LENGTH - 1)
                SPIM_ENABLE_SPIS();
                SpiMasterRecvData((uint8_t*)&p[1], (SSAP_SYNC_WORD_LENGTH - 1));
                SPIM_DISABLE_SPIS();

                if(ResponseContext->SyncWord == SSAP_SYNC_WORD_LE)
                #endif

                {
                    SPIM_ENABLE_SPIS();
                    SpiMasterRecvData((uint8_t*)&ResponseContext->Command, (sizeof(SSAPCmdContext) - SSAP_SYNC_WORD_LENGTH));
                    SPIM_DISABLE_SPIS();

                    Flag = TRUE;
                    break;
                }
            }

            if(Cnt++ > 10000)
            {
                break;
            }
        }

        if(Flag && ResponseContext->Response == SSAP_CMD_RESP_OKSEND)
        {
            FileStartFlag = FALSE;
        }
        else
        {
            CommandContext->SyncWord = SSAP_SYNC_WORD_LE;
            CommandContext->Command  = SSAP_CMD_START;
            CommandContext->Content  = 4 + strlen((const char*)gLongFileName);
            CommandContext->CrcValue = GetCRC16NBS((uint8_t*)CommandContext, sizeof(SSAPCmdContext) - SSAP_CRC_VALUE_LENGTH);
        }
    }

    DBG("[INFO]: Send song info done.\n");

    while(1)
    {
        FileLen -= SSAP_SEND_DATA_LENGTH;

        FileEndFlag = FileLen <= 0;

        if(!FileEndFlag)
        {
            FileRead(pBuffer, 1, SSAP_SEND_DATA_LENGTH, FileHandle);

            *pCrcValue = GetCRC16NBS(pBuffer, SSAP_SEND_DATA_LENGTH);
        }

        while(1)
        {
            CommandContext->SyncWord = SSAP_SYNC_WORD_LE;

            ReturnFlag = FALSE;

            #if 1//支持命令
            if(FileEndFlag)
            {
                DBG("[INFO]: End of file\n");
                DBG("[SCMD]: STOP\n");

                CommandContext->Command  = SSAP_CMD_STOP;
                CommandContext->CrcValue = GetCRC16NBS((uint8_t*)CommandContext, sizeof(SSAPCmdContext) - SSAP_CRC_VALUE_LENGTH);

                while(SPIM_STREAM_CONTROL_GET_STATUS());

                //Send UART comand to slaver
                SPIM_ENABLE_SPIS();
                SpiMasterSendData((uint8_t*)CommandContext, sizeof(SSAPCmdContext));
                SPIM_DISABLE_SPIS();

                return;
            }
            //Process UART command
            else if(0 != (KeyValue = GetKeyValue()))
            {
                SetKeyValue(0);

                switch(KeyValue)
                {
                case 0x20://space: play/pause
                    PauseFlag = !PauseFlag;
                    DBG("[SCMD]: %s\n", PauseFlag ? "PAUSE" : "RESUME");
                    CommandContext->Command = PauseFlag ? SSAP_CMD_PAUSE : SSAP_CMD_RESUME;
                    break;

                case 'm'://mute/unmute                
                    MuteFlag = !MuteFlag;
                    DBG("[SCMD]: %s\n", MuteFlag ? "MUTE" : "UNMUTE");
                    CommandContext->Command = MuteFlag ? SSAP_CMD_MUTE : SSAP_CMD_UNMUTE;
                    break;

                case 'e':
                    DBG("[SCMD]: STOP\n");
                    CommandContext->Command = SSAP_CMD_STOP;
                    ReturnFlag = TRUE;
                    break;

                case 0x93://up
                case 'k' :
                    DBG("[SCMD]: VOL+\n");
                    CommandContext->Command = SSAP_CMD_VOL_ADD;
                    break;

                case 0x95://down
                case 'j' :
                    DBG("[SCMD]: VOL-\n");
                    CommandContext->Command = SSAP_CMD_VOL_SUB;
                    break;

                case 0x92://left
                case 'h' :
                    *FileIndex = *FileIndex > 2 ? *FileIndex - 2 : 1;
                    DBG("[SCMD]: PREVIOUS\n");
                    CommandContext->Command = SSAP_CMD_STOP;
                    ReturnFlag = TRUE;
                    break;

                case 0x94://right
                case 'l' :
                    DBG("[SCMD]: NEXT\n");
                    CommandContext->Command = SSAP_CMD_STOP;
                    ReturnFlag = TRUE;
                    break;

                default  :
                    DBG("[SCMD]: UNKNOW\n");
                    CommandContext->Command = SSAP_CMD_UNKOWN;
                    break;
                }

                if(CommandContext->Command != SSAP_CMD_UNKOWN)
                {
                    CommandContext->CrcValue = GetCRC16NBS((uint8_t*)CommandContext, sizeof(SSAPCmdContext) - SSAP_CRC_VALUE_LENGTH);

                    while(SPIM_STREAM_CONTROL_GET_STATUS());

                    //Send UART comand to slaver
                    SPIM_ENABLE_SPIS();
                    SpiMasterSendData((uint8_t*)CommandContext, sizeof(SSAPCmdContext));
                    SPIM_DISABLE_SPIS();

                    if(ReturnFlag)
                    {
                        return;
                    }
                }
            }
            #else
            if(FileEndFlag)
            {
                return;
            }
            #endif
            else//Send DATA
            {
                while(SPIM_STREAM_CONTROL_GET_STATUS());

                //1. Send SSAP_CMD_DATA command
                //DBG("-> Send command SSAP_CMD_DATA\n"));        
                CommandContext->Command  = SSAP_CMD_DATA;
                CommandContext->Content  = SSAP_SEND_DATA_LENGTH;
                CommandContext->CrcValue = GetCRC16NBS((uint8_t*)CommandContext, sizeof(SSAPCmdContext) - SSAP_CRC_VALUE_LENGTH);

                #if 0 //分开发送

                SPIM_ENABLE_SPIS();
                SpiMasterSendData((uint8_t*)CommandContext, sizeof(SSAPCmdContext));
                SPIM_DISABLE_SPIS();

                //2. Send data
                //DBG("-> Send data\n"));
                SPIM_ENABLE_SPIS();
                SpiMasterSendData(&SPIM_SEND_BUFFER[sizeof(SSAPCmdContext)], sizeof(SPIM_SEND_BUFFER) - sizeof(SSAPCmdContext));
                SPIM_DISABLE_SPIS();

                #else //合并发送

                SPIM_ENABLE_SPIS();
                SpiMasterSendData(SPIM_SEND_BUFFER, sizeof(SPIM_SEND_BUFFER));
                SPIM_DISABLE_SPIS();

                #endif

                //3. Receive response
                //DBG("-> Receive response\n"));
                p = (uint8_t*)&ResponseContext->SyncWord;

                Cnt  = 0;
                Flag = FALSE;
                while(1)
                {
                    SPIM_ENABLE_SPIS();
                    p[0] = SpiMasterRecvByte();
                    SPIM_DISABLE_SPIS();

                    if(p[0] == SSAP_SYNC_BYTE)
                    {
                        #if (SSAP_SYNC_WORD_LENGTH - 1)
                        SPIM_ENABLE_SPIS();
                        SpiMasterRecvData((uint8_t*)&p[1], (SSAP_SYNC_WORD_LENGTH - 1));
                        SPIM_DISABLE_SPIS();

                        if(ResponseContext->SyncWord == SSAP_SYNC_WORD_LE)
                        #endif

                        {
                            SPIM_ENABLE_SPIS();
                            SpiMasterRecvData((uint8_t*)&ResponseContext->Command, (sizeof(SSAPCmdContext) - SSAP_SYNC_WORD_LENGTH));
                            SPIM_DISABLE_SPIS();

                            Flag = TRUE;
                            break;
                        }
                    }

                    if(Cnt++ > 10000)
                    {
                        break;
                    }
                }

                if(Flag)
                {
                    if(ResponseContext->Command == SSAP_CMD_DATA)
                    {
                        if(ResponseContext->Response == SSAP_CMD_RESP_OKSEND)
                        {
                            break;
                        }
                        else if(ResponseContext->Response == SSAP_CMD_RESP_NEXTSEND)
                        {
                            //DBG("[INFO]: FULL\n"));
                        }
                        else if(ResponseContext->Response == SSAP_CMD_RESP_RESEND)
                        {
                            DBG("[INFO]: CRC ERROR\n");

                            if(!SSAP_RESEND_IF_CRC_ERROR)
                            {
                                break;
                            }
                        }
                    }
                }
                else
                {
                    DBG("[INFO]: LOST\n");
                }
            }
        }
    }
}

bool CheckAllDiskLinkFlag(void)
{
    return TRUE;
}

int32_t main(void)
{
    FAT_FILE* FileHandle = &gFatFile;

    uint32_t FileIndex;
    uint32_t FileCount;

    uint8_t  KeyValue;

    bool IsSdInitAndFsMount;

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
    DBG("|             SPI STREAM AUDIO PLAYER APPLICATION (MASTER)                 |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");

	//SPI master initialization
    SPIM_STREAM_CONTROL_INIT();
    SPIM_CONTROL_SPIS_INIT();
    GpioSpimIoConfig(3);
    SpiMasterInit(0, SPIM_CLK_DIV_24M);

    *(uint32_t*)(&SPIM_SEND_BUFFER[sizeof(SSAPCmdContext)]) = SSAP_SYNC_WORD_LE;

    FileIndex = 1;
    IsSdInitAndFsMount = SdInitAndFsMount();

    while(1)
    {
        if(IsSdInitAndFsMount)
        {
            FileCount = FsGetCurContext()->gFsInfo.FileSum;

            if(FileIndex > FileCount)
            {
                FileIndex = 1;
            }

            while(FileOpenByNum(FileHandle, NULL, FileIndex++))
            {
                int32_t DecoderType = UNKOWN_DECODER;

                if((FileHandle->ShortName[8] == 'W' && FileHandle->ShortName[9] == 'M' && FileHandle->ShortName[10] == 'A') ||
                   (FileHandle->ShortName[8] == 'W' && FileHandle->ShortName[9] == 'M' && FileHandle->ShortName[10] == 'V') ||
                   (FileHandle->ShortName[8] == 'A' && FileHandle->ShortName[9] == 'S' && FileHandle->ShortName[10] == 'F'))
                {
                    DecoderType = WMA_DECODER;
                }
                else if((FileHandle->ShortName[8] == 'M' && FileHandle->ShortName[9] == 'P' && FileHandle->ShortName[10] == '2') ||
                        (FileHandle->ShortName[8] == 'M' && FileHandle->ShortName[9] == 'P' && FileHandle->ShortName[10] == '3'))
                {
                    DecoderType = MP3_DECODER;
                }
                else if((FileHandle->ShortName[8] == 'S' && FileHandle->ShortName[9] == 'B' && FileHandle->ShortName[10] == 'C'))
                {
                    DecoderType = SBC_DECODER;
                }
                else if((FileHandle->ShortName[8] == 'W' && FileHandle->ShortName[9] == 'A' && FileHandle->ShortName[10] == 'V'))
                {
                    DecoderType = WAV_DECODER;
                }
                else if((FileHandle->ShortName[8] == 'F' && FileHandle->ShortName[9] == 'L' && FileHandle->ShortName[10] == 'A'))
                {
                    DecoderType = FLAC_DECODER;
                }
                else if((FileHandle->ShortName[8] == 'A' && FileHandle->ShortName[9] == 'A' && FileHandle->ShortName[10] == 'C'))
                {
                    DecoderType = AAC_DECODER;
                }
                else if((FileHandle->ShortName[8] == 'A' && FileHandle->ShortName[9] == 'I' && FileHandle->ShortName[10] == 'F'))
                {
                    DecoderType = AIF_DECODER;
                }

                if((DecoderType == MP3_DECODER) ||
                   (DecoderType == SBC_DECODER) ||
                   (DecoderType == AAC_DECODER))
                {               
                    int32_t i;

                    memset((void*)gLongFileName, 0, sizeof(gLongFileName));

                    if(FileGetLongName((void*)FileHandle, gLongFileName, sizeof(gLongFileName)))
                    {
                        StrUnicode2Gbk((uint16_t*)gLongFileName, sizeof(gLongFileName) / 2);                
                    }
                    else
                    {
                        memcpy(gLongFileName, FileHandle->ShortName, 8);
                        for(i = 7; i >= 0; i--)
                        {
                            if(gLongFileName[i] == 0x20)
                            {
                                gLongFileName[i] = 0x00;
                            }
                            else
                            {
                                break;
                            }
                        }
                        i++;
                        gLongFileName[i++] = '.';
                        gLongFileName[i++] = FileHandle->ShortName[8];
                        gLongFileName[i++] = FileHandle->ShortName[9];
                        gLongFileName[i++] = FileHandle->ShortName[10];
                    }
                    
                    DBG("\n\n[INFO]: Playing song %s\n", gLongFileName);

                    SpimStreamAudioPlayerProcess(FileHandle, &FileIndex, DecoderType);				
                }

                FileClose(FileHandle);
            }
        }

        //continue;

        DBG("\n[INFO]: All Songs Played.\n");
        DBG("\n[INFO]: Press the follow keys to continue:\n");
        DBG("        [R    ]: RESTART\n");
        DBG("        [p    ]: REPLAY\n");
        DBG("        [LEFT ]: PRIVIOUS\n");
        DBG("        [UP   ]: PRIVIOUS\n");
        DBG("        [RIGHT]: NEXT\n");
        DBG("        [DOWN ]: NEXT\n");
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
            case 'R':
                DBG("[KCMD]: RESTART\n");
                FileIndex = 1;
                IsSdInitAndFsMount = SdInitAndFsMount();
                break;               

            case 'p' :
                FileIndex = 1;
                DBG("[KCMD]: REPLAY\n");
                break;

            case 0x93://up
            case 'k' :
                FileIndex = FileIndex - 2;
                DBG("[KCMD]: PREVIOUS\n");
                break;

            case 0x95://down
            case 'j' :
                FileIndex = 1;
                DBG("[KCMD]: NEXT\n");
                break;

            case 0x92://left
            case 'h' :
                FileIndex = FileIndex - 2;
                DBG("[KCMD]: PREVIOUS\n");
                break;

            case 0x94://right
            case 'l' :
                FileIndex = 1;
                DBG("[KCMD]: NEXT\n");
                break;

            default  :
                DBG("[KCMD]: UNKNOW\n");
                BreakFlag = FALSE;
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
