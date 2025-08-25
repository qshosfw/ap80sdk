/******************************************************************************
 * @file    mixer.h
 * @author  Orson
 * @version V1.0.0
 * @date    29-April-2014
 * @brief   audio mixer
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#ifndef __CODEC_MIXER_H__
#define __CODEC_MIXER_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#define MIXER_FORMAT_MONO			1	//��������Դ��ÿ��������2���ֽ�
#define MIXER_FORMAT_STERO			2	//˫������Դ��ÿ��������4���ֽ�


//Mixerģ���ʼ��
//PcmFifoAddr: ������PMEM��
void Codec_MixerInit(void* PcmFifoAddr, uint16_t PcmFifoSize);
	
//PcmFormat:   1--��������2--˫����
void Codec_MixerConfigFormat(uint8_t PcmFormat);

//���ĳͨ���Ƿ���������µ�����
bool Codec_MixerIsDone(void);

//PcmBuf:      PCM�������׵�ַ
//SampleCnt:   ��������
void Codec_MixerSetData(void* PcmBuf, uint16_t SampleCnt);

//��ȡĳ����Դ��Mixer��ʣ��Ĳ�������������Ϊ�ڸ�����Դ�������µĵ���
uint16_t Codec_MixerGetRemainSamples(void);



#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
