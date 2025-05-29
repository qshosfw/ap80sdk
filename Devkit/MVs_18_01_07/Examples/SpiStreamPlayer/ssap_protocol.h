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

#ifndef __SSAP_PROTOCOL_H__
#define __SSAP_PROTOCOL_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include <stdint.h>

#define SSAP_SEND_DATA_LENGTH       (512)
#define SSAP_SYNC_WORD_LENGTH       (1)
#define SSAP_CRC_VALUE_LENGTH       (2)
#define SSAP_RESEND_IF_CRC_ERROR    (1)

/**
 * @brief SPI Stream Audio Player Protocol Command Set
 *        1. For Reduced Instruction Set, see the context SSAPCmdContext when SSAP_SYNC_WORD_LENGTH equal 1.
 *        2. For Complex Instruction Set, see the context SSAPCmdContext when SSAP_SYNC_WORD_LENGTH equal 4.
 *        
 * @Note  For those commands with few content (less than 2 bytes) or without content, the content data
 *        just fill in the SSPPCmdContext.Content field.
 *        For those commands with large content (more than 2 bytes), such as command SSPP_CMD_DATA, first
 *        fill in the SSPPCmdContext.Content field with the length of content data, and then send a content
 *        data packet with the format as follow:
 *        SYNC_WORD + content_data + CRC
 */
typedef enum _SSPP_CMD
{
    SSAP_CMD_UNKOWN = 0,
    SSAP_CMD_START,
    SSAP_CMD_PAUSE,
    SSAP_CMD_RESUME,
    SSAP_CMD_STOP,
    SSAP_CMD_MUTE,
    SSAP_CMD_UNMUTE,
    SSAP_CMD_DATA,
    SSAP_CMD_VOL_SET,
    SSAP_CMD_VOL_ADD,
    SSAP_CMD_VOL_SUB,
}SSPP_CMD;

typedef enum __SSAP_CMD_RESPONSE
{
    SSAP_CMD_RESP_UNKOWN = 0,
    SSAP_CMD_RESP_OKCMD,
    SSAP_CMD_RESP_ERCMD,
    SSAP_CMD_RESP_OKSEND,
    SSAP_CMD_RESP_RESEND,
    SSAP_CMD_RESP_NEXTSEND,
}SSAP_CMD_RESPONSE;

#if   (SSAP_SYNC_WORD_LENGTH == 1)

#define SSAP_SYNC_BYTE              'S'
#define SSAP_SYNC_WORD              'S'    //SPI Stream Player Protocol SyncWord
#define SSAP_SYNC_WORD_BE           'S'
#define SSAP_SYNC_WORD_LE           'S'

typedef struct _SSAPCmdContext
{
    uint8_t  SyncWord;
    uint8_t  Command;
    uint16_t Content;
    uint16_t CrcValue;
}SSAPCmdContext;

typedef struct _SSAPCmdResponseContext
{
    uint8_t  SyncWord;
    uint8_t  Command;
    uint16_t Response;
    uint16_t CrcValue;
}SSAPCmdResponseContext;

#elif (SSAP_SYNC_WORD_LENGTH == 4)

#define SSAP_SYNC_BYTE              'S'
#define SSAP_SYNC_WORD              'SSAP' //SPI Stream Player Protocol SyncWord
#define SSAP_SYNC_WORD_BE           'SSAP'
#define SSAP_SYNC_WORD_LE           'PASS'

typedef struct _SSAPCmdContext
{
    uint32_t SyncWord;
    uint16_t Command;
    uint16_t Content;
    uint16_t Reserved;
    uint16_t CrcValue;
}SSAPCmdContext;

typedef struct _SSAPCmdResponseContext
{
    uint32_t SyncWord;
    uint16_t Command;
    uint16_t Response;
    uint16_t Reserved;
    uint16_t CrcValue;
}SSAPCmdResponseContext;

#endif

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__SSAP_PROTOCOL_H__
