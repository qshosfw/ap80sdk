#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__
#include "type.h"

#if AUDIO_EFFECT_ONLINE_CONFIG_EN

void AudioEffectOnlineConfigInit(void);
void AudioEffectOnlineDataProcess(void);
void AudioEffectOnlineDataFeed(uint8_t);
void AudioEffectOnlineDatasFeed(uint8_t *, int32_t);

#endif

#endif