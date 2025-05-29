/**
 **************************************************************************************
 * @file    enc_out_buff.h
 * @brief   encoder out buffer manage api
 * 
 * @author  Aissen Li
 * @version V1.0.0
 * 
 * $Created: 2013-09-13 10:02:11$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __ENC_OUT_BUFF_H__
#define __ENC_OUT_BUFF_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include <stdint.h>

void EncoderOutBufferInit(uint8_t* buffer, uint32_t capacity);

uint32_t EncoderOutBufferPush(uint8_t* buffer, uint32_t length);

uint32_t EncoderOutBufferPop(void* fp);

uint32_t EncoderOutBufferPopByBlock(void* fp);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif

