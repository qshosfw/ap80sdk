#include "stdlib.h"
#include "string.h" 
#include "type.h" 
#include "app_config.h"
#include "cache.h"
#include "uart.h"
#include "clk.h"
#include "gpio.h"
#include "wakeup.h"
#include "timer.h"
#include "watchdog.h"
#include "miscfg.h"
#include "nvm.h"
#include "delay.h"

extern void SysTickInit(void);

#define MAX_CMD_LEN	1024
#define TIMEOUT_200MS 200


__attribute__((section(".driver.isr"))) void Timer1Interrupt(void)
{
	Timer1IntClr();
}


#define UPGRADE_NVM_ADDR        (176)//boot upgrade information at NVRAM address

int32_t main(void)
{	
	uint32_t BootNvmInfo;
	
	ClkModuleDis(ALL_MODULE_CLK_SWITCH & (~(FSHC_CLK_EN)));	
	ClkModuleEn(FSHC_CLK_EN | FUART_CLK_EN);			//enable Fuart clock for debugging
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating
	
#ifdef USE_POWERKEY_SLIDE_SWITCH
	ClkModuleEn(TIMER1_CLK_EN);
	SysPowerKeyInit(POWERKEY_MODE_SLIDE_SWITCH, 300);	//硬开关模式
#endif

	ClkPorRcToDpll(0);              //clock src is 32768hz OSC
	ClkDpllClkGatingEn(1);

	CacheInit();
	SysTickInit();	
	
	GpioFuartRxIoConfig(FUART_RX_PORT);
	GpioFuartTxIoConfig(FUART_TX_PORT);
	FuartInit(115200, 8, 0, 1);
	
	DBG("------------------------------------------------------\n");
	DBG("          Upgrade Request example                      \n");
	DBG("       Mountain View Silicon Tech. Inc.               \n"); 
	DBG("------------------------------------------------------\r\n\r\n");
	
	Timer1HaltModeSet(1);    //halt mode
	Timer1Set(2000);         //2ms
	NVIC_EnableIRQ(TMR1_IRQn);

	DelayMs(200);
	WdgDis();	//关闭Watchdog	
	
	DBG("\r\n【Waitting for】 Upgrade Request Command:'U' or 'u'\r\n"); 
	while(1)
	{
		uint8_t cmd_buf[MAX_CMD_LEN] = {0};
		uint16_t cmd_len;

		cmd_len =  FuartRecv(cmd_buf,MAX_CMD_LEN, TIMEOUT_200MS);
		if(cmd_len>0)
		{
			unsigned char cmd_head = cmd_buf[0];
			switch(cmd_head)
			{
				case 'U':
				case 'u':
					DBG("\r\nDo Upgrade Request\r\n");
					DBG("\r\n\r\n\r\n\r\n");
					//write upgrade ball offset address to NVM(176), and then reset system
					BootNvmInfo = UPGRADE_REQT_MAGIC;//0x100000;
					NvmWrite(UPGRADE_NVM_ADDR, (uint8_t*)&BootNvmInfo, 4);
					NVIC_SystemReset();
					break;
					
				default:
					printf("\r\nUnknown command! Upgrade Request Command is 'U' or 'u'.\r\n");
					continue;
			}
		}
		DBG(".");
	}
}

//end of file
