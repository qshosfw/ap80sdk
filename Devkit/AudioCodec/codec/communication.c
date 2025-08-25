#include <stdint.h>
#include <math.h>
#include <string.h>
#include "drc.h"
#include "echo.h"
#include "expander.h"
#include "pitch_shifter.h"
#include "eq.h"
#include "blue_ns.h"
#include "blue_ns_core.h"
#include "user_effects_param.h"
#include "mcu_circular_buf.h"
#include "codec_effects.h"
#include "communication.h"
#include "uart.h"
#include "dac.h"

#if AUDIO_EFFECT_ONLINE_CONFIG_EN
MCU_CIRCULAR_CONTEXT gUartCt;
uint8_t  gUartFifo[AUDIO_EFFECT_ONLINE_CONFIG_FIFO];
uint8_t  gUartBuf[256];
uint8_t  gUartPreFlag;
uint16_t gUartReadLen;

void AudioEffectOnlineConfigInit(void)
{
	MCUCircular_Config(&gUartCt, gUartFifo, sizeof(gUartFifo));
	gUartReadLen = 0;
	gUartPreFlag = 0;
}

void AudioEffectFirmwareInfoProcess(void)
{
	uint8_t buf[12];
	
	buf[0]  = 0xA5;
	buf[1]  = 0x5A;
	buf[2]  = 0x00;
	buf[3]  = 0x07;
	buf[4]  = 0x50;
	buf[5]  = USER_EFFECT_MAJOR_VERSION;
	buf[6]  = USER_EFFECT_MINOR_VERSION;
	buf[7]  = USER_EFFECT_PATCH_VERSION;
	buf[8]  = 1;
	buf[9]  = 33;
	buf[10] = 0;
	buf[11] = 0x16;
	
	#ifdef FUART_PORT_EN
	FuartSend(buf, 12);
	#else
	BuartSend(buf, 12);	
	#endif
	
}

void AudioEffectSystemStatusProcess(void)
{
	uint8_t sendBuf[200];
	uint16_t usaMEM;
	uint16_t aveMCPS;
	uint16_t i;
	
	usaMEM = GetMemoryUsageInfor();
	aveMCPS = 0;
	for(i = 0; i < CPU_MCPS_BUF_SIZE; i++)
	{
		aveMCPS += CPUMcpsValue[i];
	}
	aveMCPS = aveMCPS/CPU_MCPS_BUF_SIZE;
		
	memset(sendBuf, 0, sizeof(sendBuf));
	sendBuf[0]  = 0xA5;
	sendBuf[1]  = 0x5A;
	sendBuf[2]  = 0x02;
	sendBuf[3]  = 0x0A;
	sendBuf[4]  = 0xFF;
	memcpy(&sendBuf[5], &usaMEM, 2);
	memcpy(&sendBuf[7], &aveMCPS, 2);
	sendBuf[9] = 0;
	sendBuf[10] = 96;
	sendBuf[11] = 0;
	sendBuf[12] = 128;
	sendBuf[13] = 0;
	sendBuf[14] = 0x16;
	
	#ifdef FUART_PORT_EN
	FuartSend(sendBuf, 15);
	#else
	BuartSend(sendBuf, 15);
	#endif
	
}

void AudioEffectListProcess(uint8_t *buf, uint32_t len)
{
	uint8_t listNo = buf[2];
	
	if(len != 1)//目前仅支持查询
		return;
	if(listNo == 0)//effect list
	{
		uint8_t sendBuf[200], i;
		
		memset(sendBuf, 0, sizeof(sendBuf));
		sendBuf[0]  = 0xA5;
		sendBuf[1]  = 0x5A;
		sendBuf[2]  = 0x80;
		sendBuf[3]  = 2+AUDIO_EFFECT_NODE_NUM*2;
		sendBuf[4]  = 0x00;
		sendBuf[5]  = AUDIO_EFFECT_NODE_NUM;
		for(i = 0; i < AUDIO_EFFECT_NODE_NUM; i++)
		{
			sendBuf[6+i*2] = user_effect_exec_table[i].effect_type;
		}
		sendBuf[6+AUDIO_EFFECT_NODE_NUM*2] = 0x16;
		
		#ifdef FUART_PORT_EN
		FuartSend(sendBuf, 7+AUDIO_EFFECT_NODE_NUM*2);
		#else
		BuartSend(sendBuf, 7+AUDIO_EFFECT_NODE_NUM*2);
		#endif
	}
	else
	{
		uint8_t sendBuf[200];
		memset(sendBuf, 0, sizeof(sendBuf));
		sendBuf[0]  = 0xA5;
		sendBuf[1]  = 0x5A;
		sendBuf[2]  = 0x80;
		sendBuf[3]  = 1+strlen(user_effect_exec_table[listNo-1].effect_name);
		sendBuf[4]  = listNo;
		memcpy(&sendBuf[5], user_effect_exec_table[listNo-1].effect_name, strlen(user_effect_exec_table[listNo-1].effect_name));
		sendBuf[5+strlen(user_effect_exec_table[listNo-1].effect_name)] = 0x16;
		
		#ifdef FUART_PORT_EN
		FuartSend(sendBuf, 6+strlen(user_effect_exec_table[listNo-1].effect_name));
		#else
		BuartSend(sendBuf, 6+strlen(user_effect_exec_table[listNo-1].effect_name));
		#endif
	}
}

void AudioEffectParameterProcess(uint8_t *buf, uint32_t len)
{
	uint8_t addrIdx = buf[0] - 0x81;
	
	if(addrIdx >= AUDIO_EFFECT_NODE_NUM)
		return;
	
	if(len == 0 ) //查询模式
	{
		uint8_t sendBuf[200];
		
		memset(sendBuf, 0, sizeof(sendBuf));
		sendBuf[0]  = 0xA5;
		sendBuf[1]  = 0x5A;
		sendBuf[2]  = buf[0];
		sendBuf[3]  = 1+2+gEffectNodeList.EffectNode[addrIdx].ParamCnt*2;
		sendBuf[4]  = 0xFF;
		sendBuf[5]  = gEffectNodeList.EffectNode[addrIdx].Enable;
		if(gEffectNodeList.EffectNode[addrIdx].Enable)
			memcpy(&sendBuf[7], gEffectNodeList.EffectNode[addrIdx].EffectUnit, gEffectNodeList.EffectNode[addrIdx].ParamCnt*2);
		else
			AudioEffectGetIndivParameters(addrIdx, &sendBuf[7]);
		sendBuf[7+gEffectNodeList.EffectNode[addrIdx].ParamCnt*2] = 0x16;
		
		#ifdef FUART_PORT_EN
		FuartSend(sendBuf, 7+gEffectNodeList.EffectNode[addrIdx].ParamCnt*2+1);
		#else
		BuartSend(sendBuf, 7+gEffectNodeList.EffectNode[addrIdx].ParamCnt*2+1);
		#endif
	}
	else if(len > 0 && buf[2] != 0xFF)//设置
	{
		int16_t setData[10];
		
		memcpy(setData, &buf[3], len-1);
		
		if(buf[2] == 0x00)//enable or diable index
		{
			if(gEffectNodeList.EffectNode[addrIdx].Enable != setData[0])
			{
				//if(setData[0] == 0)
					AudioEffectSaveAllParametersToMemory();
				gEffectNodeList.EffectNode[addrIdx].Enable = setData[0];
				AudioEffectSaveIndivEnValueToMemory(addrIdx);
				
				DacDigitalMuteSet(1, 1);
				
				AudioEffectDeInit();
				AudioEffectInit(USER_EFFECT_CHANNEL);
				
				ClearADCandDACMemoryFifo();
				DacDigitalMuteSet(0, 0);
			}
		}
		else
		{
			if(gEffectNodeList.EffectNode[addrIdx].Enable)
			{
				if(gEffectNodeList.EffectNode[addrIdx].EffectType == DRC)//drc几个参数比较特殊，单独拎出来
				{
					uint8_t offerSet[10] = {0, 0, 1, 2, 4, 7, 10, 13, 16, 17};
					if(buf[2] >= 10)  return;
					memcpy((int16_t *)gEffectNodeList.EffectNode[addrIdx].EffectUnit + offerSet[buf[2]], setData, len-1);
					gEffectNodeList.EffectNode[addrIdx].FuncAudioEffectInit(gEffectNodeList.EffectNode[addrIdx].EffectUnit, USER_EFFECT_CHANNEL, USER_EFFECT_SAMPLE_RATE);
				}
				else
				{
					memcpy((int16_t *)gEffectNodeList.EffectNode[addrIdx].EffectUnit + buf[2]-1, setData, 2);
					gEffectNodeList.EffectNode[addrIdx].FuncAudioEffectInit(gEffectNodeList.EffectNode[addrIdx].EffectUnit, USER_EFFECT_CHANNEL, USER_EFFECT_SAMPLE_RATE);
				}
				
			}
		}
	}
	else if(len > 0 && buf[2] == 0xFF)
	{
		memcpy((int16_t *)gEffectNodeList.EffectNode[addrIdx].EffectUnit, &buf[5], gEffectNodeList.EffectNode[addrIdx].ParamCnt*2);
		if(buf[3] == gEffectNodeList.EffectNode[addrIdx].Enable && gEffectNodeList.EffectNode[addrIdx].Enable)
		{
			gEffectNodeList.EffectNode[addrIdx].FuncAudioEffectInit(gEffectNodeList.EffectNode[addrIdx].EffectUnit, USER_EFFECT_CHANNEL, USER_EFFECT_SAMPLE_RATE);
		}
		else if(buf[3] != gEffectNodeList.EffectNode[addrIdx].Enable)
		{
			if(buf[3] == 0)
				AudioEffectSaveAllParametersToMemory();
			gEffectNodeList.EffectNode[addrIdx].Enable = buf[3];
			AudioEffectSaveIndivEnValueToMemory(addrIdx);
			
			DacDigitalMuteSet(1, 1);
			
			AudioEffectDeInit();
			AudioEffectInit(USER_EFFECT_CHANNEL);
			
			ClearADCandDACMemoryFifo();
			DacDigitalMuteSet(0, 0);			
		}
	}
}

void AudioEffectSaveToFlash(void)
{
	AudioEffectSaveParamToFlash();
}

void AudioEffectOnlinePacketProcess(uint8_t *buf, uint32_t len)
{
	switch(buf[0])
	{
		case 0x00:
			AudioEffectFirmwareInfoProcess();
			break;
			
		case 0x02:
			AudioEffectSystemStatusProcess();
			break;
			
		case 0x80:
			AudioEffectListProcess(buf, len);
			break;
			
		case 0xFD:
			AudioEffectSaveToFlash();
			break;
			
		default:
			if(buf[0] > 0x80 && buf[0] < 0xfc)
			{
				AudioEffectParameterProcess(buf, len);
			}
			break;
	}
}

void AudioEffectOnlineDataProcess(void)
{
	uint16_t i, len;
	uint16_t ipacket_len;
	uint8_t address;
	uint8_t clearFlag = 1;
	
	len = MCUCircular_GetDataLen(&gUartCt);
	if(len >= 1)
	{
		if(len + gUartReadLen > 256)
		{
			len = 256 - gUartReadLen;
		}
		MCUCircular_GetData(&gUartCt, &gUartBuf[gUartReadLen], len);
		gUartReadLen = gUartReadLen + len;
		
		if(gUartReadLen >= 2)
		{
			for(i = 0; i < gUartReadLen-1; i++)
			{
				if((gUartBuf[i] == 0xA5) && (gUartBuf[i+1] == 0x5A))
				{
					ipacket_len = 0;
					if(i + 3 > gUartReadLen-1)
					{
						gUartReadLen = gUartReadLen - i;
						memcpy(&gUartBuf[0], &gUartBuf[i], gUartReadLen);
						clearFlag = 0;
						break;
					}
					
					ipacket_len = gUartBuf[i+3];
					if(i + 4 + ipacket_len > gUartReadLen-1)
					{
						gUartReadLen = gUartReadLen - i;
						memcpy(&gUartBuf[0], &gUartBuf[i], gUartReadLen);
						clearFlag = 0;
						break;
					}

					if(gUartBuf[i+4+ipacket_len] == 0x16)
					{
						address = gUartBuf[i+2];
						AudioEffectOnlinePacketProcess(&gUartBuf[i+2], gUartBuf[i+3]);
					}
				}
				else
				{
					if(i == gUartReadLen-2 && gUartBuf[i+1] == 0xA5)
					{
						memcpy(&gUartBuf[0], &gUartBuf[i+1], 1);
						gUartReadLen = 1;
						clearFlag = 0;
					}
				}
				
			}
			if(clearFlag) gUartReadLen = 0;
		}
	}
}

void AudioEffectOnlineDataFeed(uint8_t data)
{
		MCUCircular_PutData(&gUartCt, &data, 1);
}

void AudioEffectOnlineDatasFeed(uint8_t *data, int32_t len)
{
		MCUCircular_PutData(&gUartCt, data, len);
}

#endif