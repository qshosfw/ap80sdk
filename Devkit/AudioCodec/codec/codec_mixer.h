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

#define MIXER_FORMAT_MONO			1	//单声道音源，每个采样点2个字节
#define MIXER_FORMAT_STERO			2	//双声道音源，每个采样点4个字节


//Mixer模块初始化
//PcmFifoAddr: 必须在PMEM中
void Codec_MixerInit(void* PcmFifoAddr, uint16_t PcmFifoSize);
	
//PcmFormat:   1--单声道，2--双声道
void Codec_MixerConfigFormat(uint8_t PcmFormat);

//检查某通道是否可以设置新的数据
bool Codec_MixerIsDone(void);

//PcmBuf:      PCM缓冲区首地址
//SampleCnt:   采样点数
void Codec_MixerSetData(void* PcmBuf, uint16_t SampleCnt);

//获取某输入源在Mixer内剩余的采样点数，折算为在该输入源采样率下的点数
uint16_t Codec_MixerGetRemainSamples(void);



#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
