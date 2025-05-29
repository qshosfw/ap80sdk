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
#include "pwm.h"
#include "watchdog.h"
#include "timer.h"

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

// �ٶ�PWM1, PWM2�����Ƶ����һ����
#define KHZ							1000

#define	PWM_FREQ					(10*KHZ) 	// PWM���Ƶ��
#define PWM_CIRCLE_DIVISOR			10			// PWMƫ�����ӣ�����ÿһ�����ڷֳ�divisor�ݣ�PWM��ʼλ�õ�ƫ�������ڴ�ֵ

#define PWM1_OFFSET					0			// PWM1 ��ʼλ��ƫ�Ƶ�λ
#define PWM2_OFFSET					5			// PWM2 ��ʵλ��ƫ�Ƶ�λ

#define PWM1_HIGH_LEVEL_PERCENT		50			// PWM1 ռ�ձ� %
#define PWM2_HIGH_LEVEL_PERCENT		40			// PWM2 ռ�ձ� %

// ����Ϊ�򻯱��ʽ����ĺ꣬�������޸�
#define PWM_FREQ_DIV				(12*1000*1000/PWM_FREQ)		// ����
#define PWM_CIRCLE_TIME				((1000*1000)/PWM_FREQ)		// һ������ʱ�䵥λus
#define TIMER_VALUE					(PWM_CIRCLE_TIME/PWM_CIRCLE_DIVISOR)	// timer�ж�ʱ��

uint8_t start_point = 0;
__attribute__((section(".driver.isr")))
void Timer1Interrupt(void)
{
	Timer1IntClr();
	if(start_point > PWM_CIRCLE_DIVISOR)
		return;
	
	if(start_point == PWM1_OFFSET)
	{
		if(start_point == 100)	// �˴���ʵ�����壬������Ϊ�˱�֤����timer��ִ��pwm1��ʱ�������timer��ִ��pwm2��ʱ����ͬ������ִ���������ж���䣩
		{
			start_point = PWM1_OFFSET;
		}
		// start PWM1 output
		PwmDisableChannel(PWM_CH6_B8_B21);	
		PwmConfig(PWM_CH6_B8_B21, PWM_FREQ_DIV, ((PWM_FREQ_DIV*PWM1_HIGH_LEVEL_PERCENT)/100));	//50%
		PwmEnableChannel(PWM_CH6_B8_B21, PWM_IO_SEL0, PWM_MODE_OUT);
	}	
	
	if(start_point == PWM2_OFFSET)
	{
		// start PWM2 output
		PwmDisableChannel(PWM_CH7_B9_B20);
		PwmConfig(PWM_CH7_B9_B20, PWM_FREQ_DIV, ((PWM_FREQ_DIV*PWM2_HIGH_LEVEL_PERCENT)/100));	// 40%
		PwmEnableChannel(PWM_CH7_B9_B20, PWM_IO_SEL0, PWM_MODE_OUT);
	}

	start_point++;
}

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
	GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);
	FuartInit(115200, 8, 0, 1);
    
    /* Config Gpiob6 used for pwm pin */
//    GpioClrRegOneBit(GPIO_B_IE, GPIOB8);
//    GpioSetRegOneBit(GPIO_B_OE, GPIOB8); 
    
	
    APP_DBG("\n");
    APP_DBG("/==========================================================================\\\n");
    APP_DBG("|                   PWM TESTBENCH                                          |\n");
    APP_DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    APP_DBG("\\==========================================================================/\n");
    APP_DBG("\n\n");
    APP_DBG(" ==============================================\n");
    APP_DBG(" Please send num to sel pwm mode\n");
    APP_DBG(" 1: Pwm mode voltage output\n");
    APP_DBG(" 2: Pwm mode current pulldown1 1.7mA\n");
    APP_DBG(" 3: Pwm mode current pulldown2 2.4mA\n");
    APP_DBG(" 4: Pwm mode current pulldown1+pulldown2 4.1mA\n");
    APP_DBG(" 5: Synchronization output PWM1 & PWM2\n");
    APP_DBG("\n");

    while(1)
    {
        if(-1 != FuartRecvByte(Buf))
        {
            switch(Buf[0])
            {
                case '1':
                    APP_DBG("\n Voltage output, Freq: 1MHZ, Duty: 0.5\n");
                    PwmDisableChannel(PWM_CH6_B8_B21);
                    PwmConfig(PWM_CH6_B8_B21, 12, 6);
                    PwmEnableChannel(PWM_CH6_B8_B21, PWM_IO_SEL0, PWM_MODE_OUT);
                    break;
                
                case '2':
                    APP_DBG("\n Current pulldown1 1.7mA, Freq: 10KHZ, Duty: 0.25\n");
                    PwmDisableChannel(PWM_CH6_B8_B21);
                    PwmConfig(PWM_CH6_B8_B21, 1200, 300);
                    PwmEnableChannel(PWM_CH6_B8_B21, PWM_IO_SEL0, PWM_MODE_PD1);                
                    break;
                
                case '3':
                    APP_DBG("\n Current pulldown2 2.4mA, Freq: 1KHZ, Duty: 0.75\n");
                    PwmDisableChannel(PWM_CH6_B8_B21);
                    PwmConfig(PWM_CH6_B8_B21, 12000, 9000);
                    PwmEnableChannel(PWM_CH6_B8_B21, PWM_IO_SEL0, PWM_MODE_PD2);
                    break;
                
                case '4':
                    APP_DBG("\n Current pulldown1+pulldown2 4.1mA, Freq: 500HZ, Duty: 0.5\n");
                    PwmDisableChannel(PWM_CH6_B8_B21);
                    PwmConfig(PWM_CH6_B8_B21, 24000, 12000);
                    PwmEnableChannel(PWM_CH6_B8_B21, PWM_IO_SEL0, PWM_MODE_PD12);
                    break;
                
				case '5':
					APP_DBG("\n Output PWM1 & PWM2 by Timer\n");
					NVIC_DisableIRQ(TMR1_IRQn);
					start_point = 0;
					NVIC_EnableIRQ(TMR1_IRQn);
					Timer1Set(TIMER_VALUE);
					break;
                default:
                   break;
            }
        }
        WdgFeed();
    }
}


//
