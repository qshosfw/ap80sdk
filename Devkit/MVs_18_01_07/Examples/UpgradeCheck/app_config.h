///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: app_config.h
///////////////////////////////////////////////////////////////////////////////
#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "delay.h"
#include "timeout.h"

#define FUNC_CARD_EN

//****************************************************************************************
//                 UART DEBUGπ¶ƒ‹≈‰÷√        
//****************************************************************************************
#ifndef FUNC_DEBUG_EN
#define FUNC_DEBUG_EN
#endif
#ifdef FUNC_DEBUG_EN
	#define FUNC_APP_DEBUG_EN
	#define FUNC_FS_DEBUG_EN
	#define FUART_RX_PORT  0xFF	//rx port  0--A1£¨1--B6£¨2--C4£¨0xFF--NO USE
	#define FUART_TX_PORT  1	//tx port  0--A0£¨1--B7£¨2--C3£¨0xFF--NO USE
#endif

#include "debug.h"

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif/*_APP_CONFIG_H_*/
