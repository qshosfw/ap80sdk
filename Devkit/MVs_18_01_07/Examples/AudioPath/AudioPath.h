///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2014, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: AudioPath.h
///////////////////////////////////////////////////////////////////////////////
#ifndef __AUDIOPATH_H__
#define __AUDIOPATH_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "chip_info.h"
#define ADC_FIFO_ADDR  PMEM_ADDR
#define ADC_FIFO_LEN   (4 * 1024)

#define I2S_FIFO_ADDR  (ADC_FIFO_ADDR + ADC_FIFO_LEN)
#define I2S_FIFO_LEN   (4 * 1024)

#define PCM_FIFO_ADDR  (I2S_FIFO_ADDR + I2S_FIFO_LEN)
#define PCM_FIFO_LEN   (10 * 1024)

#define DEC_MEM_ADDR    VMEM_ADDR
#define DEC_MEM_MAX_SIZE (20 * 1024)

typedef enum _STATUS_MCU
{
    STATUS_IDLE = 0,
    STATUS_ADCIN_BUSY,
    STATUS_I2SIN_BUSY,
}STATUS_MCU;

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
//
