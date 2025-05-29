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

//****************************************************************************************
//                 UART DEBUG功能配置        
//****************************************************************************************
	#define FUNC_DEBUG_EN
	#ifdef FUNC_DEBUG_EN
		#define FUNC_APP_DEBUG_EN
		#define FUNC_FS_DEBUG_EN
		#define FUART_RX_PORT  1	//rx port  0--A1，1--B6，2--C4，0xFF--NO USE
		#define FUART_TX_PORT  1	//tx port  0--A0，1--B7，2--C3，0xFF--NO USE
	#endif

//****************************************************************************************
//                 WATCH DOG功能配置        
//****************************************************************************************
	#define FUNC_WATCHDOG_EN			//WATCHDOG功能

#include "debug.h"

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif/*_APP_CONFIG_H_*/
