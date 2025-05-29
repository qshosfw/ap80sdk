/**
 *******************************************************************************
 * @file    sys_vol.c
 * @author  Richard
 * @version V1.0.0
 * @date    07-01-2014
 * @brief   sys vol set here
 * @maintainer: Sam
 *******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */
#include "type.h"
#include "app_config.h"
#include "sys_vol.h"
#include "dac.h"
#include "sound_remind.h"
#include "task_decoder.h"
#include "mixer.h"
#include "device_audio.h"
#include "audio_adc.h"
#include "clk.h"
#include "audio_path.h"
#include "eq.h"
#include "i2s.h"
#include "rst.h"
#include "gpio.h"
#include "bt_control_api.h"
#include "sys_app.h"
#include "bt_app_func.h"
#include "eq_params.h"
#include "pcm_fifo.h"
#include "pcm_transfer.h"
#include "beep.h"
#include "audio_decoder.h"
#include "eq_params.h"
#include "string.h"

extern uint8_t EchoDelayStep;
extern uint8_t EchoDepthStep;

extern uint32_t gCurDecoderType;

uint8_t gAudioAnaChannel = AUDIO_CH_NONE;
////fadein/out��ؿ��Ʊ���
bool IsFadeOutEn = 0;
bool IsFadeOutDone = 0;
bool IsFadeInEn = 0;
static int32_t VolTemp1 = 4095 * 8;

#ifdef FUNC_SPEC_TRUM_EN
uint8_t AudioSpecTrumStep = 0;
#endif

#ifdef FUNC_AUDIO_DRC_EFFECT_EN
bool gDRCEffectInited = FALSE;
#endif

#ifdef FUNC_AUDIO_VB_EFFECT_EN
bool gVbEffectInited = FALSE;
#endif

#ifdef FUNC_AUDIO_3D_EFFECT_EN
bool	g3dEffectInited = FALSE;
#endif

#if defined(FUNC_AUDIO_VB_EFFECT_EN) || defined (FUNC_AUDIO_3D_EFFECT_EN) || defined (FUNC_AUDIO_DRC_EFFECT_EN) || (EQ_STYLE_MODE == 2) || (EQ_STYLE_MODE == 3)
static int32_t *AudioBasePcmAddr1 = (int32_t *)(VMEM_ADDR+ 28*1024);
static uint16_t AudioBaseLength1;
static uint8_t  AudioVb3dFirstData = TRUE;
static uint8_t  AudioVb3dDone = TRUE;
#endif


#if (EQ_STYLE_MODE == 2) || (EQ_STYLE_MODE == 3)
SwEqContext gSwEq;
SwEqWorkBuffer gSwEqWorkBuffer;
SwEqContext * p_gSwEq = &gSwEq;
#else
SwEqContext * p_gSwEq = NULL;
#endif

uint8_t			mixer_pcm_format = MIXER_FORMAT_STERO;

//HF������
#ifdef FUNC_BT_HF_EN
const uint16_t gHfVolArr[MAX_BTHF_VOLUME + 1] =
{
	0,		42,		107,	173,	
	332,	541,	880,	1035,	
	1218,	1433,	1686,	1984,
	2434,	2946,	3500,	4095
};
#endif

//�����������ݱ�ʾ������Ƶͨ·���ֲ��ֵ�Gainֵ
//4095��ʾ0dB,Ϊ0ʱ��ʾMute������������
//С��4095Ϊ�����棬����4095Ϊ������
//��������֮��ļ��㹫ʽΪ "20*log(Vol1/Vol2)"����λdB

//decoder������
const uint16_t gDecVolArr[MAX_VOLUME + 1] =
{
	0,
	26,		31,		36,		42,		49,		58,		67,		78,
	91,		107,	125,	147,	173,	204,	240,	282,
	332,	391,	460,	541,	636,	748,	880,	1035,
	1218,	1433,	1686,	1984,	2434,	2946,	3500,	4096
};

//LINEIN��FM������
const uint16_t gAnaVolArr[MAX_VOLUME + 1] =
{
	0, 
	56,		66,		76,		89,		104,	121,	141,	164,
	192,	223,	260,	303,	354,	412,	480,	560,
	652,	760,	886,	1033,	1203,	1402,	1635,	1905,
	2220,	2587,	3016,	3515,	4096,	4774,	5564,	6484
};

#ifdef FUNC_MIC_EN
//MIC������
const uint16_t gMicVolArr[MAX_MICIN_VOLUME + 1] =
{
	0,
	609,	724,	861,	1024,	1218,	1448,	1722,	2048,
	2435,	2896,	3444,	4096,	4871,	5793,	6889,	8192
};

#endif

//����ϵͳ������
void SetSysVol(void)
{
	uint16_t LeftVol;
	uint16_t RightVol;
	
	if(gSys.Volume > MAX_VOLUME)
	{
		gSys.Volume = MAX_VOLUME;
	}
	
	if((gSys.CurModuleID == MODULE_ID_BLUETOOTH) && (gSys.Volume >= MAX_VOLUME))
	{
		if(IsAvrcpConnected())
		{
			BTPlayCtrlVolUp(); 			// �������������󣬿ɼ��������ֻ�������
		}
	}
	
	if((gSys.CurModuleID == MODULE_ID_USB_AUDIO)
	|| (gSys.CurModuleID == MODULE_ID_USB_READER)
	|| (gSys.CurModuleID == MODULE_ID_USB_AUDIO_READER))
	{
		LeftVol = AudioLeftVolume * gDecVolArr[gSys.Volume] / AUDIO_MAX_VOLUME;
		RightVol = AudioRightVolume * gDecVolArr[gSys.Volume] / AUDIO_MAX_VOLUME;
		MixerConfigVolume(MIXER_SOURCE_USB, LeftVol, RightVol);  
	}
	else
	{
		MixerConfigVolume(MIXER_SOURCE_DEC, gDecVolArr[gSys.Volume], gDecVolArr[gSys.Volume]);
	}
	
	MixerConfigVolume(MIXER_SOURCE_ANA_MONO, gAnaVolArr[gSys.Volume], gAnaVolArr[gSys.Volume]);
	MixerConfigVolume(MIXER_SOURCE_ANA_STERO, gAnaVolArr[gSys.Volume], gAnaVolArr[gSys.Volume]); 

#ifdef FUNC_MIC_EN	
	if(gSys.MicVolume > MAX_MICIN_VOLUME)
	{
		gSys.MicVolume = MAX_MICIN_VOLUME;
	}
	MixerConfigVolume(MIXER_SOURCE_MIC, gMicVolArr[gSys.MicVolume], gMicVolArr[gSys.MicVolume]);
#endif
}

// ����HFģʽ����
#ifdef FUNC_BT_HF_EN
void SetBtHfVol(uint8_t vol)
{
	if(vol > MAX_BTHF_VOLUME)
	{
		vol = MAX_BTHF_VOLUME;
	}
	APP_DBG("[BT] HF Vol = %d\n", vol);
	gSys.HfVolume = vol;
    
#ifdef FUNC_BT_HF_UART
    DacVolumeSet(gHfVolArr[gSys.HfVolume], gHfVolArr[gSys.HfVolume]);
#else
	PhubAdcI2sInGainSet(gHfVolArr[gSys.HfVolume], gHfVolArr[gSys.HfVolume]);
#endif
}

uint8_t GetBtHfVol(void)
{
	return gSys.HfVolume;
}
#endif


//����I2S�����������
void I2sPortSet(void)
{
    GpioI2sIoConfig(I2SIN_I2S_IO_PORT);     
#ifdef FUNC_I2S_MASTER_EN
    GpioMclkIoConfig(I2SIN_MCLK_IO_PORT);   
#else
    GpioMclkIoConfig(I2SIN_MCLK_IO_PORT+2); 
#endif
}


//����I2S����I2S��DACͬʱ���ͨ��
void AudioI2sOutSetChannel(uint32_t SampleRate, bool IsAndDacOut)
{
	static uint32_t PreSampleRate = 0;
	static uint32_t PreIsAndDacOut = FALSE; //��һ��ǿ��Ϊ0,���ǻ����SampleRate��ֵһ��
	if((PreSampleRate == SampleRate) && (PreIsAndDacOut == IsAndDacOut))
	{
		return;
	}
	
    PcmFifoClear();
    PhubPathClose(ALL_PATH_CLOSE);   
	RstPhubModule();//reset ���ͨ·�Ĵ���ֵ������pop����phub�Ḵλ�Ĵ�������Ҫ������    
    RstMclk();      //reset ���ͨ·�Ĵ���ֵ������pop��
    DacAdcSampleRateSet(SampleRate, USB_MODE);
    I2sSampleRateSet(SampleRate, USB_MODE);
#ifdef FUNC_I2S_MASTER_EN
    I2sSetMasterMode(MDAC_DATA_FROM_DEC, USB_MODE, I2S_FORMAT_I2S);
#else
    I2sSetSlaveMode(MDAC_DATA_FROM_DEC, USB_MODE, I2S_FORMAT_I2S);
#endif
    WaitMs(50);
    if(IsAndDacOut)//i2s��dacͬʱ���
    {
        PhubPathSel(ADCIN2PMEM_PCMFIFO2DACOUTI2SOUT);
    }
    else           //ֻ��I2S���
    {
        PhubPathSel(ADCIN2PMEM_PCMFIFOBEEP2_IISOUT);
    } 
	
	PreSampleRate = SampleRate;
	PreIsAndDacOut = IsAndDacOut;
}

//����STA���ͨ��
void AudioStaOutSetChannel(uint32_t SampleRate, bool IsAndDacOut)
{
	static uint32_t PreSampleRate = 0;
	if(PreSampleRate == SampleRate)
	{
		return;
	}
	
    PcmFifoClear();
    PhubPathClose(ALL_PATH_CLOSE);
	RstPhubModule();//reset ���ͨ·�Ĵ���ֵ������pop����phub�Ḵλ�Ĵ�������Ҫ������
    RstMclk();      //reset ���ͨ·�Ĵ���ֵ������pop��
    if(SampleRate < 32000) 
    {
        DacSoftMuteSet(TRUE, TRUE);
        AdcClose();        
        DacConfig(MDAC_DATA_FROM_DEC, NORMAL_MODE);
        CodecAdcAnaInit();
        CodecAdcFmGainConfig(18, 18);  
        CodecAdcChannelSel(ADC_CH_MDAC_LOOPBACK_L | ADC_CH_MDAC_LOOPBACK_R);
#ifdef FUNC_I2S_MASTER_EN
        I2sSetMasterMode(I2S_DATA_FROM_ADC, NORMAL_MODE, I2S_FORMAT_I2S);
#else
        I2sSetSlaveMode(I2S_DATA_FROM_ADC, NORMAL_MODE, I2S_FORMAT_I2S);
#endif
        I2sClassdSTASampleRateSet(SampleRate, NORMAL_MODE, 1);
        AdcToI2s(NORMAL_MODE);
        DacSoftMuteSet(FALSE, FALSE);
        WaitMs(50); 
        PhubPathSel(ADCIN2PMEM_PCMFIFO2DACOUT);
    }
    else
    {
        DacAdcSampleRateSet(SampleRate, NORMAL_MODE);
        AdcClose();
#ifdef FUNC_I2S_MASTER_EN
        I2sClassdMasterModeSet(I2S_DATA_FROM_DEC, I2S_FORMAT_I2S);
#else
        I2sClassdSlaveModeSet(I2S_DATA_FROM_DEC, I2S_FORMAT_I2S);
#endif
        I2sClassdSTASampleRateSet(SampleRate, NORMAL_MODE, 0); 
        AdcConfig(NORMAL_MODE);
        WaitMs(50); 
        if(IsAndDacOut)
        {
            PhubPathSel(ADCIN2PMEM_PCMFIFO2DACOUTI2SOUT);
        }
        else
        {
            PhubPathSel(ADCIN2PMEM_PCMFIFOBEEP2_IISOUT);
        }          
    }
	
	PreSampleRate = SampleRate;
}
//DAC�������
void AudioDacOutSetChannel(void)
{
    DacSoftMuteSet(TRUE, TRUE);
    PcmFifoClear();
    PhubPathClose(ALL_PATH_CLOSE);
	RstPhubModule();//reset ���ͨ·�Ĵ���ֵ������pop����phub�Ḵλ�Ĵ�������Ҫ������
	RstMclk();//reset ���ͨ·�Ĵ���ֵ������pop��
	PcmTxSetPcmDataMode(DATA_MODE_STEREO_LR);
	CodecDacChannelSel(DAC_CH_DECD_L | DAC_CH_DECD_R);
	DacSoftMuteSet(TRUE, TRUE);	
	DacConfig(MDAC_DATA_FROM_DEC, USB_MODE);
	DacAdcSampleRateSet(44100, USB_MODE);	 //���ֲ���
	WaitMs(100); 

	CodecDacInit(FALSE);            	//dac init
	
	PhubPathSel(ADCIN2PMEM_PCMFIFO2DACOUT);
	
	DacSoftMuteSet(FALSE, FALSE);
}

//ģ��ͨ·��ʼ��
void AudioAnaInit(void)
{
	//MIC��ʼ��
	CodecMicBiasEn(1);
	
	//FM�����ʼ����C12[ADC_CH_FM2_R]	
	GpioClrRegBits(GPIO_C_PU, GPIOC12);		
	GpioSetRegBits(GPIO_C_PD, GPIOC12);
		
	GpioClrRegBits(GPIO_C_PU, GPIOC11);//C11[ADC_CH_FM1_R]		
	GpioSetRegBits(GPIO_C_PD, GPIOC11);
	
	//ADC��ʼ��
	CodecAdcAnaInit();					//adc init
#if (!defined (OUTPUT_CHANNEL_CLASSD)) && (!defined (OUTPUT_CHANNEL_DAC_CLASSD))
	AdcToPmem(USB_MODE, ADC_FIFO_ADDR - PMEM_ADDR, ADC_FIFO_LEN);
#else 
    AdcToPmem(NORMAL_MODE, ADC_FIFO_ADDR - PMEM_ADDR, ADC_FIFO_LEN);
#endif
	AdcPmemWriteEn();
    
#ifdef FUNC_I2SIN_EN
    I2sWritePmemDis();
    I2sInToPmemConfig(I2SIN_FIFO_ADDR - PMEM_ADDR, I2SIN_FIFO_LEN);
#endif
}

//�������ͨ·��ʼ��
void AudioOutputInit(void)
{
#ifdef OUTPUT_CHANNEL_DAC
    AudioDacOutSetChannel();
    DacMonoConfig(CFG_DAC_MODE, FALSE);
#elif defined (OUTPUT_CHANNEL_I2S)
    I2sPortSet();
    AudioI2sOutSetChannel(44100, 0);

#elif defined (OUTPUT_CHANNEL_DAC_I2S)
    I2sPortSet();
    AudioI2sOutSetChannel(44100, 1);
    DacMonoConfig(CFG_DAC_MODE, FALSE);
#elif defined (OUTPUT_CHANNEL_CLASSD)
    I2sPortSet();
    AdcDacClkModeSet(NORMAL_MODE);
    AudioStaOutSetChannel(44100, 0);
#else
    I2sPortSet();
    AdcDacClkModeSet(NORMAL_MODE);
    AudioStaOutSetChannel(44100, 1);
    DacMonoConfig(CFG_DAC_MODE, FALSE);
#endif    
}

//����ͨ·����������
void AudioSampleRateSet(uint32_t CurSamplerate)
{    
#ifdef OUTPUT_CHANNEL_DAC
    DacAdcSampleRateSet(CurSamplerate, USB_MODE);
    
#elif defined (OUTPUT_CHANNEL_I2S)
    I2sPortSet();
    I2sSampleRateSet(CurSamplerate, USB_MODE);

#elif defined (OUTPUT_CHANNEL_DAC_I2S)
    I2sPortSet();
    I2sSampleRateSet(CurSamplerate, USB_MODE);
    DacAdcSampleRateSet(CurSamplerate, USB_MODE);

#elif defined (OUTPUT_CHANNEL_CLASSD)
    I2sPortSet();
    AudioStaOutSetChannel(CurSamplerate, 0);
    
#else
    I2sPortSet();
    AudioStaOutSetChannel(CurSamplerate, 1);
#endif    
}

bool IsFillMemData = FALSE;
extern void PcmMemFillData(void);

//����ģ��ͨ·
void AudioAnaSetChannel(uint8_t Channel)
{
#ifdef FUNC_MIC_EN
	bool MicEnable = ((gAudioAnaChannel == AUDIO_CH_MIC) 
					|| (gAudioAnaChannel == AUDIO_CH_MIC_LINEIN) 
					|| (gAudioAnaChannel == AUDIO_CH_MIC_FM)); 
#endif
    if(gAudioAnaChannel == Channel)
	{
        return;
	}
	gAudioAnaChannel = Channel;
    PcmFifoClear();
    
	switch(Channel)
	{
		case AUDIO_CH_NONE:			//������
			MixerDisable(MIXER_SOURCE_MIC);
			MixerDisable(MIXER_SOURCE_ANA_MONO);
			MixerDisable(MIXER_SOURCE_ANA_STERO);
			AdcPmemWriteDis();
			break;

		case AUDIO_CH_MIC:			//������MIC
		#ifdef FUNC_MIC_EN
			MixerDisable(MIXER_SOURCE_ANA_MONO);
			MixerDisable(MIXER_SOURCE_ANA_STERO);
			AdcVolumeSet(0, 0);
			AdcPmemWriteDis();
			WaitMs(20);

			CodecAdcChannelSel(ADC_CH_MIC_L);
			CodecAdcMicGainConfig(MICIN_ANA_GAIN_VAL, MICIN_ANA_GAIN_VAL);
			if(!MicEnable)
			{
				MixerConfigFormat(MIXER_SOURCE_MIC, 44100, MIXER_FORMAT_MONO);
				mixer_pcm_format = MIXER_FORMAT_MONO;
				SetSysVol();
				MixerEnable(MIXER_SOURCE_MIC);
			}
			
			WaitMs(50);
			IsFillMemData = TRUE;
			PcmMemFillData();
			IsFillMemData = FALSE;
			AdcVolumeSet(ADC_DIGITAL_VOL, ADC_DIGITAL_VOL);
			AdcPmemWriteEn();
		#endif
			break;
			
		case AUDIO_CH_LINEIN:		//˫����LINEIN
		#ifdef FUNC_LINEIN_EN
			MixerDisable(MIXER_SOURCE_MIC);
			MixerDisable(MIXER_SOURCE_ANA_MONO);
			MixerDisable(MIXER_SOURCE_ANA_STERO);
			AdcVolumeSet(0, 0);
			AdcPmemWriteDis();
			WaitMs(20);

			CodecAdcLineInGainConfig(LINEIN_ANA_GAIN_VAL, LINEIN_ANA_GAIN_VAL);
			CodecAdcChannelSel(ADC_CH_LINE_L | ADC_CH_LINE_R);

			MixerConfigFormat(MIXER_SOURCE_ANA_STERO, 44100, MIXER_FORMAT_STERO);
			mixer_pcm_format = MIXER_FORMAT_STERO;
			SetSysVol();
			WaitMs(50);
			IsFillMemData = TRUE;
			MixerEnable(MIXER_SOURCE_ANA_STERO);
			PcmMemFillData();
			IsFillMemData = FALSE;
			AdcVolumeSet(ADC_DIGITAL_VOL, ADC_DIGITAL_VOL);
			AdcPmemWriteEn();
		#endif
			break;
        
        case AUDIO_CH_I2SIN:        //I2S IN
			MixerDisable(MIXER_SOURCE_MIC);
			MixerDisable(MIXER_SOURCE_ANA_MONO);
            WaitMs(20);
        
            CodecAdcChannelSel(ADC_CH_NONE); 
        
#ifdef OUTPUT_CHANNEL_DAC            
            I2sSampleRateSet(44100, USB_MODE);
    #ifdef FUNC_I2S_MASTER_EN
            I2sSetMasterMode(MDAC_DATA_FROM_DEC, USB_MODE, I2S_FORMAT_I2S);
    #else
            I2sSetSlaveMode(MDAC_DATA_FROM_DEC, USB_MODE, I2S_FORMAT_I2S); 
    #endif
#endif       
            MixerConfigFormat(MIXER_SOURCE_ANA_STERO, 44100, MIXER_FORMAT_STERO);
            mixer_pcm_format = MIXER_FORMAT_STERO;
			SetSysVol();
			WaitMs(50);
            IsFillMemData = TRUE;
			MixerEnable(MIXER_SOURCE_ANA_STERO);
			PcmMemFillData();
			IsFillMemData = FALSE;
			MixerEnable(MIXER_SOURCE_ANA_STERO); 
            I2sWritePmemEn();
            break;

		case AUDIO_CH_FM:			//˫����FM
		#ifdef FUNC_RADIO_EN
			MixerDisable(MIXER_SOURCE_MIC);
			MixerDisable(MIXER_SOURCE_ANA_MONO);
			AdcVolumeSet(0, 0);
			AdcPmemWriteDis();
			WaitMs(20);

			CodecAdcFmGainConfig(FMIN_ANA_GAIN_VAL, FMIN_ANA_GAIN_VAL);
			CodecAdcChannelSel(ADC_CH_FM2_L | ADC_CH_FM2_R);
			
			MixerConfigFormat(MIXER_SOURCE_ANA_STERO, 44100, MIXER_FORMAT_STERO);
			mixer_pcm_format = MIXER_FORMAT_STERO;
			SetSysVol();
			WaitMs(50);
			IsFillMemData = TRUE;
			MixerEnable(MIXER_SOURCE_ANA_STERO);
			PcmMemFillData();
			IsFillMemData = FALSE;
			AdcVolumeSet(ADC_DIGITAL_VOL, ADC_DIGITAL_VOL);
			AdcPmemWriteEn();
		#endif
			break;

		case AUDIO_CH_MIC_LINEIN:	//MIC + ������LINEIN
		#if (defined(FUNC_MIC_EN) && defined(FUNC_LINEIN_EN))
			MixerDisable(MIXER_SOURCE_ANA_STERO);
            AdcVolumeSet(0, 0);
			AdcPmemWriteDis();
			WaitMs(20);

			CodecAdcMicGainConfig(MICIN_ANA_GAIN_VAL, MICIN_ANA_GAIN_VAL);
			CodecAdcLineInGainConfig(LINEIN_ANA_GAIN_VAL, LINEIN_ANA_GAIN_VAL);
			CodecAdcChannelSel(ADC_CH_MIC_L | ADC_CH_LINE_R);

			if(!MicEnable)
        	{
				MixerConfigFormat(MIXER_SOURCE_MIC, 44100, MIXER_FORMAT_MONO);
				mixer_pcm_format = MIXER_FORMAT_MONO;
				SetSysVol();
				MixerEnable(MIXER_SOURCE_MIC);
        	}

			MixerConfigFormat(MIXER_SOURCE_ANA_MONO, 44100, MIXER_FORMAT_MONO);
			mixer_pcm_format = MIXER_FORMAT_MONO;
			SetSysVol();
			
			WaitMs(50);
			IsFillMemData = TRUE;
			MixerEnable(MIXER_SOURCE_ANA_MONO);
			PcmMemFillData();
			IsFillMemData = FALSE;
			AdcVolumeSet(ADC_DIGITAL_VOL, ADC_DIGITAL_VOL);
			AdcPmemWriteEn();
		#endif
			break;		
			
		case AUDIO_CH_MIC_FM:		//MIC + ������FM
		#if (defined(FUNC_MIC_EN) && defined(FUNC_RADIO_EN))
			MixerDisable(MIXER_SOURCE_ANA_STERO);
            AdcVolumeSet(0, 0);
			AdcPmemWriteDis();
			WaitMs(20);

			CodecAdcFmGainConfig(FMIN_ANA_GAIN_VAL, FMIN_ANA_GAIN_VAL);
			CodecAdcMicGainConfig(MICIN_ANA_GAIN_VAL, MICIN_ANA_GAIN_VAL);
			CodecAdcChannelSel(ADC_CH_MIC_L | ADC_CH_FM2_R);

			if(!MicEnable)
        	{
				MixerConfigFormat(MIXER_SOURCE_MIC, 44100, MIXER_FORMAT_MONO);
				mixer_pcm_format = MIXER_FORMAT_MONO;
				SetSysVol();
				MixerEnable(MIXER_SOURCE_MIC);
        	}

			MixerConfigFormat(MIXER_SOURCE_ANA_MONO, 44100, MIXER_FORMAT_MONO);
			mixer_pcm_format = MIXER_FORMAT_MONO;
			SetSysVol();
			WaitMs(50);
			IsFillMemData = TRUE;
			MixerEnable(MIXER_SOURCE_ANA_MONO);
			PcmMemFillData();
			IsFillMemData = FALSE;
			AdcVolumeSet(ADC_DIGITAL_VOL, ADC_DIGITAL_VOL);
			AdcPmemWriteEn();
		#endif
			break;
		
		default:
			break;
	}	
	
#ifdef FUNC_AUDIO_DRC_EFFECT_EN
	if(gSys.AudioDRCEffectFlag)
	{
		gDRCEffectInited = FALSE;
		InitVb3dPcmPara();
	}
#endif	
	
#ifdef FUNC_AUDIO_3D_EFFECT_EN
	if(gSys.Audio3dEffectFlag)
	{
		g3dEffectInited = FALSE;
		InitVb3dPcmPara();
	}
#endif

#ifdef FUNC_AUDIO_VB_EFFECT_EN
    if(gSys.AudioVbEffectFlag)
    {
        gVbEffectInited = FALSE;
        InitVb3dPcmPara();
    }    
#endif
    
#ifdef FUNC_MIC_ALC_EN
	if((gAudioAnaChannel == AUDIO_CH_MIC) 
	|| (gAudioAnaChannel == AUDIO_CH_MIC_LINEIN) 
	|| (gAudioAnaChannel == AUDIO_CH_MIC_FM))
	{
		AdcAlcConfig(ALC_LEFT_ONLY, 0x0D, 0x04, 0x03);	
		AdcAlcGainSet(ALC_MAX_GAIN, ALC_MIN_GAIN);
		AdcAlcNoiseGateSet(ALC_MUTE_OUTPUT, ALC_NOISE_DB, ALC_NOISE_EN);         //ALC_MUTE_OUTPUT   ALC_HOLD_PGA_GAIN      			
	}
	else
	{
		AdcAlcSel(ALC_DISABLE);	
	}
#endif
}


//����������ȡ��
void AudioPlayerMute(bool MuteEnFlag)
{
	gSys.MuteFlag = MuteEnFlag;

	// ���������㾲�����ýӿ�
	if(gSys.MuteFlag)
	{
		DecoderTaskPause();
		SoundRemind(SOUND_MUTE_ON);  
		DacSoftMuteSet(TRUE, TRUE);
		DecoderTaskResume();
	}
	else
	{
		DecoderTaskPause();
		DacSoftMuteSet(FALSE, FALSE);
		SoundRemind(SOUND_MUTE_OFF);
		DecoderTaskResume();
	}

	// ˢ��������ʾ
	APP_DBG("Mute:%d\n", gSys.MuteFlag);
}


//��ģʽ�µ�ͨ����Ϣ����
void CommonMsgProccess(uint16_t Msg)
{
	switch(Msg)
	{	
		case MSG_VOL_UP:	
			if(gSys.Volume < MAX_VOLUME)
			{
				gSys.Volume++;
			}
			
			if(gSys.MuteFlag)
			{
				gSys.MuteFlag = 0;
				AudioPlayerMute(FALSE);
			}
			APP_DBG("Vol+(%d)\n", gSys.Volume);
			SetSysVol();
			break;
				
		case MSG_VOL_DW:
			if(gSys.Volume > 0)
			{
				gSys.Volume--;
			}
            
            if(gSys.MuteFlag)
			{
				gSys.MuteFlag = 0;
				AudioPlayerMute(FALSE);
			}            
			APP_DBG("Vol-(%d)\n", gSys.Volume);
			SetSysVol();
			break;
		
		case MSG_EQ:
			AudioPlayerSwitchEQ();
#ifdef FUNC_TREB_BASS_EN
			gSys.EqStatus = 1;
#endif
			break;
		
		case MSG_MUTE:
			AudioPlayerMute(!gSys.MuteFlag);
			break;
		
#ifdef FUNC_AUDIO_DRC_EFFECT_EN
		case MSG_DRC:
			gSys.AudioDRCEffectFlag = !gSys.AudioDRCEffectFlag;
			DBG("DRC: %d\n", gSys.AudioDRCEffectFlag);
			if(gSys.AudioDRCEffectFlag)
			{
                gDRCEffectInited = FALSE;
				InitVb3dPcmPara();
			}
			break;
#endif
			
#ifdef FUNC_AUDIO_3D_EFFECT_EN
		case MSG_3D:
			gSys.Audio3dEffectFlag = !gSys.Audio3dEffectFlag;
			DBG("3D: %d\n", gSys.Audio3dEffectFlag);
			if(gSys.Audio3dEffectFlag)
			{
                g3dEffectInited = FALSE;
				InitVb3dPcmPara();
			}
			break;
#endif

#ifdef FUNC_AUDIO_VB_EFFECT_EN
        case MSG_VB:
            gSys.AudioVbEffectFlag = !gSys.AudioVbEffectFlag;
            APP_DBG("VB: %d\n",gSys.AudioVbEffectFlag);
            if(gSys.AudioVbEffectFlag)
            {
                gVbEffectInited = FALSE;
                InitVb3dPcmPara();
            }
            break;
#endif
			
#ifdef FUNC_SOUND_REMIND
		case MSG_REMIND:
			if(gSys.SoundRemindOn)
			{
				DBG("SoundRemindOn close!\n");
				SoundRemind(SOUND_SPEECH_CL);  
				gSys.SoundRemindOn = 0;
			}
			else
			{
				DBG("SoundRemindOn open!\n");
				gSys.SoundRemindOn = 1;
				SoundRemind(SOUND_SPEECH_OP);  
			}
			AudioSysInfoSetBreakPoint();
			break;

		case MSG_LANG:
			gSys.LanguageMode = (gSys.LanguageMode == LANG_ZH) ? LANG_EN : LANG_ZH;
			DBG("LANG: %d\n", gSys.LanguageMode);
			SoundRemind(SOUND_LANG_MODE);  
			AudioSysInfoSetBreakPoint();
			break;
#endif

		case MSG_MIC_VOL_UP:
		#ifdef FUNC_MIC_EN
			if(gSys.MicVolume < MAX_MICIN_VOLUME)
			{
				gSys.MicVolume++;
			}
			DBG("MIC_VOL_UP: %d\n", gSys.MicVolume);
			SetSysVol();
		#endif
			break;

		case MSG_MIC_VOL_DW:
		#ifdef FUNC_MIC_EN
			if(gSys.MicVolume > 0)
			{
				gSys.MicVolume--;
			}
			DBG("MIC_VOL_DW: %d\n", gSys.MicVolume);
			SetSysVol();
		#endif
			break;

		case MSG_ECHO_DELAY_UP:
		#ifdef FUNC_MIC_ECHO_EN
			if(EchoDelayStep < MAX_ECHO_DELAY_STEP)
			{
				EchoDelayStep++;
			}
			DBG("ECHO_DELAY_UP: %d\n", EchoDelayStep);
		#endif
			break;

		case MSG_ECHO_DELAY_DW:
		#ifdef FUNC_MIC_ECHO_EN
			if(EchoDelayStep > 0)
			{
				EchoDelayStep--;
			}
			DBG("ECHO_DELAY_DW: %d\n", EchoDelayStep);
		#endif
			break;

		case MSG_ECHO_DEPTH_UP:
		#ifdef FUNC_MIC_ECHO_EN
			if(EchoDepthStep < MAX_ECHO_DEPTH_STEP)
			{
				EchoDepthStep++;
			}			
			DBG("ECHO_DEPTH_UP: %d\n", EchoDepthStep);
		#endif
			break;

		case MSG_ECHO_DEPTH_DW:
		#ifdef FUNC_MIC_ECHO_EN
			if(EchoDepthStep > 0)
			{
				EchoDepthStep--;
			}
			DBG("ECHO_DEPTH_DW: %d\n", EchoDepthStep);
		#endif
			break;

		case MSG_TREB_UP:
		#ifdef FUNC_TREB_BASS_EN
			if(gSys.TrebVal < MAX_TREB_VAL)
			{
				gSys.TrebVal++;
				TrebBassSet(gSys.TrebVal, gSys.BassVal);
			}
			gSys.EqStatus = 0;
			DBG("TREB_UP: %d\n", gSys.TrebVal);
		#endif
			break;

		case MSG_TREB_DW:
		#ifdef FUNC_TREB_BASS_EN
			if(gSys.TrebVal > 0)
			{
				gSys.TrebVal--;
				TrebBassSet(gSys.TrebVal, gSys.BassVal);
			}
			gSys.EqStatus = 0;
			DBG("TREB_DW: %d\n", gSys.TrebVal);
		#endif
			break;

		case MSG_BASS_UP:
		#ifdef FUNC_TREB_BASS_EN
			if(gSys.BassVal < MAX_BASS_VAL)
			{
				gSys.BassVal++;
				TrebBassSet(gSys.TrebVal, gSys.BassVal);
			}
			gSys.EqStatus = 0;
			DBG("BASS_UP: %d\n", gSys.BassVal);
		#endif
			break;

		case MSG_BASS_DW:
		#ifdef FUNC_TREB_BASS_EN
			if(gSys.BassVal > 0)
			{
				gSys.BassVal--;
				TrebBassSet(gSys.TrebVal, gSys.BassVal);
			}
			gSys.EqStatus = 0;
			DBG("BASS_DW: %d\n", gSys.BassVal);
		#endif
			break;
				
#ifdef FUNC_AUDIO_MENU_EN
		case MSG_MENU:
			switch(gSys.AudioSetMode)
			{
				case AUDIO_SET_MODE_MAIN_VOL:	//����������
					DBG("MAIN_VOL\n");
					break;
				
				case AUDIO_SET_MODE_MIC_VOL:	//MIC��������
					DBG("MIC_VOL\n");
					SoundRemind(SOUND_MIC_VOL);  
					break;

				case AUDIO_SET_MODE_ECHO_DEPTH:	//MIC�������
					DBG("ECHO_DEPTH\n");
					SoundRemind(SOUND_ECHO_REGULATE); 
					break;

				case AUDIO_SET_MODE_ECHO_DELAY:	//MIC��ʱ����
					DBG("ECHO_DELAY\n");
					SoundRemind(SOUND_DELAY_REGULATE);  
					break;

				case AUDIO_SET_MODE_TREB:		//��������
					DBG("TREB\n");
					SoundRemind(SOUND_TREB_REGULATE);  
					break;

				case AUDIO_SET_MODE_BASS:		//��������
					DBG("BASS\n");
					SoundRemind(SOUND_BASS_REGULATE);  
					break;
				
				default:
					break;
			}   
			break;
#endif				
		default:
			break;		
	}	
}

#ifdef FUNC_SPEC_TRUM_EN
//Ƶ�ײɼ�
void SpecTrumProc(int16_t* Buffer, uint32_t Len)
{
	uint32_t i;
	int32_t TempSampleData;
	   
	for(i = 0; i < Len; i += 8)
	{
		if((gSys.CurModuleID == MODULE_ID_LINEIN) 
			|| (gSys.CurModuleID == MODULE_ID_RADIO) 
			|| (gSys.CurModuleID == MODULE_ID_RECORDER))
		{
			TempSampleData = (Buffer[i] << 1);
		}
		else
		{
			TempSampleData = Buffer[i];
		}
		if(TempSampleData < 1000)
		{
		       AudioSpecTrumStep = 0;	
		}
		else if(TempSampleData  < 6554)
		{
			AudioSpecTrumStep = 1;	
		}
		else if(TempSampleData < 13108)
		{
			AudioSpecTrumStep = 2;
		}
		else if(TempSampleData < 19622)
		{
			AudioSpecTrumStep = 3;
		}
		else if(TempSampleData < 26216)
		{
			AudioSpecTrumStep = 4;
		}
		else if(TempSampleData < 32770)
		{
			AudioSpecTrumStep = 5;
		}		
 	}		
}
#endif

#ifdef FUNC_AUDIO_DRC_EFFECT_EN
int32_t InitDrc(DRCContext *ct, int32_t num_channels, int32_t sample_rate)
{
	int32_t fc = AUDIO_EFFECT_PARAM_DRC_CROSSOVER_FREQ;
	int32_t threshold[2] = {AUDIO_EFFECT_PARAM_DRC_THRESHHOLD1,AUDIO_EFFECT_PARAM_DRC_THRESHHOLD2};
	int32_t ratio[2] = {AUDIO_EFFECT_PARAM_DRC_RATIO1,AUDIO_EFFECT_PARAM_DRC_RATIO2};
	int32_t attack_tc[2] = {AUDIO_EFFECT_PARAM_DRC_ATTACK_TIME1,AUDIO_EFFECT_PARAM_DRC_ATTACK_TIME2};
	int32_t release_tc[2] = {AUDIO_EFFECT_PARAM_DRC_RELEASE_TIME1,AUDIO_EFFECT_PARAM_DRC_RELEASE_TIME2};

	return init_drc(ct, num_channels, sample_rate, fc, 2, threshold, ratio, attack_tc, release_tc);
}

//DRC��Ч����
void AudioDRCPro(int16_t *PcmDataAddr, int32_t CurNumChannel, int32_t CurSampleRate, int32_t PcmDataLen)
{
	int32_t	pregain = AUDIO_EFFECT_PARAM_DRC_PREGAIN;//�˴�pregain������Ϊ����������
	
	if(gSys.AudioDRCEffectFlag)
	{
		if(!gDRCEffectInited)
		{
			if(DRC_ERROR_OK != InitDrc(&gSys.AudioDRCEffectHandle, CurNumChannel, CurSampleRate))
			{
				gDRCEffectInited = FALSE;
			}
			else
			{
				gDRCEffectInited = TRUE;
			}
		}
		else
		{			
            apply_drc(&gSys.AudioDRCEffectHandle, PcmDataAddr, PcmDataAddr, PcmDataLen, pregain);		
		}
	}
}
#endif

#ifdef FUNC_AUDIO_3D_EFFECT_EN
//3D��Ч����
void Audio3dPro(int16_t *PcmDataAddr, int32_t CurNumChannel, int32_t CurSampleRate, int32_t PcmDataLen)
{
	if(gSys.Audio3dEffectFlag)
	{
		if(!g3dEffectInited)
		{
			if(THREE_D_ERROR_OK != init_3d(&gSys.Audio3dEffectHandle, CurNumChannel, CurSampleRate))
			{
				g3dEffectInited = FALSE;
			}
			else
			{
				g3dEffectInited = TRUE;
			}
		}
		else
		{
			if(CurNumChannel == 2)
            {
                apply_3d(&gSys.Audio3dEffectHandle, PcmDataAddr, PcmDataAddr, PcmDataLen, AUDIO_EFFECT_PARAM_3D_DEPTH, AUDIO_EFFECT_PARAM_3D_POSTGAIN, AUDIO_EFFECT_PARAM_3D_PREGAIN);
            }				
		}
	}
}
#endif

#ifdef FUNC_AUDIO_VB_EFFECT_EN
//VB��Ч����
void AudioVbPro(int16_t *PcmDataAddr, int32_t CurNumChannel, uint32_t CurSamplerate, int32_t PcmDataLen)
{
    int16_t *PcmAddr = (int16_t *)PcmDataAddr;
    
    if(PcmDataLen == 0)
    {
        return;
    }
    
    if(gSys.AudioVbEffectFlag)
    {
        if(!gVbEffectInited)
        {
            init_vb(&gSys.AudioVbEffectHandle, CurNumChannel, CurSamplerate, AUDIO_EFFECT_PARAM_VB_CUTOFF_FREQ);
            gVbEffectInited = TRUE;
        }
        apply_vb(&gSys.AudioVbEffectHandle,PcmAddr,  PcmAddr, PcmDataLen,AUDIO_EFFECT_PARAM_VB_INTENSITY, AUDIO_EFFECT_PARAM_VB_PREGAIN, AUDIO_EFFECT_PARAM_VB_POSTGAIN);
    }    
}
#endif

#if defined(FUNC_AUDIO_VB_EFFECT_EN) || defined(FUNC_AUDIO_3D_EFFECT_EN) || defined(FUNC_AUDIO_DRC_EFFECT_EN) || (EQ_STYLE_MODE == 2) || (EQ_STYLE_MODE == 3)
void InitVb3dPcmPara(void)
{        
    AudioVb3dFirstData = TRUE;
    AudioVb3dDone = TRUE;
}

void DealWithVb3dPcmPara(uint32_t CurChannelNum, uint32_t CurSampleRate)
{
    if((AudioBasePcmAddr1 == NULL) || AudioVb3dDone|| AudioVb3dFirstData)
    {
        return;
    }
	
#ifdef FUNC_AUDIO_VB_EFFECT_EN
	if(gSys.AudioVbEffectFlag)
		AudioVbPro((int16_t *)AudioBasePcmAddr1, CurChannelNum, CurSampleRate, AudioBaseLength1);
#endif
    
#ifdef FUNC_AUDIO_3D_EFFECT_EN
    if(gSys.CurModuleID == MODULE_ID_BLUETOOTH 
#ifdef FUNC_SOUND_REMIND
    && !IsSoundRemindPlaying()
#endif
    )
    {
        Audio3dPro((int16_t *)AudioBasePcmAddr1, CurChannelNum, CurSampleRate, AudioBaseLength1);           
    }
#endif

#if (EQ_STYLE_MODE == 2) || (EQ_STYLE_MODE == 3)
	if(gSys.EqStatus == 1)
		AudioSwEQPro(AudioBasePcmAddr1, CurChannelNum, AudioBaseLength1);
#endif

#ifdef FUNC_AUDIO_DRC_EFFECT_EN
#ifdef FUNC_SOUND_REMIND
    if(!IsSoundRemindPlaying())
#endif
		AudioDRCPro((int16_t *)AudioBasePcmAddr1, CurChannelNum, CurSampleRate, AudioBaseLength1);
#endif

    AudioVb3dDone = TRUE;
}

#define WMA_VB_FIFO_SIZE            2048

uint32_t SaveCurSendLast(int32_t *PcmIn, uint16_t PcmDataLen, uint32_t CurChannelNum)
{
	uint32_t i;
	uint16_t TmpLen;
	uint16_t LoopCount = PcmDataLen;

	if(AudioBasePcmAddr1 == NULL)
	{
		return PcmDataLen;
	}

	if(CurChannelNum == 1)
	{
		LoopCount >>= 1;				// ����������Ϊ˫�������ݵ�һ��
	}

	if(AudioVb3dFirstData)
	{
		memset((void *)AudioBasePcmAddr1, 0, WMA_VB_FIFO_SIZE<<CurChannelNum);
		for(i=0; i<LoopCount; i++)
		{
			AudioBasePcmAddr1[i] = (AudioBasePcmAddr1[i] ^ PcmIn[i]);
			PcmIn[i] = (AudioBasePcmAddr1[i] ^ PcmIn[i]);
			AudioBasePcmAddr1[i] = (AudioBasePcmAddr1[i] ^ PcmIn[i]);
		}
		AudioBaseLength1 = PcmDataLen;
		AudioVb3dFirstData = FALSE;
		AudioVb3dDone = TRUE;
		return PcmDataLen;
	}

	if(CurChannelNum == 1)
	{
		LoopCount = (PcmDataLen > AudioBaseLength1 ? PcmDataLen/2 : AudioBaseLength1/2);
	}
	else
	{
		LoopCount = (PcmDataLen > AudioBaseLength1 ? PcmDataLen : AudioBaseLength1);
	}

	for(i=0; i<LoopCount; i++)
	{
		AudioBasePcmAddr1[i] = (AudioBasePcmAddr1[i] ^ PcmIn[i]);
		PcmIn[i] = (AudioBasePcmAddr1[i] ^ PcmIn[i]);
		AudioBasePcmAddr1[i] = (AudioBasePcmAddr1[i] ^ PcmIn[i]);
	}

	TmpLen = AudioBaseLength1;
	AudioBaseLength1 = PcmDataLen;
	PcmDataLen = TmpLen;

	AudioVb3dDone = FALSE;
	return PcmDataLen;
}
#endif
//

//��������Ԥ���棬Vol <= 4095,����ƥ�䲻ͬģʽ֮��ĵ�ƽ����
void DigitalGainSet(int32_t *PcmIn, uint16_t PcmDataLen, uint32_t CurChannelNum, uint16_t Vol)
{
	uint32_t i;
	int16_t *PcmBuf; 
	uint32_t VolTemp = Vol * 8;
	
	PcmBuf = (int16_t *)PcmIn;
	PcmDataLen = PcmDataLen * CurChannelNum;
	for(i=0; i<PcmDataLen; i++)
	{
		PcmBuf[i] = (PcmBuf[i] * VolTemp) >> 15;
	}
}

void DigitalFadeOut(int32_t *PcmIn, uint16_t PcmSampleLen, uint32_t CurChannelNum)
{
	uint32_t i;
	uint32_t j = 0;
	int16_t *PcmBuf; 
	uint32_t VolTemp;
	
	static int32_t VolStep = 22 * 8;
	
	if(PcmSampleLen > 192)
	{
		j = PcmSampleLen - 192;
	}
	else
	{
		VolStep = (4095 / PcmSampleLen + 1) * 8;
	}
	PcmBuf = (int16_t *)PcmIn;
	if(CurChannelNum == 1)
	{
		if((IsFadeOutEn) && (!IsFadeOutDone))
		{
			for(i=j; i<PcmSampleLen; i++)
			{
				if(VolTemp1 > 0)
				{
					VolTemp1 -= VolStep;
				}
				else
				{
					VolTemp1 = 0;
					IsFadeOutDone = TRUE;
				}
				VolTemp = VolTemp1;
				PcmBuf[i] = ((int32_t)PcmBuf[i] * VolTemp) >> 15;
			}
		}
	}
	else
	{
		if((IsFadeOutEn) && (!IsFadeOutDone))
		{
			for(i=j; i<PcmSampleLen; i++)
			{				
				if(VolTemp1 > 0)
				{
					VolTemp1 -= VolStep;
				}
				else
				{
					VolTemp1 = 0;
					IsFadeOutDone = TRUE;
				}
				VolTemp = VolTemp1;
				PcmBuf[2 * i] = ((int32_t)PcmBuf[2 * i] * VolTemp) >> 15;
				PcmBuf[2 * i + 1] = ((int32_t)PcmBuf[2 * i + 1] * VolTemp) >> 15;
			}
		}
	}
	if(IsFadeOutDone)
	{
		//AudioPlayerSwitchEQ();
		//do something ...
		IsFadeOutDone = FALSE;
		IsFadeOutEn = FALSE;
	}
}

void DigitalFadeIn(int32_t *PcmIn, uint16_t PcmSampleLen, uint32_t CurChannelNum)
{
	uint32_t i;
	uint32_t j = 0;
	int16_t *PcmBuf; 
	uint32_t VolTemp;
	
	static int32_t VolStep = 22 * 8;
	
	if(PcmSampleLen > 192)
	{
		j = 192;
	}
	else
	{
		j = PcmSampleLen;
		VolStep = (4095 / PcmSampleLen + 1) * 8;
	}
	
	PcmBuf = (int16_t *)PcmIn;
	if(CurChannelNum == 1)
	{
		for(i=0; i<j; i++)
		{
			PcmBuf[i] = ((int32_t)PcmBuf[i] * VolTemp) >> 15;
			if(IsFadeInEn)
			{
				if(VolTemp1 < 4095 * 8)
				{
					VolTemp1 += VolStep;
				}
				else
				{
					VolTemp1 = 4095 * 8;
					IsFadeInEn = FALSE;
				}
				VolTemp = VolTemp1;
			}
		}
	}
	else
	{
		for(i=0; i<j; i++)
		{
			PcmBuf[2 * i] = ((int32_t)PcmBuf[2 * i] * VolTemp) >> 15;
			PcmBuf[2 * i + 1] = ((int32_t)PcmBuf[2 * i + 1] * VolTemp) >> 15;
			
			if(IsFadeInEn)
			{
				if(VolTemp1 < 4095 * 8)
				{
					VolTemp1 += VolStep;
				}
				else
				{
					VolTemp1 = 4095 * 8;
					IsFadeInEn = FALSE;
				}
				VolTemp = VolTemp1;
			}
		}
	}
}

/**
 * @brief  set all audio channel EQ
 * @param  NONE
 * @return NONE
 */
void AudioPlayerSwitchEQ(void)
{
	uint32_t SampleRateTmp = DacAdcSampleRateGet();
	
	gSys.Eq = (gSys.Eq + 1) % EQ_STYLE_SUM;
	APP_DBG("\nSwitchEQ: %s\n\n", EqStyleNameGet(gSys.Eq)); 
/*	
	switch(gSys.Eq)
   	{
	   	case EQ_STYLE_FLAT:
			APP_DBG("EQ_STYLE_FLAT\n");
			SoundRemind(SOUND_EQ_FLAT);  
			break;

		case EQ_STYLE_CLASSICAL:
			APP_DBG("EQ_STYLE_CLASSICAL\n");
			SoundRemind(SOUND_EQ_CLASSIC);  
			break;

		case EQ_STYLE_POP:
			APP_DBG("EQ_STYLE_POP\n");
			SoundRemind(SOUND_EQ_POP);  
			break;

		case EQ_STYLE_ROCK:
			APP_DBG("EQ_STYLE_ROCK\n");
			SoundRemind(SOUND_EQ_ROCK);  
			break;

		case EQ_STYLE_JAZZ:
			APP_DBG("EQ_STYLE_JAZZ\n");
			SoundRemind(SOUND_EQ_JAZZ);  
			break;

		default:
			break;
   	}
*/
	if(SampleRateTmp == 0)
	{
		SampleRateTmp = 44100;
	}

    EqStyleSelect(p_gSwEq, SampleRateTmp, gSys.Eq);
}

/////////////////////////����Ϊtreb/bass��غ���//////////////////////////////////////////////////////////////////////////////////////////
EqFilterParams BassTreb_0db[2] =
{
	{ 0x0001, 0x0082, 0x02cc, 0x0400 }, // Parameters for filter 0
	{ 0x0002, 0x1388, 0x02cc, 0x0400 }, // Parameters for filter 4
};

const uint16_t TrebBassTab[] =
{
	0x02ff,		//-10db
	0x032d,		//-8db
	0x035d,		//-6db
	0x0390,		//-4db
	0x03c6,		//-2db
	0x0400,		//0db
	0x043c,		// 2db
	0x047c,		// 4db
	0x048c,		// 5db
	0x049c,		// 6db
	0x04ac,		// 7db
	//0x0509,		// 8db
	//0x0555		// 10db
};

/**
 * @brief  Treb  Bass gain set
 * @param  TrebBassMode: 0 :treb mode;1:bass mode
 * @param  TrebVol: treb volume(0~10)
 * @param  BassVol: bass volume(0~10)
 * @return NONE
 */
void TrebBassSet(uint8_t TrebVol, uint8_t BassVol)
{    
	uint8_t  i;	  
	uint16_t TrebSqrtA = BassTreb_0db[1].sqrtA;
	uint16_t BassSqrtA = BassTreb_0db[0].sqrtA;

    for(i = 1; i <= 10; i++)
    {
        BassTreb_0db[1].sqrtA = TrebSqrtA + (TrebBassTab[TrebVol] - TrebSqrtA)*i/10;
        BassTreb_0db[0].sqrtA = BassSqrtA + (TrebBassTab[BassVol] - BassSqrtA)*i/10;
        EqStyleConfigure(44100, (EqFilterParams*)BassTreb_0db, 2, 0x10000);
        WaitUs(150);
    } 
}

