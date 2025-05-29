/**
 *******************************************************************************
 * @file    sys_vol.h
 * @author  Richard
 * @version V1.0.0
 * @date    07-01-2013
 * @brief   sys vol declared here
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
#ifndef __SYS_VOL_H__
#define __SYS_VOL_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "app_config.h"

///////////////////////////////////////////////////////////
//          Gain Volume Table for PGA	(dB)			 //
///////////////////////////////////////////////////////////
//	Vol // Gain of LineIn // Gain of FMIn // Gain of Mic //
//  0   // 	   5.34       //     21.36     //     42.0    //
//  1   // 	   4.54       //     20.25     //     41.0    //
//  2   // 	   3.62       //     19.13     //     40.1    //
//  3   // 	   2.59       //     18.0      //     39.0    //
//  4   // 	   1.46       //     16.88     //     38.1    //
//  5   // 	   0.12       //     15.77     //     37.2    //
//  6   // 	  -1.16       //     14.59     //     36.1    //
//  7   // 	  -1.91       //     13.37     //     35.1    //
//  8   // 	  -2.8        //     12.11     //     34.2    //
//  9   //    -3.83       //     10.83     //     33.4    //
//  10  // 	  -5.02       //     9.53      //     32.6    //
//  11  // 	  -6.4        //     8.22      //     31.9    //
//  12  //    -6.91       //     6.8       //     30.7    //
//  13  // 	  -8.36       //     5.26      //     29.6    //
//  14  // 	  -9.75       //     3.63      //     28.7    //
//  15  // 	  -11.06      //     1.88      //     27.8    //
//  16  // 	  -12.28	  //     0         //     27.0    //
//  17  // 	  -13.5 	  //    -1.9       //     26.3    //
//  18  // 	  -14.71      //    -3.65      //     25.0    //
//  19  // 	  -15.91	  //    -5.29      //     24.0    //
//  20  // 	  -17.09	  //    -6.82      //     22.9    //
//  21  // 	  -18.23	  //    -8.25      //     22.0    //
//  22  // 	  -19.31	  //    -9.56      //     21.2    //
//  23  // 	  -20.4 	  //    -10.86     //     20.1    //
//  24  // 	  -21.51	  //    -12.15     //     19.2    //
//  25  // 	  -22.62	  //    -13.41     //     18.3    //
//  26  // 	  -23.71	  //    -14.63     //     17.3    //
//  27  // 	  -24.78	  //    -15.82     //     16.3    //
//  28  // 	  -25.8 	  //    -16.93     //     15.3    //
//  29  // 	  -26.84      //    -18.05     //     14.4    //
//  30  // 	  -27.9 	  //    -19.19     //     13.4    //
//  31  // 	  -28.96      //    -20.32     //     12.5    //
//  32  // 	  -30.02	  //    -21.43     //     11.6    //
//  33  // 	  -31.06	  //    -22.52     //     10.6    //
//  34  // 	  -32.05	  //    -23.55     //     9.7     //
//  35  // 	  -33.07	  //    -24.61     //     8.7     //
//  36  // 	  -34.1 	  //    -25.68     //     7.7     //
//  37  // 	  -35.15      //    -26.76     //     6.8     //
//  38  // 	  -36.19	  //    -27.82     //     5.9     //
//  39  // 	  -37.21	  //    -28.87     //     5.0     //
//  40  // 	  -38.19	  //    -29.87     //     4.0     //
//  41  // 	  -39.2 	  //    -30.89     //     3.1     //
//  42  // 	  -40.22      //    -31.93     //     2.1     //
//  43  // 	  -41.25	  //    -32.98     //     1.2     //
//  44  // 	  -42.29	  //    -34.03     //     0.2     //
//  45  // 	  -43.3 	  //    -35.05     //    -0.8     //
//  46  // 	  -44.27      //    -36.03     //    -1.7     //
//  47  // 	  -45.27	  //    -37.04     //    -2.5     //
//  48  // 	  -46.29	  //    -38.07     //    -3.6     //
//  49  // 	  -47.32	  //    -39.11     //    -4.6     //
//  50  // 	  -48.35	  //    -40.14     //    -5.6     //
//  51  // 	  -49.36	  //    -41.16     //    -6.5     //
//  52  // 	  -50.33	  //    -42.13     //    -7.5     //
//  53  // 	  -51.33	  //    -43.13     //    -8.4     //
//  54  // 	  -52.34	  //    -44.15     //    -9.4     //
//  55  // 	  -53.37	  //    -45.18     //    -10.4    //
//  56  // 	  -54.4 	  //    -46.21     //    -11.3    //
//  57  // 	  -55.41      //    -47.22     //    -12.3    //
//  58  // 	  -56.38	  //    -48.19     //    -13.2    //
//  59  // 	  -57.37	  //    -49.19     //    -14.2    //
//  60  // 	  -58.38	  //    -50.2      //    -15.7    //
//  61  // 	  -59.41	  //    -51.23     //    -16.1    //
//  62  // 	  -60.44	  //    -52.26     //    -17.0    //
//  63  // 	  -61.44	  //    -53.26     //    -18.0    //
////////////////////////////////////////////////////////////

//ģ������Դ����
#define AUDIO_CH_NONE				0	//������
#define AUDIO_CH_MIC				1	//������MIC
#define AUDIO_CH_LINEIN				2	//˫����LINEIN
#define AUDIO_CH_FM					3	//˫����FM
#define AUDIO_CH_MIC_LINEIN			4	//MIC + ������LINEIN
#define AUDIO_CH_MIC_FM				5	//MIC + ������FM
#define AUDIO_CH_I2SIN              6   //˫����I2Sin

extern uint8_t AudioSpecTrumStep;
//3D��Ч����
extern void AudioDRCPro(int16_t *PcmDataAddr, int32_t CurNumChannel, int32_t CurSampleRate, int32_t PcmDataLen);
extern void Audio3dPro(int16_t *PcmDataAddr, int32_t CurNumChannel, int32_t CurSampleRate, int32_t PcmDataLen);
//VB��Ч����
extern void AudioVbPro(int16_t *PcmDataAddr, int32_t CurNumChannel, uint32_t CurSamplerate, int32_t PcmDataLen);
//���浱ǰ֡����ȡ��һ֡
extern uint32_t SaveCurSendLast(int32_t *PcmIn, uint16_t PcmDataLen, uint32_t CurChannelNum);
//�������������Ч
extern void DealWithVb3dPcmPara(uint32_t CurChannelNum, uint32_t CurSampleRate);
//��ʼ��
extern void InitVb3dPcmPara(void);
//��������Ԥ���棬Vol <= 4095,����ƥ�䲻ͬģʽ֮��ĵ�ƽ����
extern void DigitalGainSet(int32_t *PcmIn, uint16_t PcmDataLen, uint32_t CurChannelNum, uint16_t Vol);
//���ֵ�����������������pop�������DigitalFadeIn()��������ҪӦ�ò��Լ�������޸�
extern void DigitalFadeOut(int32_t *PcmIn, uint16_t PcmSampleLen, uint32_t CurChannelNum);
//���ֵ������
extern void DigitalFadeIn(int32_t *PcmIn, uint16_t PcmSampleLen, uint32_t CurChannelNum);

#if (EQ_STYLE_MODE == 2) || (EQ_STYLE_MODE == 3)
#define AudioSwEQPro(PcmDataAddr, CurNumChannel, PcmDataLen) \
do \
{ \
	int32_t ts = 0; \
	int16_t* PcmBufAddr = (int16_t *)PcmDataAddr; \
	extern SwEqContext gSwEq;\
	extern SwEqWorkBuffer gSwEqWorkBuffer;\
	while(ts < PcmDataLen) \
	{ \
		int32_t cs = PcmDataLen - ts > SW_EQ_PROC_FRAME_SAMPLES ? SW_EQ_PROC_FRAME_SAMPLES : PcmDataLen - ts; \
		SwEqApply(&gSwEq, &gSwEqWorkBuffer, (int16_t *)&PcmBufAddr[ts * CurNumChannel], cs, CurNumChannel); \
		ts += cs; \
	} \
}while(0)
#endif

//Ƶ�ײɼ�
void SpecTrumProc(int16_t* Buffer, uint32_t Len);

//����ϵͳ������
void SetSysVol(void);

#ifdef FUNC_BT_HF_EN
// ��������HF����
void SetBtHfVol(uint8_t vol);

// ��ȡ����HF����
uint8_t GetBtHfVol(void);
#endif

//����������ȡ��
void AudioPlayerMute(bool MuteEnFlag);

//ģ��ͨ·��ʼ��
void AudioAnaInit(void);

//�������ͨ·��ʼ��
void AudioOutputInit(void);

//����ͨ·����������
void AudioSampleRateSet(uint32_t CurSamplerate);

//����ģ��ͨ·
void AudioAnaSetChannel(uint8_t Channel);

//����I2S����I2S��DACͬʱ���ͨ��
void AudioI2sOutSetChannel(uint32_t SampleRate, bool IsAndDacOut);

//����STA���ͨ��
void AudioStaOutSetChannel(uint32_t SampleRate, bool IsAndDacOut);

//��ģʽ�µ�ͨ����Ϣ����
void CommonMsgProccess(uint16_t Msg);

#ifdef FUNC_AUDIO_DRC_EFFECT_EN
extern bool	gDRCEffectInited;
int32_t InitDrc(DRCContext *ct, int32_t num_channels, int32_t sample_rate);
#endif
/**
 * @brief  set all audio channel EQ
 * @param  NONE
 * @return NONE
 */
void AudioPlayerSwitchEQ(void);

/**
 * @brief  Treb  Bass gain set
 * @param  TrebBassMode: 0 :treb mode;1:bass mode
 * @param  TrebVol: treb volume(0~10)
 * @param  BassVol: bass volume(0~10)
 * @return NONE
 */
void TrebBassSet(uint8_t TrebVol, uint8_t BassVol);


#ifdef FUNC_AUDIO_3D_EFFECT_EN
extern bool	g3dEffectInited;
#endif

#ifdef FUNC_AUDIO_VB_EFFECT_EN
extern bool gVbEffectInited;
//#define WORKBUFSIZE sizeof(VB_BUFFER)
//extern uint8_t vbBuffer[WORKBUFSIZE];
#endif

extern uint8_t			mixer_pcm_format;
extern const uint16_t gDecVolArr[MAX_VOLUME + 1];

extern bool IsFillMemData;

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif

