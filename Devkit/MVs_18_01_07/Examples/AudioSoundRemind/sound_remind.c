///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: sound_remind.c
//  maintainer: Halley
///////////////////////////////////////////////////////////////////////////////

#include "app_config.h"
#include "sound_remind.h"
#include "dac.h"
#include "clk.h"
#include "pcm_fifo.h"
#include "spi_flash.h"
#include "audio_utility.h"
#include "debug.h"

#include "audio_decoder.h"
#include "timeout.h"
#include "delay.h"
#include "uart.h"

#include "pcm_fifo.h"
#include "pcm_transfer.h"

//��Ӣ��˫����ʾ��
typedef struct _SOUND_TABLE_ITEM
{
 	uint16_t Id;
	uint32_t IdChn;
	uint32_t IdEng;

} SOUND_TABLE_ITEM;

#define STR_TO_UINT(s)	((s[0] << 0) | (s[1] << 8) | (s[2] << 16) | (s[3] << 24))	
static const SOUND_TABLE_ITEM SoundTable[] = 
{
//   ID                                 ����ID                  Ӣ��ID
/*	{SOUND_NUM0,					STR_TO_UINT("NUM0"),	STR_TO_UINT("ZERO")},	//0  
 	{SOUND_NUM1,					STR_TO_UINT("NUM1"),	STR_TO_UINT("ONE ")},	//1    
 	{SOUND_NUM2,					STR_TO_UINT("NUM2"),	STR_TO_UINT("TWO ")},	//2   
 	{SOUND_NUM3,					STR_TO_UINT("NUM3"),	STR_TO_UINT("THRE")},	//3    
 	{SOUND_NUM4,					STR_TO_UINT("NUM4"),	STR_TO_UINT("FOUR")},	//4  
 	{SOUND_NUM5,					STR_TO_UINT("NUM5"),	STR_TO_UINT("FIVE")},	//5   
 	{SOUND_NUM6,					STR_TO_UINT("NUM6"),	STR_TO_UINT("SIX ")},	//6 
 	{SOUND_NUM7,					STR_TO_UINT("NUM7"),	STR_TO_UINT("SEVN")},	//7  
 	{SOUND_NUM8,					STR_TO_UINT("NUM8"),	STR_TO_UINT("EIGT")},	//8    
 	{SOUND_NUM9,					STR_TO_UINT("NUM9"),	STR_TO_UINT("NINE")},	//9   
 	{SOUND_PLUS5,					STR_TO_UINT("JIA5"),	STR_TO_UINT("PLS5")},	//��5
 	{SOUND_SPEECH_OP,				STR_TO_UINT("YYDK"),	STR_TO_UINT("SPOP")},	//������
 	{SOUND_SPEECH_CL,				STR_TO_UINT("YYGB"),	STR_TO_UINT("SPCL")},	//�����ر�	
 	{SOUND_SPEECH_CL_FORCE,			STR_TO_UINT("YYQZ"),	STR_TO_UINT("COCS")},	//����ǿ�ƹر�
 	{SOUND_NEXT_FOLD,				STR_TO_UINT("XYML"),	STR_TO_UINT("NXFD")},	//��һĿ¼
 	{SOUND_REP_1,					STR_TO_UINT("DQXH"),	STR_TO_UINT("REP1")},	//����ѭ��
 	{SOUND_REP_ALL,					STR_TO_UINT("QBXH"),	STR_TO_UINT("RPAL")},	//ȫ��ѭ��
 	{SOUND_START_REC,				STR_TO_UINT("KSLY"),	STR_TO_UINT("STRE")},	//��ʼ¼��
 	{SOUND_SAVE_REC,				STR_TO_UINT("LYBC"),	STR_TO_UINT("SARE")},	//¼������
 	{SOUND_REC_PLAYBACK,			STR_TO_UINT("LYBF"),	STR_TO_UINT("REPL")},	//¼������
 	{SOUND_DELETE_REC,				STR_TO_UINT("SCLY"),	STR_TO_UINT("DERE")},	//ɾ��¼��
 	{SOUND_LINEIN,					STR_TO_UINT("YPSR"),	STR_TO_UINT("LINE")},	//��Ƶ����
 	{SOUND_UDISC_PLAYBACK,			STR_TO_UINT("YPBF"),	STR_TO_UINT("UDPB")},	//u�̲���
 	{SOUND_CARD_PLAYBACK,			STR_TO_UINT("C BF"),	STR_TO_UINT("CDPB")},	//������
 	{SOUND_RADIO_MODE,				STR_TO_UINT("SYMS"),	STR_TO_UINT("RDMD")},	//����ģʽ
 	{SOUND_PREV_TRACK,				STR_TO_UINT("S YS"),	STR_TO_UINT("PRSO")},	//��һ��
 	{SOUND_NEXT_TRACK,				STR_TO_UINT("X YS"),	STR_TO_UINT("NESN")},	//��һ��
 	{SOUND_PAUSE,					STR_TO_UINT("Z  T"),	STR_TO_UINT("PAUS")},	//��ͣ
 	{SOUND_STOP,					STR_TO_UINT("T  Z"),	STR_TO_UINT("STOP")},	//ֹͣ
	{SOUND_ECHO_REGULATE,			STR_TO_UINT("HXTJ"),	STR_TO_UINT("ECRE")},	//�������
 	{SOUND_ECHO_PLUS,				STR_TO_UINT("HXAD"),	STR_TO_UINT("ECPL")},	//�����
 	{SOUND_ECHO_REDUCE,				STR_TO_UINT("HXDE"),	STR_TO_UINT("ECRD")},	//�����
 	{SOUND_DELAY_REGULATE,			STR_TO_UINT("YSTJ"),	STR_TO_UINT("DLRE")},	//��ʱ����
 	{SOUND_DELAY_PLUS,				STR_TO_UINT("YSAD"),	STR_TO_UINT("DLPL")},	//��ʱ��
 	{SOUND_DELAY_REDUCE,			STR_TO_UINT("YSDE"),	STR_TO_UINT("DLRD")},	//��ʱ��
 	{SOUND_TREB_REGULATE,			STR_TO_UINT("GYTJ"),	STR_TO_UINT("TRRE")},	//��������
 	{SOUND_TREB_PLUS,				STR_TO_UINT("GYAD"),	STR_TO_UINT("TRPL")},	//������
 	{SOUND_TREB_REDUCE,				STR_TO_UINT("GYDE"),	STR_TO_UINT("TRRD")},	//������
 	{SOUND_BASS_REGULATE,			STR_TO_UINT("DYTJ"),	STR_TO_UINT("BARE")},	//��������
 	{SOUND_BASS_PLUS,				STR_TO_UINT("DYAD"),	STR_TO_UINT("BAPL")},	//������
	{SOUND_BASS_REDUCE,				STR_TO_UINT("DYDE"),	STR_TO_UINT("BARD")},	//������
 	{SOUND_MIC_OPEN,				STR_TO_UINT("HTDK"),	STR_TO_UINT("MIOP")},	//��Ͳ��
 	{SOUND_MIC_CLOSE,				STR_TO_UINT("HTGB"),	STR_TO_UINT("MICL")},	//��Ͳ�ر�
	{SOUND_EQ_FLAT,					STR_TO_UINT("EQZR"),	STR_TO_UINT("FLAT")},	//��Ȼ
 	{SOUND_EQ_CLASSIC,				STR_TO_UINT("EQGD"),	STR_TO_UINT("CLAS")},	//�ŵ�
 	{SOUND_EQ_POP,					STR_TO_UINT("EQLX"),	STR_TO_UINT("POP ")},	//����
 	{SOUND_EQ_ROCK,					STR_TO_UINT("EQYG"),	STR_TO_UINT("ROCK")},	//ҡ��
 	{SOUND_EQ_JAZZ,					STR_TO_UINT("EQJS"),	STR_TO_UINT("JAZZ")},	//��ʿ
 	{SOUND_EQ_SOFT,					STR_TO_UINT("EQRH"),	STR_TO_UINT("SOFT")},	//���
 	{SOUND_SUBBASS,					STR_TO_UINT("Z DY"),	STR_TO_UINT("SUBA")},	//�ص���
 	{SOUND_MENU,					STR_TO_UINT("C  D"),	STR_TO_UINT("MENU")},	//�˵�
 	{SOUND_BT_MODE,					STR_TO_UINT("BTMS"),	STR_TO_UINT("BTMD")},	//����ģʽ
 	{SOUND_BT_CONNECT,				STR_TO_UINT("BTLJ"),	STR_TO_UINT("BTCN")},	//��������
 	{SOUND_BT_DISCONNECT,			STR_TO_UINT("BTDK"),	STR_TO_UINT("BTDN")},	//�����Ͽ�
 	{SOUND_POINT,					STR_TO_UINT("DIAN"),	STR_TO_UINT("POIT")},	//��
 	{SOUND_LANG_MODE,				STR_TO_UINT("ZWMS"),	STR_TO_UINT("ENMD")},	//����ģʽ/Ӣ��ģʽ
 	{SOUND_USB_AUDIO_MODE,			STR_TO_UINT("SKMS"),	STR_TO_UINT("UAMD")},	//����ģʽ
 	{SOUND_USB_AUDIO_READER_MODE,	STR_TO_UINT("YX T"),	STR_TO_UINT("ARMD")},	//һ��ͨģʽ
 	{SOUND_PLAY,		        	STR_TO_UINT("B  F"),	STR_TO_UINT("PLAY")},	//����
 	{SOUND_FF,		        		STR_TO_UINT("K  J"),	STR_TO_UINT("F  F")},	//���
 	{SOUND_FB,						STR_TO_UINT("K  T"),	STR_TO_UINT("F  B")},	//����
 	{SOUND_MHZ,						STR_TO_UINT("ZHHZ"),	STR_TO_UINT("M HZ")},	//�׺���
 	{SOUND_REP_FOLD,				STR_TO_UINT("MLXH"),	STR_TO_UINT("RPFD")},	//Ŀ¼ѭ��
 	{SOUND_INTRO,					STR_TO_UINT("LLBF"),	STR_TO_UINT("INTR")},	//�������
 	{SOUND_RANDOM,					STR_TO_UINT("SJBF"),	STR_TO_UINT("RAND")},	//�������
 	{SOUND_AUTO_SCAN,				STR_TO_UINT("ZDST"),	STR_TO_UINT("ATSC")},	//�Զ���̨
 	{SOUND_PRE_STATION,				STR_TO_UINT("SYDT"),	STR_TO_UINT("PRST")},	//��һ��̨
 	{SOUND_NXT_STATION,				STR_TO_UINT("XYDT"),	STR_TO_UINT("NXST")},	//��һ��̨
 	{SOUND_PWR_OFF,					STR_TO_UINT("YIGJ"),	STR_TO_UINT("PWOF")},	//�ѹػ�
 	{SOUND_GUITAR_VOL,				STR_TO_UINT("GTYL"),	STR_TO_UINT("GUVL")},	//��������
 	{SOUND_SEARCHING,				STR_TO_UINT("SSZH"),	STR_TO_UINT("SEAR")},	//������
 	{SOUND_MUTE_ON,					STR_TO_UINT("JYDK"),	STR_TO_UINT("MUON")},	//������
 	{SOUND_MUTE_OFF,				STR_TO_UINT("JYGB"),	STR_TO_UINT("MUOF")},	//�����ر�
 	{SOUND_MIC_VOL,					STR_TO_UINT("HTYL"),	STR_TO_UINT("MIVL")},	//��Ͳ����
 	{SOUND_MAIN_VOL,				STR_TO_UINT("ZHYL"),	STR_TO_UINT("MAVL")},	//������
 	{SOUND_MIC_MODE_OPEN,			STR_TO_UINT("HTMK"),	STR_TO_UINT("MMOP")},	//��Ͳģʽ��
 	{SOUND_MIC_MODE_CLOSE,			STR_TO_UINT("HTMG"),	STR_TO_UINT("MMCL")},	//��Ͳģʽ��
 	{SOUND_MIC_FIRST_OPEN,			STR_TO_UINT("HTYK"),	STR_TO_UINT("MFOP")},	//MIC���ȿ�
 	{SOUND_MIC_FIRST_CLOSE,			STR_TO_UINT("HTYG"),	STR_TO_UINT("MFCL")},	//MIC���ȹ�
 	{SOUND_OPEN,					STR_TO_UINT("D  K"),	STR_TO_UINT("OPEN")},	//��
 	{SOUND_CLOSE,					STR_TO_UINT("G  B"),	STR_TO_UINT("CLOS")},	//�ر�
 	{SOUND_BAT_LOW_PWR,				STR_TO_UINT("DLDI"),	STR_TO_UINT("LPWR")},	//��ص����ͣ�����
 	{SOUND_CHARGING,				STR_TO_UINT("ZZCD"),	STR_TO_UINT("CHAG")},	//���ڳ��
 	{SOUND_BAT_FULL,				STR_TO_UINT("DLYM"),	STR_TO_UINT("FULL")},	//�����ѳ���
 	{SOUND_WAIT_PAIR,				STR_TO_UINT("DDPD"),	STR_TO_UINT("WTPA")},	//�ȴ�����豸
 	{SOUND_SHI,						STR_TO_UINT("S HI"),	STR_TO_UINT("T EN")},	//ʮ
 	{SOUND_BAI,						STR_TO_UINT("B AI"),	STR_TO_UINT("HDRD")},	//��
 	{SOUND_MIC_INSERT,				STR_TO_UINT("HTCR"),	STR_TO_UINT("MCIN")},	//��Ͳ����
 	{SOUND_MIC_PULLOUT,				STR_TO_UINT("HTBC"),	STR_TO_UINT("MCOU")},	//��Ͳ�γ�
 	{SOUND_50MS,					STR_TO_UINT("WUSH"),	STR_TO_UINT("FIFT")},	//50
 	{SOUND_100MS,					STR_TO_UINT("YIBA"),	STR_TO_UINT("OHDD")},	//100
 	{SOUND_150MS,					STR_TO_UINT("YBWS"),	STR_TO_UINT("OHFF")},	//150
 	{SOUND_200MS,					STR_TO_UINT("LBAI"),	STR_TO_UINT("THDD")},	//200  	
*/

 	{SOUND_ON,						STR_TO_UINT("K  G"),	STR_TO_UINT("PWON")},	//ON  	
 	{SOUND_OFF,						STR_TO_UINT("G  J"),	STR_TO_UINT("PWOF")},	//OFF  	
 	{SOUND_BAT_LOW,					STR_TO_UINT("DLGD"),	STR_TO_UINT("BTLO")},	//��ص�������
 	{SOUND_CHARGING,				STR_TO_UINT("ZZCD"),	STR_TO_UINT("CHAR")},	//��ؿ�ʼ���
 	{SOUND_BAT_FULL,				STR_TO_UINT("CDWC"),	STR_TO_UINT("FULL")},	//��س�����
 	{SOUND_START_WORKING,			STR_TO_UINT("KSSD"),	STR_TO_UINT("SWEP")},	//����ɨ��ģʽ
	
};

typedef struct _SOUND_REMIND_CONTROL
{
	bool     IsRunning;

	uint32_t ConstDataAddr;
	uint32_t ConstDataSize;
	uint32_t ConstDataOffset;

} SOUND_REMIND_CONTROL;

static SOUND_REMIND_CONTROL SoundRemindControl;
static MemHandle SoundRemindFile;		//��ʾ���ļ�
static uint8_t SoundRemindFileBuf[256];	//��ʾ���ļ�������

//decoder task״̬
#define DECODER_STATE_STOP			0
#define DECODER_STATE_PLAY			1
#define DECODER_STATE_PAUSE			2
static uint8_t DecoderTaskState = DECODER_STATE_STOP;

//��ǰ�ļ�
static void*    DecoderFileHandle = NULL;

TIMER SoundRemindTimer;

uint32_t SoundRemindFillStreamCallback(void *buffer, uint32_t length)
{
    int32_t RemainBytes = SoundRemindControl.ConstDataSize - SoundRemindControl.ConstDataOffset;
    int32_t ReadBytes   = length > RemainBytes ? RemainBytes : length;

    if(ReadBytes == 0)
	{
		return 0;	//�˴β���������
	}
	
    APP_DBG("SoundRemind Fill Stream Callback...\n");
    
    if(SpiFlashConstDataRead((uint8_t*)buffer,
	                         ReadBytes,
	                         SoundRemindControl.ConstDataAddr + SoundRemindControl.ConstDataOffset) < 0)
	{
		ReadBytes = 0;
        APP_DBG("read const data error!\r\n");
	}

	SoundRemindControl.ConstDataOffset += ReadBytes;

	return ReadBytes;
}

/////////////////////////////////////////////////////////////////////////
// stream ����������䣨decoder taskÿ�ν�֡ʱ���ã�
// return -1 -- stream over; 0 -- no fill; other value -- filled bytes
int32_t SoundRemindFillStream(void)
{
	int32_t FillBytes;	//����Ҫ���ص����ݳ���
	int32_t LeftBytes;	//�ļ������������е����ݳ���

    APP_DBG("SoundRemind Fill Stream...\n");
    
	LeftBytes = (SoundRemindFile.mem_len - SoundRemindFile.p);
	FillBytes = SoundRemindControl.ConstDataSize - SoundRemindControl.ConstDataOffset;
	if(FillBytes > SoundRemindFile.mem_capacity - LeftBytes)
	{
		FillBytes = SoundRemindFile.mem_capacity - LeftBytes;
	}

	if((LeftBytes > 0) && (SoundRemindFile.p == 0))
	{
		return 0;	//�������ݣ���������û������
	} 

	if(FillBytes == 0)
	{
		return -1;	//���Ž���
	}

	memcpy(SoundRemindFile.addr, SoundRemindFile.addr + SoundRemindFile.p, LeftBytes);
	if(SpiFlashConstDataRead((uint8_t*)(SoundRemindFile.addr + LeftBytes),
	                         FillBytes,
	                         SoundRemindControl.ConstDataAddr + SoundRemindControl.ConstDataOffset) < 0)
	{
		APP_DBG("read const data error!\n");
	}
		
	SoundRemindFile.p = 0;
	SoundRemindFile.mem_len = LeftBytes + FillBytes;
	SoundRemindControl.ConstDataOffset += FillBytes;

	return FillBytes;
}

//����������ʾ��
//ע�⣺�ú���������main task�е��ã����������������
void SoundRemind(uint16_t SoundId)
{     
	uint16_t i;
	uint32_t ConstDataId;
	
    APP_DBG("Enter Sound Remind...\n");
    
	if(SoundRemindControl.IsRunning)
	{
		return;
	}
	APP_DBG("SoundRemind id : %d\n", SoundId);

	SoundRemindControl.IsRunning = 1;
	SoundRemindControl.ConstDataSize = 0;
	SoundRemindControl.ConstDataAddr = 0;
	SoundRemindControl.ConstDataOffset = 0;
	
	SoundRemindFile.addr = SoundRemindFileBuf;
	SoundRemindFile.mem_capacity = sizeof(SoundRemindFileBuf);    // �����ݻ����С
	SoundRemindFile.mem_len = 0;
	SoundRemindFile.p = 0;

	//ͨ��SoundId���Ҷ�Ӧ��ConstDataId
	for(i = 0; i < sizeof(SoundTable)/sizeof(SOUND_TABLE_ITEM); i++)
	{
		if(SoundTable[i].Id == SoundId)
		{
			//ConstDataId = (gSys.LanguageMode == LANG_ZH) ? SoundTable[i].IdChn : SoundTable[i].IdEng; 
			ConstDataId = SoundTable[i].IdChn;//IdEng; 
			break;	//�ҵ�
		}
	}	
	if(i >= sizeof(SoundTable)/sizeof(SOUND_TABLE_ITEM))
	{
		APP_DBG("Can not get the SoundId : %d\n", SoundId);
		SoundRemindControl.IsRunning = 0;
		return;		//δ�ҵ�
	}
    APP_DBG("the SoundId is available. \n");
	
    DBG("$$$$$$$$$$\n");
    DBG("ConstDataId = 0x%x\n", ConstDataId);
    DBG("ConstDataSize = 0x%x\n", SoundRemindControl.ConstDataSize);
    DBG("ConstDataAddr = 0x%x\n", SoundRemindControl.ConstDataAddr);
    DBG("$$$$$$$$$$\n");
    
	if(SpiFlashConstGetInfo(ConstDataId, &SoundRemindControl.ConstDataSize, &SoundRemindControl.ConstDataAddr) < 0)
	{
		APP_DBG("Get const data info error!\n");
		SoundRemindControl.IsRunning = 0;
		return;
	}
    APP_DBG("Get const data info from spi flash = OK!\n");

	SoundRemindFillStream();
    
    DecoderTaskPlayStart(&SoundRemindFile, IO_TYPE_MEMORY, 0);
}

//��ʼ��decoder�������ص�ǰ��������
static DecoderType DecoderGetType(void)
{
	audio_decoder = NULL;

    DBG("****** DecoderGetType...\n");
	
	mv_mread_callback_set(SoundRemindFillStreamCallback);

	if(RT_SUCCESS != audio_decoder_initialize((uint8_t*)VMEM_ADDR, DecoderFileHandle, IO_TYPE_MEMORY, MP3_DECODER))
	{
        APP_DBG("mp3����ʽ����Error type: %d\r\n", audio_decoder_get_error_code());
        audio_decoder_close();
        audio_decoder = NULL;
        return UNKOWN_DECODER;
    }

	return MP3_DECODER;
}

//�������ų�ʼ��
static bool DecoderPlayStart(void)
{	
	//���ų�ʼ��
	uint32_t DecoderType;
	APP_DBG("DoPlayer...\n");

    DecoderType = DecoderGetType();
	if(DecoderType == UNKOWN_DECODER)
	{	
		return FALSE;
	}
	
	APP_DBG("decoder size = %d\n", audio_decoder->decoder_size);
	DBG("Fs = %d, Chl = %d\n", audio_decoder->song_info->sampling_rate, audio_decoder->song_info->num_channels);

	NVIC_EnableIRQ(DECODER_IRQn);
	
    PcmTxInitWithDefaultPcmFifoSize((PCM_DATA_MODE)audio_decoder->song_info->pcm_data_mode);
    PcmFifoInitialize(20*1024, 8*1024, 0, 0);
    DacAdcSampleRateSet(audio_decoder->song_info->sampling_rate, USB_MODE);
    //DacAdcSampleRateSet(44100, USB_MODE);
    DacVolumeSet(0xED8, 0xED8);//max volume 
    CodecDacMuteSet(FALSE, FALSE);

	return TRUE;
}

//��ʼ����
//FileHandle: �ļ�ָ��
//FileType:   IO_TYPE_FILE--����U��/SD���е��ļ���IO_TYPE_MEMORY--������ʾ��
bool DecoderTaskPlayStart(void* FileHandle, uint8_t FileType, uint32_t StartTime)
{	
	DBG("DecoderTaskPlayStart()\n");
	
	if(DecoderTaskState != DECODER_STATE_STOP)
	{
		return FALSE;
	}

    DBG("DecoderTaskPlayStart...OK!\n");

	DecoderFileHandle = FileHandle;

    if(DecoderPlayStart())
	{
        DecoderTaskState = DECODER_STATE_PLAY;
	}
    else
    {
        SoundRemindControl.IsRunning = 0;
        return FALSE;
    }
	
	return TRUE;
}

static bool SongPlayDo(void)
{
    if(RT_NO == audio_decoder_check_pcm_transfer_done())
	{
		return TRUE;
	}

    if(SoundRemindFillStream() == -1 && audio_decoder_get_error_code() == -127)
	{
        DBG("++++ SoundRemindFillStream over ....\n");
		return FALSE; // �����Ž���
	}

    if(RT_SUCCESS == audio_decoder_decode())
    {
        if(RT_YES == is_audio_decoder_with_hardware())//wav����ҪӲ������
		{	
			audio_decoder_wait_for_hardware_decoder_done();
		}
//		APP_DBG("XR: %d ms, %d\n", PastTimeGet(&DecoderTimer), audio_decoder->song_info->pcm_data_length);

        audio_decoder_clear_pcm_transfer_done();
        audio_decoder_start_pcm_transfer();
    }
    else
	{
		APP_DBG("audio_decoder_get_error_code(%d)\n", audio_decoder_get_error_code());
	}

	return TRUE;
}

void DecoderTaskInit(void)
{
    DecoderTaskState = DECODER_STATE_STOP;

	memset(&SoundRemindControl, 0x00, sizeof(SoundRemindControl));
	memset(&SoundRemindFile, 0x00, sizeof(SoundRemindFile));

    TimeOutSet(&SoundRemindTimer, 500);
}

void DecoderTaskEntrance(void)
{
    if(!SoundRemindControl.IsRunning)
    {
        return;
    }

    if(!IsTimeOut(&SoundRemindTimer))
    {
        return;
    }

    TimeOutSet(&SoundRemindTimer, 1);

	//����״̬�½���
	if(DecoderTaskState == DECODER_STATE_PLAY)
	{
		if(!SongPlayDo())
		{
            mv_mread_callback_unset();
            
			DecoderTaskState = DECODER_STATE_STOP;

            SoundRemindControl.IsRunning = 0;
		}
	}
}

