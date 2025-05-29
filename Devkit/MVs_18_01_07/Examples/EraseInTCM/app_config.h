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
#include "msgq.h"
#include "timer_api.h"
#include "timeout.h"
#include "delay.h"
#include "chip_info.h"


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

//****************************************************************************************
//                 断点记忆功能配置        
//****************************************************************************************
	#define FUNC_BREAKPOINT_EN			// 系统断点信息管理
	#ifdef FUNC_BREAKPOINT_EN
		#define FUNC_NVM_TO_FLASH_EN		// 掉电记忆功能（支持将NVM数据保存到FLASH中，防止掉电丢失）
		#define FUNC_MATCH_PLAYER_BP		// 获取FS扫描后与播放模式断点信息相匹配的文件、文件夹ID号
	#endif
//****************************************************************************************
//                 POWER MONITOR功能配置        
//****************************************************************************************
// 定义电能监视(适用于带电池系统)的功能宏和选项宏
// 电能监视包括，电池电压检测及低电压后的系统行为以及充电指示等
// 电池电压检测，是指LDOIN输入端的电压检测功能(电池系统一般都是电池直接给LDOIN管脚供电)
// 该功能宏打开后，默认包含电池电压检测功能，有关电池电压检测的其它可定义参数，请详见power_monitor.c文件

//USE_POWERKEY_SLIDE_SWITCH 和USE_POWERKEY_SOFT_PUSH_BUTTON 两个宏不要同时定义
	#define USE_POWERKEY_SLIDE_SWITCH     //for slide switch case ONLY
//	#define USE_POWERKEY_SOFT_PUSH_BUTTON //for soft push button case ONLY

	#define FUNC_POWER_MONITOR_EN
	
//		#define	OPTION_CHARGER_DETECT		//打开该宏定义，支持GPIO检测充电设备插入功能
			//充电检测端口设置
			#define CHARGE_DETECT_PORT_PU			GPIO_E_PU
			#define CHARGE_DETECT_PORT_PD			GPIO_E_PD
			#define CHARGE_DETECT_PORT_IN			GPIO_E_IN
			#define CHARGE_DETECT_PORT_IE			GPIO_E_IE
			#define CHARGE_DETECT_PORT_OE			GPIO_E_OE
			#define CHARGE_DETECT_BIT				(1 << 3)


#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)
#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif/*_APP_CONFIG_H_*/
