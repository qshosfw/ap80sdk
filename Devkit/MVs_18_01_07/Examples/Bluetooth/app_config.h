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
//#include "debug.h"
	
#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)
	
//****************************************************************************************
//                 ¿∂—¿≈‰÷√
//****************************************************************************************
#define  BT_DEVICE_TYPE                 BTUartDeviceMTK662X

#define  BT_LDOEN_RST_GPIO_PORT         GPIO_PORT_B
#define  BT_LDOEN_RST_GPIO_PIN          23//8//23

#define  BUART_RX_FIFO_SIZE             (1024*6)
#define  BUART_TX_FIFO_SIZE             (1024)

#define  BT_STACK_MEM_SIZE_WITHOUT_SPP  (25800)
#define  BT_STACK_MEM_SIZE_WITH_SPP     (25800 + 5608)

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif/*_APP_CONFIG_H_*/
