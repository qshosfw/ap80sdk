/**
 **************************************************************************************
 * @file    ssap_protocol.h
 * @brief   SPI Stream Audio Player Protocol
 * 
 * @author  Aissen Li
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __SPI_PROTOCOL_H__
#define __SPI_PROTOCOL_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include <stdint.h>

typedef enum _SPIM_CMD
{
    SPIM_UNKOWN = 0,
    SPIM_READ,//param length
	SPIM_WRITE//Param length	
}SPIM_CMD;

typedef enum _SPIS_RESPOND
{
	SPIS_UNKONW = 0,
	SPIS_NO_DATA,//not enough data to be sent.  //param  actually data length
	SPIS_NO_ROOM,//not enouth room to receive data  //param  actually data room
	SPIS_OK
}SPIS_RESPOND;

#define SPI_SYNC_BYTE              'S'

typedef struct _SPICmdContext
{
    uint8_t  SyncWord;
    uint8_t  Message;
    uint16_t Param;
    uint16_t CrcValue;
}SpiMessContext;

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
