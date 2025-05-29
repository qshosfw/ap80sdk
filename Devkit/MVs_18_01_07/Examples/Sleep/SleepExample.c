///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2014, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: SleepExample.c
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "type.h"
#include "clk.h"
#include "gpio.h"
#include "uart.h"
#include "cache.h"
#include "timeout.h"
#include "watchdog.h"  
#include "dac.h"
#include "audio_adc.h"
#include "adc.h"

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

__attribute__((section(".driver.isr"))) void GpioInterrupt(void)
{   
    GpioIntClr(GPIO_A_INT, GPIOA10);
}

void GpioInterruptInit(void)
{
    uint32_t Port = GPIOA10;

	GpioSetRegOneBit(GPIO_A_IE, Port);
	GpioClrRegOneBit(GPIO_A_OE, Port);  
	GpioSetRegOneBit(GPIO_A_PU, Port);
	GpioClrRegOneBit(GPIO_A_PD, Port); 
	
	//enable interrupt
	GpioIntEn(GPIO_A_INT,Port, GPIO_NEG_EDGE_TRIGGER);

	//enable gpio irqc
	NVIC_EnableIRQ(GPIO_IRQn);	
}

void SysDeviceIoInit(void)
{	
#if (defined(FUNC_ADC_KEY_EN) || defined(FUNC_IR_KEY_EN) || defined(FUNC_CODING_KEY_EN))
	KeyInit();
#endif
}

void SysDeviceIoDeInit(void)
{
//ע:ִ�����´�����Խ��͹��ģ�����ʹ��O18������Э��ջ���벻Ҫ
//�ı�����ģ�����GPIO״̬������ᵼ�»��Ѻ�����������������

#ifndef FUNC_BT_EN
	GpioSetRegBits(GPIO_A_IE, 0xFFFFFFFF);
	GpioClrRegBits(GPIO_A_OE, 0xFFFFFFFF);
	GpioSetRegBits(GPIO_A_PU, 0xFFFFFFFF);
	GpioClrRegBits(GPIO_A_PD, 0xFFFFFFFF);	

	GpioSetRegBits(GPIO_B_IE, 0xFFFFFFEF);
	GpioClrRegBits(GPIO_B_OE, 0xFFFFFFFF);
	GpioSetRegBits(GPIO_B_PU, 0xFFFFFFEF);
	GpioClrRegBits(GPIO_B_PD, 0xFFFFFFFF);
	
	GpioSetRegBits(GPIO_C_IE, 0x7FFE);
	GpioClrRegBits(GPIO_C_OE, 0x7FFF);
	GpioSetRegBits(GPIO_C_PU, 0x7FFE);
	GpioClrRegBits(GPIO_C_PD, 0x7FFF);
#endif
}

uint32_t GetIRQSource = 0;
extern void SysPowerDownOtp(void);

void SysGotoSleepCfg(void)
{
	uint8_t i;
	
	APP_DBG("\r\n           go to sleep   \r\n");
	
	//ע: ����Sleepǰ��Ҫ�Ͽ���������(�������FastDisableBT())�����Խ��͹���	
#ifdef FUNC_BT_EN
	WaitMs(100);	//�ȴ������Ͽ�
#endif

#ifdef FUNC_WATCHDOG_EN
	WdgDis();
#endif
	SysDeviceIoDeInit();

  	//��Ƶͨ���رմ�����룬������Ҫ����ʹ��
	CodecDacChannelSel(0);
	CodecAdcChannelSel(0);
	CodecDacMuteSet(TRUE, TRUE);
	CodecAdcAnaDeInit();
	CodecDacDeinit(TRUE);

	// sar ADC close
	SarAdcPowerDown();

	// get intflag
	GetIRQSource = 0;
	for(i = 0; i < 16; i++)
	{
		GetIRQSource <<= 1;
		GetIRQSource |= NVIC_GetIRQCtlStat((IRQn_Type)i);
	}
	for(i = 0; i < 16; i++)
	{
		NVIC_DisableIRQ((IRQn_Type)i);
	}
		
	// close OTP power
	SysPowerDownOtp();
	ClkSwitchDpllToRc();	
	ClkDpllClose();
	//ClkModuleDivSet(256);//16); 	//ע: �˴����Ը��ݹ�������ʹ�õ���
	ClkModuleDis((CLK_MODULE_SWITCH)(ALL_MODULE_CLK_SWITCH & (~(FSHC_CLK_EN))));
	ClkModuleGateDis(GPIO_CLK_GATE_EN);  	
	
	// close sys init
	//systick_ioctl(3, 0); //��ֹsystick(ʹ��OS����¿���ʹ�ô˴�)
	
	// wakeup souce setting 
	GpioInterruptInit();
}

// wakeup from sleep mode  
void SysWakeUpCfg(void)
{
	int8_t i;

	ClkModuleDivSet(1);	
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);
	ClkPorRcToDpll(0);	
	
	//systick_ioctl(3, 1); //ʹ��systick (ʹ��OS����¿���ʹ�ô˴�)	
	
	for(i = 15; i >= 0; i--)
	{
		if((GetIRQSource & 0x01) != 0)
		{
			NVIC_EnableIRQ((IRQn_Type)i);
		}
		GetIRQSource >>= 1;
	}	
	
	SysDeviceIoInit();
}

//Wakeup init device
void SysWakeUpInit(void)
{	
	APP_DBG("\r\n           sys wakeup init\r\n");	
	//��Ƶͨ���ָ�������룬������Ҫ����ʹ��
	//AudioAnaInit();	//ģ��ͨ����ʼ��
	//DacVolumeSet(4000, 4000);
	//AdcVolumeSet(4000, 4000);
	//MixerInit((void*)PCM_FIFO_ADDR, PCM_FIFO_LEN);
	//AudioOutputInit();//���ͨ����ʼ��	
	
#ifdef FUNC_WATCHDOG_EN
	WdgEn();
#endif
}	

uint32_t delayCount = 0;
uint32_t count = 0;
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
	       
    APP_DBG("\n");
    APP_DBG("/==========================================================================\\\n");
    APP_DBG("|                   Sleep TESTBENCH                                        |\n");
    APP_DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    APP_DBG("\\==========================================================================/\n");
    APP_DBG("\n");
    
    APP_DBG("uart tx:GPIOB7\n");
    APP_DBG("uart rx:GPIOB6\n");
    APP_DBG("you can send 1 to o18 to sleep the system,the system will wake up when there is an interrupt of GPIOA10(negedge trigger)\n");
    
    while(1)
    {
        if(-1 != FuartRecvByte(Buf))
        {
            switch(Buf[0])
            {
                case '1':
                    WdgDis();
                    SysGotoSleepCfg();	
					__wfi();	//�˴�����Sleep״̬
					SysWakeUpCfg();				
					//WaitMs(500); //�ȴ�ң�ذ����Ķ�ȡ����ʱ����Ը���ʵ���������
					SysWakeUpInit();;
                    break;	
					                
                default:
                    break;
            }
        }
       
        delayCount++;
        if(delayCount >= 2500000)
        {          
            delayCount = 0;            
            APP_DBG("%ld\n",count);
            WdgFeed();
            count++;
        }
    }
}

