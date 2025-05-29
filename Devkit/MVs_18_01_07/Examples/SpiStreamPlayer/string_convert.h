/**
 **************************************************************************************
 * @file    string_convert.h
 * @brief   String Convert API
 * 
 * @author  Aissen Li
 * @version V1.0.0
 * 
 * $Id: string_convert.h 3654 2013-07-12 10:48:53Z lance $
 * $Created: 2013-04-19 11:41:24$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __STRING_CONVERT_H__
#define __STRING_CONVERT_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include <stdint.h>

void StrUnicode2Gbk(uint16_t *str, uint32_t size);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__STRING_CONVERT_H__

