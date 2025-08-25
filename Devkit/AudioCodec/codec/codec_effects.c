/** 
 ******************************************************************************* 
 * @file    codec_effects.c  
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

#include <stdint.h>
#include <math.h>
#include <string.h>
#include "type.h"
#include "pcm_fifo.h"
#include "codec_effects.h"
#include "dac.h"
#include "echo.h"
#include "eq.h"
#include "drc.h"
#include "pitch_shifter.h"
#include "silence_detector.h"
#include "expander.h"
#include "user_effects_param.h"
#include "mcu_circular_buf.h"
#include "spi_flash.h"
#include "clk.h"

extern int  __temporary_stack_top$libspace;
#define  DYNAMIC_ALLOCATION_START_ADDR ((uint32_t)&__temporary_stack_top$libspace + 2048)//0x20004A00//0x20003800
#define  DYNAMIC_ALLOCATION_END_ADDR    (PMEM_ADDR + AUDIO_DAC_PMEM_ADDR-16)
uint32_t DynamicAddr = DYNAMIC_ALLOCATION_START_ADDR;

EffectNodeList  gEffectNodeList;
int16_t AudioMixBuf[USER_EFFECT_FRAME_SIZE*2];


uint16_t CPUMcpsValue[CPU_MCPS_BUF_SIZE];
uint16_t CPUMcpsCnt = 0;

CODEC_EFFECTS_ERROR_CODE AudioEffectDisableAll(void)
{
	uint16_t i, j;

	for(i = 0; i < AUDIO_EFFECT_NODE_NUM; i++)
	{
		gEffectNodeList.EffectNode[i].Enable = 0;
	}
	return CODEC_EFFECTS_NO_ERROR;
}

void AudioEffectReadParamFromFlash(void)
{
	uint8_t* regs = (uint8_t*)EFFECT_PARAM_FLASH_ADDR;
	if(regs[0] == 0xA5 && regs[1] == 0x5A && regs[2] == USER_EFFECT_MAJOR_VERSION && regs[3] == USER_EFFECT_MINOR_VERSION)
	{
		memcpy(user_effect_param_table, &regs[4], sizeof(user_effect_param_table));
	}
}

void AudioEffectSaveParamToFlash(void)
{
	int32_t offset = EFFECT_PARAM_FLASH_ADDR;
	uint8_t buf[4];
	buf[0] = 0xA5;
	buf[1] = 0x5A;
	buf[2] = USER_EFFECT_MAJOR_VERSION;
	buf[3] = USER_EFFECT_MINOR_VERSION;
	
	 __DISABLE_IRQ();
	SpiFlashInfoInit();
	SpiFlashErase(offset, 4096);
	SpiFlashWrite(offset, buf, sizeof(buf));
	SpiFlashWrite(offset+sizeof(buf), user_effect_param_table, sizeof(user_effect_param_table));
	__ENABLE_IRQ();
}

CODEC_EFFECTS_ERROR_CODE AudioEffectInit(uint8_t channel)
{
	uint16_t i, j;

	gEffectNodeList.Channel = channel;
	
	DynamicAddr = DYNAMIC_ALLOCATION_START_ADDR;
	if(DynamicAddr % 4)
	{
		DynamicAddr = ((DynamicAddr >> 2) << 2) + 4;
	}

	for(i = 0, j = 0; i < AUDIO_EFFECT_NODE_NUM; i++)
	{
		gEffectNodeList.EffectNode[i].EffectType = user_effect_exec_table[i].effect_type;
		switch(user_effect_exec_table[i].effect_type)
		{
			case DRC:
			{
				//APP_DBG("DRCUnit Size = %d\n", sizeof(DRCUnit));
				gEffectNodeList.EffectNode[i].ParamCnt = sizeof(DRCParam)/sizeof(uint16_t);
				if(DynamicAddr + sizeof(DRCUnit) <= DYNAMIC_ALLOCATION_END_ADDR)
				{
					if(((user_effect_param_table[j]-1)-2) != sizeof(DRCParam))
					{
						return CODEC_EFFECTS_PARAM_MISMATCH;
					}
					gEffectNodeList.EffectNode[i].Enable = user_effect_param_table[j+2];
					if(gEffectNodeList.EffectNode[i].Enable)
					{
						gEffectNodeList.EffectNode[i].EffectUnit = (void *)DynamicAddr;
						memset((void *)gEffectNodeList.EffectNode[i].EffectUnit, 0, sizeof(DRCUnit));
						memcpy(&((DRCUnit*)(gEffectNodeList.EffectNode[i].EffectUnit))->param, &user_effect_param_table[j+4], sizeof(DRCParam));
						gEffectNodeList.EffectNode[i].FuncAudioEffectApply = (AudioEffectApplyFunc)AudioEffectDRCApply;
						gEffectNodeList.EffectNode[i].FuncAudioEffectInit = (AudioEffectInitFunc)AudioEffectDRCInit;
						((DRCUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->enable = gEffectNodeList.EffectNode[i].Enable;
						((DRCUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->channel = USER_EFFECT_CHANNEL;
						AudioEffectDRCInit(gEffectNodeList.EffectNode[i].EffectUnit, USER_EFFECT_CHANNEL, USER_EFFECT_SAMPLE_RATE);
						DynamicAddr += sizeof(DRCUnit);
					}
				}
				else
				{
					gEffectNodeList.EffectNode[i].ParamCnt = 0;
					gEffectNodeList.EffectNode[i].Enable = 0;
				}
				break;
			}
			
			case EXPANDER:
			{
				//APP_DBG("ExpanderUnit Size = %d\n", sizeof(ExpanderUnit));
				gEffectNodeList.EffectNode[i].ParamCnt = sizeof(ExpanderParam)/sizeof(uint16_t);
				if(DynamicAddr + sizeof(ExpanderUnit) <= DYNAMIC_ALLOCATION_END_ADDR)
				{
					if(((user_effect_param_table[j]-1)-2) != sizeof(ExpanderParam))
					{
						return CODEC_EFFECTS_PARAM_MISMATCH;
					}
					gEffectNodeList.EffectNode[i].Enable = user_effect_param_table[j+2];
					if(gEffectNodeList.EffectNode[i].Enable)
					{
						gEffectNodeList.EffectNode[i].ParamCnt = sizeof(ExpanderParam)/sizeof(uint16_t);
						gEffectNodeList.EffectNode[i].EffectUnit = (void *)DynamicAddr;
						memset((void *)gEffectNodeList.EffectNode[i].EffectUnit, 0, sizeof(ExpanderUnit));
						memcpy(&((ExpanderUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->param, &user_effect_param_table[j+4], sizeof(ExpanderParam));
						gEffectNodeList.EffectNode[i].FuncAudioEffectApply = (AudioEffectApplyFunc)AudioEffectExpanderApply;
						gEffectNodeList.EffectNode[i].FuncAudioEffectInit = (AudioEffectInitFunc)AudioEffectExpanderInit;
						((ExpanderUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->enable = gEffectNodeList.EffectNode[i].Enable;
						((ExpanderUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->channel = USER_EFFECT_CHANNEL;
						AudioEffectExpanderInit(gEffectNodeList.EffectNode[i].EffectUnit, USER_EFFECT_CHANNEL, USER_EFFECT_SAMPLE_RATE);
						DynamicAddr += sizeof(ExpanderUnit);
					}
				}
				else
				{
					gEffectNodeList.EffectNode[i].ParamCnt = 0;
					gEffectNodeList.EffectNode[i].Enable = 0;
				}
				break;
			}
			
			case GAIN_CONTROL:
			{
				//APP_DBG("GainControlUnit Size = %d\n", sizeof(GainControlUnit));
				gEffectNodeList.EffectNode[i].ParamCnt = sizeof(GainControlParam)/sizeof(uint16_t);
				if(DynamicAddr + sizeof(GainControlUnit) <= DYNAMIC_ALLOCATION_END_ADDR)
				{
					if(((user_effect_param_table[j]-1)-2) != sizeof(GainControlParam))
					{
						return CODEC_EFFECTS_PARAM_MISMATCH;
					}
					gEffectNodeList.EffectNode[i].Enable = user_effect_param_table[j+2];
					
					if(gEffectNodeList.EffectNode[i].Enable)
					{
						gEffectNodeList.EffectNode[i].EffectUnit = (void *)DynamicAddr;
						memset((void *)gEffectNodeList.EffectNode[i].EffectUnit, 0, sizeof(GainControlUnit));
						memcpy(&((GainControlUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->param, &user_effect_param_table[j+4], sizeof(GainControlParam));
						gEffectNodeList.EffectNode[i].FuncAudioEffectApply = (AudioEffectApplyFunc)AudioEffectPregainApply;
						gEffectNodeList.EffectNode[i].FuncAudioEffectInit = (AudioEffectInitFunc)AudioEffectPregainInit;
						((GainControlUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->enable = gEffectNodeList.EffectNode[i].Enable;
						((GainControlUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->channel = USER_EFFECT_CHANNEL;
						AudioEffectPregainInit(gEffectNodeList.EffectNode[i].EffectUnit, USER_EFFECT_CHANNEL, USER_EFFECT_SAMPLE_RATE);
						DynamicAddr += sizeof(GainControlUnit);
					}
				}
				else
				{
					gEffectNodeList.EffectNode[i].ParamCnt = 0;
					gEffectNodeList.EffectNode[i].Enable = 0;
				}
				break;
			}
			
			case PITCH_SHIFTER:
			{
				//APP_DBG("PitchShifterUnit Size = %d\n", sizeof(PitchShifterUnit));
				gEffectNodeList.EffectNode[i].ParamCnt = sizeof(PitchShifterParam)/sizeof(uint16_t);
				if(DynamicAddr + sizeof(PitchShifterUnit) <= DYNAMIC_ALLOCATION_END_ADDR)
				{
					if(((user_effect_param_table[j]-1)-2) != sizeof(PitchShifterParam))
					{
						return CODEC_EFFECTS_PARAM_MISMATCH;
					}
					gEffectNodeList.EffectNode[i].Enable = user_effect_param_table[j+2];
					
					if(gEffectNodeList.EffectNode[i].Enable)
					{
						gEffectNodeList.EffectNode[i].EffectUnit = (void *)DynamicAddr;
						memset((void *)gEffectNodeList.EffectNode[i].EffectUnit, 0, sizeof(PitchShifterUnit));
						memcpy(&((PitchShifterUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->param, &user_effect_param_table[j+4], sizeof(PitchShifterParam));
						gEffectNodeList.EffectNode[i].FuncAudioEffectApply = (AudioEffectApplyFunc)AudioEffectPitchShifterApply;
						gEffectNodeList.EffectNode[i].FuncAudioEffectInit = (AudioEffectInitFunc)AudioEffectPitchShifterInit;
						((PitchShifterUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->enable = gEffectNodeList.EffectNode[i].Enable;
						((PitchShifterUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->channel = USER_EFFECT_CHANNEL;
						AudioEffectPitchShifterInit(gEffectNodeList.EffectNode[i].EffectUnit, USER_EFFECT_CHANNEL, USER_EFFECT_SAMPLE_RATE);
						DynamicAddr += sizeof(PitchShifterUnit);
					}
				}
				else
				{
					gEffectNodeList.EffectNode[i].ParamCnt = 0;
					gEffectNodeList.EffectNode[i].Enable = 0;
				}
				break;
			}
			
			case ECHO:
			{
				EchoParam *p = (EchoParam *)&user_effect_param_table[j+4];
				uint32_t   buf_size;
				
				gEffectNodeList.EffectNode[i].ParamCnt = sizeof(EchoParam)/sizeof(uint16_t);
				if(((user_effect_param_table[j]-1)-2) != sizeof(EchoParam))
				{
					return CODEC_EFFECTS_PARAM_MISMATCH;
				}
				
				buf_size = (USER_EFFECT_SAMPLE_RATE * (uint32_t)p->max_delay/1000 + 32)/2;
				
				//APP_DBG("EchoUnit Size = %d, buf Size = %d\n", sizeof(EchoUnit) , buf_size);
				
				if(DynamicAddr + sizeof(EchoUnit) +  buf_size <= DYNAMIC_ALLOCATION_END_ADDR)
				{
					gEffectNodeList.EffectNode[i].Enable = user_effect_param_table[j+2];
					
					if(gEffectNodeList.EffectNode[i].Enable)
					{
						EchoUnit *pEcho;
						
						gEffectNodeList.EffectNode[i].EffectUnit = (void *)DynamicAddr;
						memset((void *)gEffectNodeList.EffectNode[i].EffectUnit, 0, sizeof(EchoUnit));
						DynamicAddr += sizeof(EchoUnit);
						pEcho = (EchoUnit *)gEffectNodeList.EffectNode[i].EffectUnit;
						pEcho->buf = (void *)DynamicAddr;
						memset((void *)pEcho->buf, 0, buf_size);
						DynamicAddr += buf_size;
						memcpy(&((EchoUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->param, &user_effect_param_table[j+4], sizeof(EchoParam));
						gEffectNodeList.EffectNode[i].FuncAudioEffectInit = (AudioEffectInitFunc)AudioEffectEchoInit;
						gEffectNodeList.EffectNode[i].FuncAudioEffectApply = (AudioEffectApplyFunc)AudioEffectEchoApply;
						((EchoUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->enable = gEffectNodeList.EffectNode[i].Enable;
						((EchoUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->channel = USER_EFFECT_CHANNEL;
						AudioEffectEchoInit(gEffectNodeList.EffectNode[i].EffectUnit, USER_EFFECT_CHANNEL, USER_EFFECT_SAMPLE_RATE);//USER_EFFECT_SAMPLE_RATE
					}
				}
				else
				{
					gEffectNodeList.EffectNode[i].ParamCnt = 0;
					gEffectNodeList.EffectNode[i].Enable = 0;
				}
				break;
			}
			
			case EQ:
			{
				//APP_DBG("EQUnit Size = %d\n", sizeof(EQUnit));
				gEffectNodeList.EffectNode[i].ParamCnt = sizeof(EQParam)/sizeof(uint16_t);
				if(DynamicAddr + sizeof(EQUnit) <= DYNAMIC_ALLOCATION_END_ADDR)
				{
					if(((user_effect_param_table[j]-1)-2) != sizeof(EQParam))
					{
						return CODEC_EFFECTS_PARAM_MISMATCH;
					}
					gEffectNodeList.EffectNode[i].Enable = user_effect_param_table[j+2];
					
					if(gEffectNodeList.EffectNode[i].Enable)
					{
						gEffectNodeList.EffectNode[i].EffectUnit = (void *)DynamicAddr;
						memset((void *)gEffectNodeList.EffectNode[i].EffectUnit, 0, sizeof(EQUnit));
						memcpy(&((EQUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->param, &user_effect_param_table[j+4], sizeof(EQParam));
						gEffectNodeList.EffectNode[i].FuncAudioEffectApply = (AudioEffectApplyFunc)AudioEffectEQApply;
						gEffectNodeList.EffectNode[i].FuncAudioEffectInit = (AudioEffectInitFunc)AudioEffectEQInit;
						((EQUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->enable = gEffectNodeList.EffectNode[i].Enable;
						((EQUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->channel = USER_EFFECT_CHANNEL;
						AudioEffectEQInit(gEffectNodeList.EffectNode[i].EffectUnit, USER_EFFECT_CHANNEL, USER_EFFECT_SAMPLE_RATE);
						DynamicAddr += sizeof(EQUnit);
					}
				}
				else
				{
					gEffectNodeList.EffectNode[i].ParamCnt = 0;
					gEffectNodeList.EffectNode[i].Enable = 0;
				}
				break;
			}
			
			case NOISE_Suppressor_Blue:
			{
				//APP_DBG("BlueNs Size = %d\n", sizeof(BlueNSUnit));
				gEffectNodeList.EffectNode[i].ParamCnt = sizeof(BlueNSParam)/sizeof(uint16_t);
				if(DynamicAddr + sizeof(BlueNSUnit) <= DYNAMIC_ALLOCATION_END_ADDR)
				{
					if(((user_effect_param_table[j]-1)-2) != sizeof(BlueNSParam))
					{
						return CODEC_EFFECTS_PARAM_MISMATCH;
					}
					gEffectNodeList.EffectNode[i].Enable = user_effect_param_table[j+2];
					
					if(gEffectNodeList.EffectNode[i].Enable)
					{
						gEffectNodeList.EffectNode[i].EffectUnit = (void *)DynamicAddr;
						memset((void *)gEffectNodeList.EffectNode[i].EffectUnit, 0, sizeof(BlueNSUnit));
						memcpy(&((BlueNSUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->param, &user_effect_param_table[j+4], sizeof(BlueNSParam));
						gEffectNodeList.EffectNode[i].FuncAudioEffectApply = (AudioEffectApplyFunc)AudioEffectBlueNSApply;
						gEffectNodeList.EffectNode[i].FuncAudioEffectInit = (AudioEffectInitFunc)AudioEffectBlueNSInit;
						((BlueNSUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->enable = gEffectNodeList.EffectNode[i].Enable;
						((BlueNSUnit*)gEffectNodeList.EffectNode[i].EffectUnit)->channel = USER_EFFECT_CHANNEL;
						AudioEffectBlueNSInit(gEffectNodeList.EffectNode[i].EffectUnit, USER_EFFECT_CHANNEL, USER_EFFECT_SAMPLE_RATE);
						DynamicAddr += sizeof(BlueNSUnit);
					}
				}
				else
				{
					gEffectNodeList.EffectNode[i].ParamCnt = 0;
					gEffectNodeList.EffectNode[i].Enable = 0;
				}
				break;
			}
			
			default:
				return CODEC_EFFECTS_NOT_SUPPORT;
				break;
		}
		
		j = j + user_effect_param_table[j] + 1;
		if(DynamicAddr % 4)
		{
			DynamicAddr = ((DynamicAddr >> 2) << 2) + 4;
			DynamicAddr = DynamicAddr > DYNAMIC_ALLOCATION_END_ADDR?DYNAMIC_ALLOCATION_END_ADDR:DynamicAddr;
		}
	}
    return CODEC_EFFECTS_NO_ERROR;
}

CODEC_EFFECTS_ERROR_CODE AudioEffectDeInit(void)
{
	uint8_t i;
	uint8_t userEffectCnt;
	
	userEffectCnt =  sizeof(user_effect_exec_table)/sizeof(uint16_t);
	if(userEffectCnt > AUDIO_EFFECT_NODE_NUM)
	{
		return CODEC_EFFECTS_CNT_OVERFLOW;
	}
	for(i = 0; i < userEffectCnt; i++)
	{
		gEffectNodeList.EffectNode[i].EffectUnit = NULL;
	}
	DynamicAddr = DYNAMIC_ALLOCATION_START_ADDR;
	if(DynamicAddr % 4)
	{
		DynamicAddr = ((DynamicAddr >> 2) << 2) + 4;
	}
	return CODEC_EFFECTS_NO_ERROR;
}

CODEC_EFFECTS_ERROR_CODE AudioEffectProcess(int16_t *pcm_in, int16_t *pcm_out, uint32_t samples)
{
	uint32_t i;
	uint8_t userEffectCnt;
	EffectNode*  pNode = NULL;
	
		
	if(USER_EFFECT_CHANNEL == 1)
	{
		for(i = 0; i < samples; i++)
		{
			AudioMixBuf[i] = pcm_in[2*i+0];//
		}
	}
	
	for(i=0; i<AUDIO_EFFECT_NODE_NUM; i++)
	{
		pNode = &gEffectNodeList.EffectNode[i];//伴奏使用第0组音效列表
		if((pNode->Enable == FALSE) || (pNode->EffectUnit == NULL))
		{
			continue;
		}
		pNode->FuncAudioEffectApply(pNode->EffectUnit, AudioMixBuf, AudioMixBuf, samples);
	}
	
	if(USER_EFFECT_CHANNEL == 1)
	{
		for(i = 0; i < samples; i++)
		{
			pcm_out[2*i+0] = AudioMixBuf[i];
			pcm_out[2*i+1] = AudioMixBuf[i];
		}
	}

	
	return CODEC_EFFECTS_NO_ERROR;
}

uint8_t AudioEffectGetIndivParameters(uint8_t curIdx, uint8_t *buf_out)
{
	uint16_t i, j;

	for(i = 0, j = 0; i < AUDIO_EFFECT_NODE_NUM; i++)
	{
		if(curIdx == i)
		{
			memcpy(buf_out, &user_effect_param_table[j+4], user_effect_param_table[j]-3);
			return user_effect_param_table[j]-3;
		}	
		j = j + user_effect_param_table[j] + 1;
	}
    return 0;
}

void AudioEffectSaveIndivEnValueToMemory(uint8_t curIdx)
{
	uint16_t i, j;

	for(i = 0, j = 0; i < AUDIO_EFFECT_NODE_NUM; i++)
	{
		if(curIdx == i)
		{
			user_effect_param_table[j+2] = 0;
			if(gEffectNodeList.EffectNode[i].Enable)
				user_effect_param_table[j+2] = 1;
			else
				user_effect_param_table[j+2] = 0;
		}
			
		j = j + user_effect_param_table[j] + 1;
	}	
}

void AudioEffectSaveAllParametersToMemory(void)
{
	uint16_t i, j;

	for(i = 0, j = 0; i < AUDIO_EFFECT_NODE_NUM; i++)
	{
		gEffectNodeList.EffectNode[i].EffectType = user_effect_exec_table[i].effect_type;
		switch(user_effect_exec_table[i].effect_type)
		{
			case DRC:
			{
				user_effect_param_table[j+2] = 0;
				if(gEffectNodeList.EffectNode[i].Enable)
				{
					user_effect_param_table[j+2] = 1;
					memcpy(&user_effect_param_table[j+4], &((DRCUnit*)(gEffectNodeList.EffectNode[i].EffectUnit))->param,  sizeof(DRCParam));
				}
				break;
			}
			
			case EXPANDER:
			{
				user_effect_param_table[j+2] = 0;
				if(gEffectNodeList.EffectNode[i].Enable)
				{
					user_effect_param_table[j+2] = 1;
					memcpy(&user_effect_param_table[j+4], &((ExpanderUnit*)(gEffectNodeList.EffectNode[i].EffectUnit))->param,  sizeof(ExpanderParam));
				}
				break;
			}
			
			case GAIN_CONTROL:
			{
				user_effect_param_table[j+2] = 0;
				if(gEffectNodeList.EffectNode[i].Enable)
				{
					user_effect_param_table[j+2] = 1;
					memcpy(&user_effect_param_table[j+4], &((GainControlUnit*)(gEffectNodeList.EffectNode[i].EffectUnit))->param,  sizeof(GainControlParam));
				}
				break;
			}
			
			case PITCH_SHIFTER:
			{
				user_effect_param_table[j+2] = 0;
				if(gEffectNodeList.EffectNode[i].Enable)
				{
					user_effect_param_table[j+2] = 1;
					memcpy(&user_effect_param_table[j+4], &((PitchShifterUnit*)(gEffectNodeList.EffectNode[i].EffectUnit))->param,  sizeof(PitchShifterParam));
				}
				break;
			}
			
			case ECHO:
			{
				user_effect_param_table[j+2] = 0;
				if(gEffectNodeList.EffectNode[i].Enable)
				{
					user_effect_param_table[j+2] = 1;
					memcpy(&user_effect_param_table[j+4], &((EchoUnit*)(gEffectNodeList.EffectNode[i].EffectUnit))->param,  sizeof(EchoParam));
				}
				break;
			}
			
			case EQ:
			{
				user_effect_param_table[j+2] = 0;
				if(gEffectNodeList.EffectNode[i].Enable)
				{
					user_effect_param_table[j+2] = 1;
					memcpy(&user_effect_param_table[j+4], &((EQUnit*)(gEffectNodeList.EffectNode[i].EffectUnit))->param,  sizeof(EQParam));
				}
				break;
			}
			
			case NOISE_Suppressor_Blue:
			{
				user_effect_param_table[j+2] = 0;
				if(gEffectNodeList.EffectNode[i].Enable)
				{
					user_effect_param_table[j+2] = 1;
					memcpy(&user_effect_param_table[j+4], &((BlueNSUnit*)(gEffectNodeList.EffectNode[i].EffectUnit))->param,  sizeof(BlueNSParam));
				}
				break;
			}
			
			default:
				break;
		}
			
		j = j + user_effect_param_table[j] + 1;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////
//***************************以下是对音效函数的配置和调用***********************************


#define CFG_MIC_PITCH_SHIFTER_FRAME_SIZE               (256)// (512)  //unit in sample
void AudioEffectPitchShifterInit(PitchShifterUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	if(unit->enable == FALSE)
	{
		return;
	}
	unit->channel = channel;
	init_pitch_shifter(&unit->ct, channel, sample_rate, unit->param.semitone_steps, CFG_MIC_PITCH_SHIFTER_FRAME_SIZE);//512
}

void AudioEffectPitchShifterApply(PitchShifterUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if(unit->enable == FALSE)
	{
		return;
	}
	if(1)
	{
		uint32_t PSSample = (CFG_MIC_PITCH_SHIFTER_FRAME_SIZE >> 1);
		uint32_t Cnt = n / PSSample;
		uint16_t iIdx;

		for(iIdx = 0; iIdx < Cnt; iIdx++)
		{
			apply_pitch_shifter(&unit->ct, (int16_t *)(pcm_in  + (PSSample * unit->channel) * iIdx),
									  	   (int16_t *)(pcm_out + (PSSample * unit->channel) * iIdx));
		}
	}
}


void AudioEffectPregainInit(GainControlUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	if(unit->enable == FALSE)
	{
		return;
	}
	unit->channel = channel;
}

void AudioEffectPregainApply(GainControlUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	int32_t s;
	int32_t pregain;

	if(unit->enable)
	{
		pregain = unit->param.mute? 0 : unit->param.gain;
		for(s = 0; s < n; s++)
		{
			if(unit->channel == 2)
			{
				pcm_out[2 * s + 0] = __ssat((((int32_t)pcm_in[2 * s + 0] * pregain + 2048) >> 12), 16-1);
				pcm_out[2 * s + 1] = __ssat((((int32_t)pcm_in[2 * s + 1] * pregain + 2048) >> 12), 16-1);
			}
			else
			{
				pcm_out[s] = __ssat((((int32_t)pcm_in[s] * pregain + 2048) >> 12), 16-1);
			}
		}
	}
}

void AudioEffectBlueNSInit(BlueNSUnit *unit, uint8_t channel, uint32_t sample_rate)
{
    if(unit->enable == FALSE)
	{
		return;
	}
	if(channel != 1)
	{
		unit->enable = FALSE;
		return;
	}
    unit->channel = channel;
	blue_ns_init(&unit->ct);
}

void AudioEffectBlueNSApply(BlueNSUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if(unit->enable == FALSE)
	{
		return;
	}
	if(1)
	{
		uint32_t PSSample = BLK_LEN;
		uint32_t Cnt = n / PSSample;
		uint32_t RemainCnt = n - Cnt * PSSample;
		uint16_t iIdx;
		int32_t ns_level = unit->param.level;
		
		ns_level = ns_level > 5? 5:ns_level;

		for(iIdx = 0; iIdx < Cnt; iIdx++)
		{
			blue_ns_run(&unit->ct, (int16_t *)(pcm_in  + PSSample * iIdx), (int16_t *)(pcm_out  + PSSample * iIdx), ns_level);
		}
		if(RemainCnt > 0)
		{
			blue_ns_run(&unit->ct, (int16_t *)(pcm_in  + PSSample *  Cnt), (int16_t *)(pcm_out  + PSSample *  Cnt), ns_level);
		}
	}
}

void AudioEffectExpanderInit(ExpanderUnit *unit, uint8_t channel, uint32_t sample_rate)
{
    if(unit->enable == FALSE)
	{
		return;
	}
    unit->channel = channel;
	expander_init(&unit->ct,  channel, sample_rate, unit->param.threshold, unit->param.ratio, unit->param.attack_time, unit->param.release_time);
}

void AudioEffectExpanderApply(ExpanderUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if(unit->enable == FALSE)
	{
		return;
	}
	expander_apply(&unit->ct, pcm_in, pcm_out, n);
}


void AudioEffectDRCInit(DRCUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	//位宽对齐
	int32_t fc = unit->param.cf_type;
	uint16_t q[2];// = {unit->param.q_l, unit->param.q_h}; 
	int32_t threshold[3];// = {unit->param.threshold[0], unit->param.threshold[1], unit->param.threshold[2]};
	int32_t ratio[3];// = {unit->param.ratio[0], unit->param.ratio[1],  unit->param.ratio[2]};
	int32_t attack_tc[3];// = {unit->param.attack_tc[0], unit->param.attack_tc[1], unit->param.attack_tc[2]};
	int32_t release_tc[3];// = {unit->param.release_tc[0], unit->param.release_tc[1],  unit->param.release_tc[2]};

	if(unit->enable == FALSE)
	{
		return;
	}
	q[0] =  unit->param.q_l;
	q[1] =  unit->param.q_h;
	threshold[0] = unit->param.threshold[0];
	threshold[1] = unit->param.threshold[1];
	threshold[2] = unit->param.threshold[2];
	ratio[0] = unit->param.ratio[0];
	ratio[1] = unit->param.ratio[1];
	ratio[2] = unit->param.ratio[2];
	attack_tc[0] = unit->param.attack_tc[0];
	attack_tc[1] = unit->param.attack_tc[1];
	attack_tc[2] = unit->param.attack_tc[2];
	
	release_tc[0] = unit->param.release_tc[0];
	release_tc[1] = unit->param.release_tc[1];
	release_tc[2] = unit->param.release_tc[2];
	
	unit->channel = channel;
	
	drc_init(&unit->ct, unit->channel, sample_rate, fc,  unit->param.mode, q, threshold, ratio, attack_tc, release_tc);	
}

void AudioEffectDRCApply(DRCUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if(unit->enable == FALSE)
	{
		return;
	}
	drc_apply(&unit->ct, pcm_in, pcm_out, n, unit->param.pregain1, unit->param.pregain2);
}


void AudioEffectEchoInit(EchoUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	uint32_t max_delay_samples;
	if(unit->enable == FALSE)
	{
		return;
	}
	unit->channel = channel;
	max_delay_samples = unit->param.max_delay * sample_rate/1000;
	echo_init(&unit->ct,  channel, sample_rate, unit->param.fc, max_delay_samples, unit->buf);
}

void AudioEffectEchoApply(EchoUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	uint32_t delay_samples = unit->param.delay;
	int32_t  direct_sound = unit->param.direct;
	if(unit->enable == FALSE)
	{
		return;
	}
	delay_samples = delay_samples * USER_EFFECT_SAMPLE_RATE/1000;
	echo_apply(&unit->ct, pcm_in, pcm_out, n, unit->param.attenuation, delay_samples, direct_sound);

}


EqFilterParams tempEqParam[10];
void AudioEffectEQInit(EQUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	uint32_t i, filter_count = 0;
	int16_t *eq_pp = NULL;

	if(unit->enable == FALSE)
	{
		return;
	}
	
	eq_pp = (int16_t *)unit->param.eq_params;
	memset(tempEqParam, 0x00, sizeof(tempEqParam));
	for(i=0; i<10; i++, eq_pp+=5)
	{
		if(eq_pp[0])
		{
			tempEqParam[filter_count].type = eq_pp[1];
			tempEqParam[filter_count].f0   = eq_pp[2];
			tempEqParam[filter_count].Q    = eq_pp[3];			
			tempEqParam[filter_count].sqrtA = sqrt(pow(10, (double)((int16_t)eq_pp[4] /256.0)/40.0)) * 1024;// sqrt(pow(10, (double)((int16_t)IICRegTab[REG_EQ_CTRL_INDEX+5+n*4] /256.0)/40.0)) * 1024; //(uint16_t)(pow(10, eq_pp[4]/256.0/40.0) * 1024 + 0.5);
			filter_count ++;
		}
	}
	unit->channel = channel;

	SwEqDisable(&unit->ct);
	SwEqRamClear(&unit->ct);
	SwEqInit(&unit->ct);
	SwEqStyleConfigure(&unit->ct, sample_rate, tempEqParam, filter_count, (uint32_t)(pow(10, unit->param.pregain/256.0/20.0)*65536 + 0.5));
	SwEqEnable(&unit->ct);
}



void AudioEffectEQApply(EQUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if(unit->enable == FALSE)
	{
		return;
	}
	if(1)
	{
		uint32_t PSSample = SW_EQ_PROC_FRAME_SAMPLES;
		uint32_t Cnt = n / PSSample;
		uint32_t RemainCnt = n - Cnt * PSSample;
		uint16_t iIdx;

		for(iIdx = 0; iIdx < Cnt; iIdx++)
		{
			SwEqApply(&unit->ct, &unit->w, (int16_t *)(pcm_in  + (PSSample * unit->channel) * iIdx),	SW_EQ_PROC_FRAME_SAMPLES, unit->channel);
			memcpy((void *)(pcm_out  + (PSSample * unit->channel) * iIdx), (void *)(pcm_in  + (PSSample * unit->channel) * iIdx), unit->channel*SW_EQ_PROC_FRAME_SAMPLES*2);
		}
		if(RemainCnt > 0)
		{
			SwEqApply(&unit->ct, &unit->w, (int16_t *)(pcm_in  + (PSSample * unit->channel) * Cnt), RemainCnt, unit->channel);
			memcpy((void *)(pcm_out  + (PSSample * unit->channel) * Cnt), (void *)(pcm_in  + (PSSample * unit->channel) * Cnt), unit->channel*RemainCnt*2);
		}
	}
}

void ClearADCandDACMemoryFifo(void)
{
	  AdcPmemWriteDis();
    AdcToPmem(USB_MODE, (uint16_t)(AUDIO_ADC_PMEM_ADDR), USER_ADC_DMA_FIFO);
    AdcPmemWriteEn();
	  PcmFifoClear();
}

uint32_t GetMemoryUsageInfor(void)
{
	return (DynamicAddr - XMEM_ADDR + USER_ADC_DMA_FIFO + USER_DAC_DMA_FIFO)/1024;
}

void CycleCntStart(void)
{
    DWT->CYCCNT = 0u;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t CycleCntEnd(uint32_t SampleCnt, uint32_t SampleRate)
{
    uint32_t CycleVal;
    uint32_t MpcsVal;
    
    CycleVal = DWT->CYCCNT;
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    
    MpcsVal = (uint64_t)CycleVal * (uint64_t)SampleRate / SampleCnt / 1000000;
    
    return MpcsVal; 
}

