/**
 **************************************************************************************
 * @file    enc_out_buff.c
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

#include <string.h>
#include "enc_out_buff.h"
#include "fat_file.h"

#define MIN(A, B)           ((A) < (B) ? (A) : (B))
#define	MAX(A, B)           ((A) > (B) ? (A) : (B))

static uint8_t* gEncoderOutBuffer;
static uint32_t gEncoderOutBufferCapacity;
static uint32_t gEncoderOutBufferPushPointer;
static uint32_t gEncoderOutBufferPopPointer;

//extern uint32_t FileWrite(const void* buffer, uint32_t size, uint32_t count, void* handle);

void EncoderOutBufferInit(uint8_t* buffer, uint32_t capacity)
{
    gEncoderOutBuffer            = buffer;
    gEncoderOutBufferCapacity    = capacity;
    gEncoderOutBufferPushPointer = 0;
    gEncoderOutBufferPopPointer  = 0;
}

uint32_t EncoderOutBufferPush(uint8_t* buffer, uint32_t length)
{    
    uint32_t RemainBytes;
    uint32_t PopPointer = gEncoderOutBufferPopPointer;
    
    if(gEncoderOutBufferPushPointer >= PopPointer)
    {
        RemainBytes = gEncoderOutBufferCapacity - gEncoderOutBufferPushPointer + gEncoderOutBufferPopPointer;
        if(RemainBytes >= length)
        {
            RemainBytes = gEncoderOutBufferCapacity - gEncoderOutBufferPushPointer;
            if(RemainBytes >= length)
            {
                memcpy(&gEncoderOutBuffer[gEncoderOutBufferPushPointer], buffer, length);
                gEncoderOutBufferPushPointer += length;
            }
            else
            {
                memcpy(&gEncoderOutBuffer[gEncoderOutBufferPushPointer], buffer, RemainBytes);
                gEncoderOutBufferPushPointer = length - RemainBytes;
                memcpy(&gEncoderOutBuffer[0], &buffer[RemainBytes], gEncoderOutBufferPushPointer);
            }
        }
        else
        {
            return 0;
        }    
    }
    else
    {
        RemainBytes = PopPointer - gEncoderOutBufferPushPointer;
        if(RemainBytes >= length)
        {
            memcpy(&gEncoderOutBuffer[gEncoderOutBufferPushPointer], buffer, length);
            gEncoderOutBufferPushPointer += length;
        }
        else
        {
            return 0;
        }
    }
    
    if(gEncoderOutBufferPushPointer >= gEncoderOutBufferCapacity)
    {
        gEncoderOutBufferPushPointer = 0;
    }
    
    return length;
}

uint32_t EncoderOutBufferPop(void* fp)
{
    uint32_t PopBytes;
    uint32_t PushPointer = gEncoderOutBufferPushPointer;

    if(PushPointer >= gEncoderOutBufferPopPointer)
    {
        PopBytes = PushPointer - gEncoderOutBufferPopPointer;
        FileWrite(&gEncoderOutBuffer[gEncoderOutBufferPopPointer], 1, PopBytes, fp);
        gEncoderOutBufferPopPointer += PopBytes;    
    }
    else
    {
        PopBytes = gEncoderOutBufferCapacity - gEncoderOutBufferPopPointer;
        FileWrite(&gEncoderOutBuffer[gEncoderOutBufferPopPointer], 1, PopBytes, fp);
        FileWrite(&gEncoderOutBuffer[0], 1, PushPointer, fp);
        PopBytes += PushPointer;
        gEncoderOutBufferPopPointer = PushPointer;
    }
    
    if(gEncoderOutBufferPopPointer >= gEncoderOutBufferCapacity)
    {
        gEncoderOutBufferPopPointer = 0;
    }
    
    return PopBytes;
}

uint32_t EncoderOutBufferPopByBlock(void* fp)
{
    uint32_t PopBytes;
    uint32_t PushPointer = gEncoderOutBufferPushPointer;

    if(PushPointer >= gEncoderOutBufferPopPointer)
    {
        PopBytes = ((PushPointer - gEncoderOutBufferPopPointer)>>9)<<9;
        if(PopBytes)
        {
            FileWrite(&gEncoderOutBuffer[gEncoderOutBufferPopPointer], 1, PopBytes, fp);
            gEncoderOutBufferPopPointer += PopBytes;
        }
    }
    else
    {
        PopBytes = ((gEncoderOutBufferCapacity - gEncoderOutBufferPopPointer + PushPointer)>>9)<<9;
        if(PopBytes)
        {
            uint32_t len = gEncoderOutBufferCapacity - gEncoderOutBufferPopPointer;
            len = MIN(len, PopBytes);
            if(len)
            {
                FileWrite(&gEncoderOutBuffer[gEncoderOutBufferPopPointer], 1, len, fp);
                gEncoderOutBufferPopPointer += len;
            }
            len = PopBytes-len;
            if(len)
            {
                FileWrite(&gEncoderOutBuffer[0], 1, len, fp);
                gEncoderOutBufferPopPointer = len;
            }
        }
    }
    
    if(gEncoderOutBufferPopPointer >= gEncoderOutBufferCapacity)
    {
        gEncoderOutBufferPopPointer = 0;
    }
    
    return PopBytes;
}
