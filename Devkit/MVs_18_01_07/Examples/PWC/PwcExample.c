///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2014, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: PwcExample.c
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "type.h"
#include "clk.h"
#include "gpio.h"
#include "uart.h"
#include "cache.h"
#include "pwc.h"
#include "pwm.h"
#include "timeout.h"
#include "watchdog.h"

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

extern void DelayMs(unsigned int Ms);
bool PrintGetValFlg = FALSE;

#define INTERRUPT_MODE

int32_t main(void)
{   
    uint8_t Buf[10];
    
    ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating

	ClkPorRcToDpll(0);                                  //clock src is 32768hz OSC 
    CacheInit();    
	/*
	 * Now, uart has been initialized automatically by OS.
	 * if you want to customize debug uart setting(defaut 115200-8-N-1),
	 * you can invoke the function at any place you want(for example here).
	 */
	GpioFuartRxIoConfig(0);
	GpioFuartTxIoConfig(0);
	FuartInit(115200, 8, 0, 1);
    
    /*
     * Pwc module can capture pulse-width with 12MHZ clock.
     * Now,we take pwm pulse as input signal and measure it using PWC module
     * Config PWM module: Port: GPIOB6; Freq: 1khz, Duty: 50%
     */
    GpioSetRegOneBit(GPIO_B_OE, GPIOB6);
	PwmConfig(PWM_CH4_B6_B23, 12000, 6000);             
	PwmEnableChannel(PWM_CH4_B6_B23, PWM_IO_SEL0, PWM_MODE_OUT);
    

    GpioPwcIoConfig(0);                                 //GPIOA10
    
#ifdef INTERRUPT_MODE
    NVIC_EnableIRQ(PWC_IRQn);
#endif    
    
    APP_DBG("\n");
    APP_DBG("/==========================================================================\\\n");
    APP_DBG("|                   PWC TESTBENCH                                          |\n");
    APP_DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    APP_DBG("\\==========================================================================/\n");
    APP_DBG("\n\n");
    APP_DBG(" ==============================================\n");
    APP_DBG(" Please send num to sel pwc capture pulse mode\n");
    APP_DBG(" 1: posedge to posedge\n");
    APP_DBG(" 2: posedge to negedge\n");
    APP_DBG(" 3: negedge to posedge\n");
    APP_DBG(" 4: negedge to negedge\n");
    APP_DBG("\n");
    while(1)
    {
        if(-1 != FuartRecvByte(Buf))
        {
            switch(Buf[0])
            {
                case '1':
                    APP_DBG("\n posedge to posedge: \n");
                    PwcOverFlowClr();
                    PwcIntClr();
                    PwcModeSet(PWC_MODE_EDGE_1_1);
                    break;
                case '2':
                    APP_DBG("\n posedge to negedge: \n");
                    PwcOverFlowClr();
                    PwcIntClr();
                    PwcModeSet(PWC_MODE_EDGE_1_0);
                    break;
                case '3':
                    APP_DBG("\n negedge to posedge: \n");
                    PwcOverFlowClr();
                    PwcIntClr();
                    PwcModeSet(PWC_MODE_EDGE_0_1);
                    break;
                case '4':
                    APP_DBG("\n negedge to negedge: \n");
                    PwcOverFlowClr();
                    PwcIntClr();
                    PwcModeSet(PWC_MODE_EDGE_0_0);
                    break;
                default:
                    break;
            }
        }
        else
        {
#ifndef INTERRUPT_MODE  
            if(PwcGetIntStatus())
            {
                APP_DBG(" Query mode: Pulse width with 12M clock = %d\n",GetPulseValue());
                PwcIntClr();//clear done status
                PwcDeInit();//disable pwc module
            }         
#endif            
        }
        WdgFeed();
    }
}

void PwcInterrupt(void)
{
//    int bb=0;
//    int cc;
#ifdef INTERRUPT_MODE    
    APP_DBG(" Interrupt mode: Pulse width with 12M clock = %d\n",GetPulseValue());
#endif
    PwcIntClr(); 
    PwcOverFlowClr();
    PwcDeInit();
}
//
