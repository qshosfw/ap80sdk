/** 
 ******************************************************************************* 
 * @file    user_effects_param.h  
 * @author  Cecilia Wang
 * @version V1.0.0 
 * @date    24-03-2023 
 * @brief   codec effects process function define here
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
 * <h2><center>&copy; COPYRIGHT 2023 MVSilicon </center></h2> 
 */
#ifndef __USER_EFFECTS_PARAM_H__
#define __USER_EFFECTS_PARAM_H__

#include "type.h"

typedef enum __EffectTypeEnum
{
	AUTO_TUNE = 0x00,
	DC_BLOCK,
	DRC,
	ECHO,
	EQ,
	EXPANDER,
	FREQ_SHIFTER,
	HOWLING_SUPPRESSOR,
	NOISE_GATE,
	PITCH_SHIFTER,
	REVERB,
	SILENCE_DETECTOR,
	THREE_D,
	VIRTUAL_BASS,
	VOICE_CHANGER,
	GAIN_CONTROL,
	VOCAL_CUT,
	PLATE_REVERB,
	REVERB_PRO,
	VOICE_CHANGER_PRO,
	PHASE_CONTROL,
	VOCAL_REMOVE,
	PITCH_SHIFTER_PRO,
	VIRTUAL_BASS_CLASSIC,
	PCM_DELAY,
	EXCITER,
	CHORUS,
	AUTO_WAH,
	STEREO_WIDENER,
	PINGPONG,
	THREE_D_PLUS,
	SINE_GENERATOR,
	NOISE_Suppressor_Blue,
	
} EffectTypeEnum;

typedef struct __Effect_Exec_Table
{
	uint8_t   effect_type;
	char     *effect_name;
	
} Effect_Exec_Table;

#define AUDIO_EFFECT_ONLINE_CONFIG_EN 			1		   //1 -- 开启在线调音， 0 -- 关闭在线调音
#if AUDIO_EFFECT_ONLINE_CONFIG_EN
#define AUDIO_EFFECT_ONLINE_CONFIG_FIFO        512 
#endif

#define USER_EFFECT_MAJOR_VERSION		  2
#define USER_EFFECT_MINOR_VERSION		  0
#define USER_EFFECT_PATCH_VERSION		  0


#define USER_EFFECT_CHANNEL  		 	 		1        //通道数;
#define USER_EFFECT_SAMPLE_RATE       12000    //采样率
#define USER_EFFECT_FRAME_SIZE		  	128      //一帧大小
#define AUDIO_EFFECT_NODE_NUM		  	  13       //当前组音效的数量

extern const Effect_Exec_Table user_effect_exec_table[AUDIO_EFFECT_NODE_NUM];//存放的一个音效列表
extern       uint8_t           user_effect_param_table[612];                 //存放的音效列表对应参数

#define USER_BUART_TX_FIFO     256
#define USER_BUART_RX_FIFO     512
#define USER_ADC_DMA_FIFO			4096      //4K Bytes
#define USER_DAC_DMA_FIFO			4096      //4K Bytes

 
#endif
