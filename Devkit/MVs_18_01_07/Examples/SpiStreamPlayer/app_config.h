///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: app_config.h
//  maintainer: Halley
///////////////////////////////////////////////////////////////////////////////
#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

//#include "type.h"
//#include "chip_info.h"

#define FUNC_CARD_EN						// SD�����Ź���	
#define	SD_PORT_NUM                 1		// SD���˿ڶ���
//SD_CLK����ΪӲ�����ţ�A3
#define CARD_DETECT_PORT_IN			GPIO_A_IN	
#define CARD_DETECT_PORT_OE			GPIO_A_OE	
#define CARD_DETECT_PORT_PU			GPIO_A_PU	
#define CARD_DETECT_PORT_PD			GPIO_A_PD	
#define CARD_DETECT_PORT_IE  		GPIO_A_IE
#define CARD_DETECT_BIT_MASK		(1 << 20)


#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)
#define	FS_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif/*_APP_CONFIG_H_*/
