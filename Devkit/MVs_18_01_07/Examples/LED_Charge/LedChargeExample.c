///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2014, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: Led_Charge.c
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include "type.h"
#include "gpio.h"
#include "clk.h"
#include "ledcharger.h"
#include "watchdog.h"
#include "cache.h"
#include "uart.h"
//#define CHARGE_DONE_DARK

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

const uint32_t Buf[512] = {0};

int main(void)
{    
    ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
    ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating

    ClkPorRcToDpll(0);                                  //clock src is 32768hz OSC       
    CacheInit();
	memset((uint8_t*)Buf, 0, sizeof(Buf)); //注意：不要删除该代码，用于凑足4K size code size
	/*
	 * Now, uart has been initialized automatically by OS.
	 * if you want to customize debug uart setting(defaut 115200-8-N-1),
	 * you can invoke the function at any place you want(for example here).
	 */
	GpioFuartRxIoConfig(0);
	GpioFuartTxIoConfig(0);
	FuartInit(115200, 8, 0, 1);
	
	APP_DBG("\n");
    APP_DBG("/==========================================================================\\\n");
    APP_DBG("|                   charge  led example                                    |\n");
    APP_DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    APP_DBG("\\==========================================================================/\n");
    APP_DBG("\n\n");
    APP_DBG("****************************************************************\n");
	APP_DBG(" Pulse LedWidth:(9+1)*62.5ms = 625ms\n");
	APP_DBG(" #ifdef CHARGE_DONE_DARK\n");
	APP_DBG(" \tLedPolarity: 1-- means the sign of charged up is 0(dark)s\n");
	APP_DBG(" #else\n");
	APP_DBG(" \tLedPolarity: 0-- means the sign of charged up is 1(light)s\n");
	APP_DBG("****************************************************************\n");
	APP_DBG("\n");
	
	
    //Set 1.7mA to GPIOA0
    GpioA0SetIcs(ICS_1MA7);
    //Pulse LedWidth:(9+1)*62.5ms = 625ms
    //LedPolarity: 1-- means the sign of charged up is 0(dark)s
#ifdef CHARGE_DONE_DARK
    ChargerSetMode(9, 1);
#else
    ChargerSetMode(9, 0);
#endif
    while(1)
    {
       WdgFeed(); 
    }
}
