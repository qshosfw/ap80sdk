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
//                 UART DEBUG��������        
//****************************************************************************************
	#define FUNC_DEBUG_EN
	#ifdef FUNC_DEBUG_EN
		#define FUNC_APP_DEBUG_EN
		#define FUNC_FS_DEBUG_EN
		#define FUART_RX_PORT  1	//rx port  0--A1��1--B6��2--C4��0xFF--NO USE
		#define FUART_TX_PORT  1	//tx port  0--A0��1--B7��2--C3��0xFF--NO USE
	#endif

//****************************************************************************************
//                 WATCH DOG��������        
//****************************************************************************************
	#define FUNC_WATCHDOG_EN			//WATCHDOG����

//****************************************************************************************
//                 �ϵ���书������        
//****************************************************************************************
	#define FUNC_BREAKPOINT_EN			// ϵͳ�ϵ���Ϣ����
	#ifdef FUNC_BREAKPOINT_EN
		#define FUNC_NVM_TO_FLASH_EN		// ������书�ܣ�֧�ֽ�NVM���ݱ��浽FLASH�У���ֹ���綪ʧ��
		#define FUNC_MATCH_PLAYER_BP		// ��ȡFSɨ����벥��ģʽ�ϵ���Ϣ��ƥ����ļ����ļ���ID��
	#endif
//****************************************************************************************
//                 POWER MONITOR��������        
//****************************************************************************************
// ������ܼ���(�����ڴ����ϵͳ)�Ĺ��ܺ��ѡ���
// ���ܼ��Ӱ�������ص�ѹ��⼰�͵�ѹ���ϵͳ��Ϊ�Լ����ָʾ��
// ��ص�ѹ��⣬��ָLDOIN����˵ĵ�ѹ��⹦��(���ϵͳһ�㶼�ǵ��ֱ�Ӹ�LDOIN�ܽŹ���)
// �ù��ܺ�򿪺�Ĭ�ϰ�����ص�ѹ��⹦�ܣ��йص�ص�ѹ���������ɶ�������������power_monitor.c�ļ�

//USE_POWERKEY_SLIDE_SWITCH ��USE_POWERKEY_SOFT_PUSH_BUTTON �����겻Ҫͬʱ����
	#define USE_POWERKEY_SLIDE_SWITCH     //for slide switch case ONLY
//	#define USE_POWERKEY_SOFT_PUSH_BUTTON //for soft push button case ONLY

	#define FUNC_POWER_MONITOR_EN
	
//		#define	OPTION_CHARGER_DETECT		//�򿪸ú궨�壬֧��GPIO������豸���빦��
			//�����˿�����
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
