/** 
 ******************************************************************************* 
 * @file    codec_effects.h  
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
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2> 
 */
#ifndef __CODEC_EFFECTS_H__
#define __CODEC_EFFECTS_H__
#include "type.h"
#include "drc.h"
#include "echo.h"
#include "expander.h"
#include "pitch_shifter.h"
#include "eq.h"
#include "blue_ns.h"
#include "blue_ns_core.h"
#include "user_effects_param.h"


typedef enum __CODEC_EFFECTS_ERROR_CODE
{
	CODEC_EFFECTS_CNT_OVERFLOW = -255,
	CODEC_EFFECTS_PARAM_MISMATCH,
	CODEC_EFFECTS_NOT_SUPPORT,
	
	CODEC_EFFECTS_NO_ERROR = 0,
	
} CODEC_EFFECTS_ERROR_CODE;



//#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)
#define AUDIO_EFFECT_ONLINE_CONFIG_FIFO        512     
#define CPU_MCPS_BUF_SIZE  50

#define EFFECT_PARAM_FLASH_ADDR					 (0x80000 - 4096)
#define PMEM_ADDR           					 	 0x20018000
#define XMEM_ADDR								 				 0x20000000
#define AUDIO_BUART_TX_FIFO_ADDR 				 (0x8000 - USER_BUART_TX_FIFO)
#define AUDIO_BUART_RX_FIFO_ADDR			   (AUDIO_BUART_TX_FIFO_ADDR - USER_BUART_RX_FIFO)
#define AUDIO_ADC_PMEM_ADDR					    (AUDIO_BUART_RX_FIFO_ADDR - USER_ADC_DMA_FIFO)
#define AUDIO_DAC_PMEM_ADDR					    (AUDIO_ADC_PMEM_ADDR - USER_DAC_DMA_FIFO)
          


typedef struct __DRCParam
{
	uint16_t		cf_type;
	uint16_t		mode;
	uint16_t		q_l;
	uint16_t		q_h;
	int16_t			threshold[3];
	int16_t			ratio[3];
	int16_t			attack_tc[3];
	int16_t			release_tc[3];
	int16_t			pregain1;
	int16_t			pregain2;
} DRCParam;
typedef struct __DRCUnit
{
	DRCParam		param;
	DRCContext 		ct;
	uint8_t			enable;
	uint8_t			channel;
} DRCUnit;

typedef struct __EchoParam
{
	int16_t  		 fc;
	int16_t  		 attenuation;
	int16_t  		 delay;
	int16_t			 direct;
	int16_t  		 max_delay;
} EchoParam;
typedef struct __EchoUnit
{
	EchoParam		param;
	EchoContext		ct;
	uint8_t			enable;
	uint8_t			channel;
	uint8_t*		buf;
} EchoUnit;


typedef struct __FilterParams
{
	uint16_t		enable;
	int16_t			type;           /**< filter type, @see EQ_FILTER_TYPE_SET                               */
	uint16_t		f0;             /**< center frequency (peak) or mid-point frequency (shelf)             */
	int16_t			Q;              /**< quality factor (peak) or slope (shelf), format: Q6.10              */
	int16_t			gain;           /**< Gain in dB, format: Q8.8 */
} FilterParams;
typedef struct __EQParam
{
	int16_t			pregain;
	uint16_t		calculation_type;
	FilterParams	eq_params[10];
	
} EQParam;
typedef struct __EQUnit
{
	EQParam			param;
	SwEqContext		ct;
	uint8_t			enable;
	uint8_t			channel;
	SwEqWorkBuffer  w;
	
} EQUnit;


typedef struct __ExpanderParam
{
	int16_t			threshold;
	int16_t			ratio;
	int16_t			attack_time;
	int16_t			release_time;
} ExpanderParam;
typedef struct __ExpanderUnit
{
	ExpanderParam	param;
	ExpanderContext	ct;
	uint8_t			enable;
	uint8_t			channel;
} ExpanderUnit;

typedef struct __BlueNSParam
{
	int16_t       level;
} BlueNSParam;
typedef struct __BlueNSUnit
{
	BlueNSParam  	param;
	BlueNSContext	ct;
	uint8_t			enable;
	uint8_t			channel;
} BlueNSUnit;


typedef struct __PitchShifterParam
{
	int16_t 		semitone_steps;
} PitchShifterParam;
typedef struct __PitchShifterUnit
{
	PitchShifterParam	param;
	PSContext			ct;
	uint8_t				enable;
	uint8_t				channel;
} PitchShifterUnit;


typedef struct __GainControlParam
{
	uint16_t      mute;
	uint16_t      gain;
} GainControlParam;
typedef struct __GainControlUnit
{
	GainControlParam	param;
	uint8_t				enable;
	uint8_t				channel;
} GainControlUnit;


//音效Apply抽象函数指针
typedef void (*AudioEffectApplyFunc)(void *effectUint, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
typedef void (*AudioEffectInitFunc)(void *effectUnit, uint8_t channel, uint32_t sample_rate);

/*音效节点*/
typedef struct __EffectNode
{
	uint8_t					Enable;
	uint8_t					EffectType;		      //音效类型
	uint8_t                 ParamCnt;
	void*		 			EffectUnit; 	      //数据域
	AudioEffectInitFunc     FuncAudioEffectInit;  //音效节点初始化函数
	AudioEffectApplyFunc	FuncAudioEffectApply; //音效节点处理函数
	
} EffectNode;

typedef struct __EffectNodeList
{
	uint8_t		  Channel;		//用于表示音效处理类型为单声道还是立体声，一组音效节点均需要保持一致
	EffectNode	  EffectNode[AUDIO_EFFECT_NODE_NUM];
} EffectNodeList;



extern EffectNodeList  gEffectNodeList;

//drc init and apply
void AudioEffectDRCInit(DRCUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectDRCApply(DRCUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
 
//expander
void AudioEffectExpanderInit(ExpanderUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectExpanderApply(ExpanderUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);

//gain
void AudioEffectPregainInit(GainControlUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectPregainApply(GainControlUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);

//pitch shifter
void AudioEffectPitchShifterInit(PitchShifterUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectPitchShifterApply(PitchShifterUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);

//echo
void AudioEffectEchoInit(EchoUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectEchoApply(EchoUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);

//eq
void AudioEffectEQInit(EQUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectEQApply(EQUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);

//blue ns
void AudioEffectBlueNSInit(BlueNSUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectBlueNSApply(BlueNSUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);

CODEC_EFFECTS_ERROR_CODE AudioEffectDisableAll(void);
CODEC_EFFECTS_ERROR_CODE AudioEffectInit(uint8_t channel);
CODEC_EFFECTS_ERROR_CODE AudioEffectDeInit(void);
CODEC_EFFECTS_ERROR_CODE AudioEffectProcess(int16_t *pcm_in, int16_t *pcm_out, uint32_t samples);

void AudioEffectSaveAllParametersToMemory(void);
void AudioEffectSaveIndivEnValueToMemory(uint8_t);
uint8_t AudioEffectGetIndivParameters(uint8_t, uint8_t *);

void AudioEffectSaveParamToFlash(void);
void AudioEffectReadParamFromFlash(void);

extern uint16_t CPUMcpsValue[CPU_MCPS_BUF_SIZE];
extern uint16_t CPUMcpsCnt;
void CycleCntStart(void);
uint32_t CycleCntEnd(uint32_t SampleCnt, uint32_t SampleRate);
uint32_t GetMemoryUsageInfor(void);
void ClearADCandDACMemoryFifo(void);

#endif
