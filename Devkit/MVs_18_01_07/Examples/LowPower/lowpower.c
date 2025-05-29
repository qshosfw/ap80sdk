#include <stdio.h>
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
#include "power_management.h"
#include "watchdog.h"
//#include "debug.h"
#include "timeout.h"
//#include "get_bootup_info.h"

//extern void SysTickInit(void);
//extern void SysTickDeInit(void);
extern const char* GetWakeupSrcName(unsigned int WakeupSrc);

#define MAX_RDBUF_LEN	1024
#define MAX_CMD_LEN	1024
#define	TIMEOUT_1000MS	1000
#define TIMEOUT_200MS 200
#define TIMEOUT_100MS 100


#define SHOW_DOT(x)	do {	\
	int wait_cnt;						\
	TIMER t;								\
	wait_cnt = x;						\
	TimeOutSet(&t, 1000);		\
	while(wait_cnt -- >0){	\
		while(!IsTimeOut(&t));;	\
		TimeOutSet(&t, 1000);	\
		DBG(".");	}				\
	DBG("\n");						\
}while(0)

#define FUART_CLR_BUFFER()do{\
		uint8_t buf;							\
		while(FuartRecv(&buf, 1, 0)>0){	\
				;	}	\
}while(0);

#define FUART_TX_FIFO_FLUSH()	while(!(FuartIOctl(UART_IOCTL_TXFIFO_STAT_GET, 0) & 1));

#define SKIP_SPACE(p) do{\
	while((*p)== ' ' )	\
		{p++;}			\
}while(0)

static uint32_t ClkSel = 0;
static uint32_t ClkDiv = 0;

__attribute__((section(".driver.isr"))) void Timer1Interrupt(void)
{
	Timer1IntClr();
	//power key 检测
	SystemPowerOffDetect();
}

__attribute__((section(".driver.isr"))) void GpioInterrupt(void)
{   
    GpioIntClr(GPIO_C_INT, GPIOC2);
}


//配置GPIOC2为唤醒源，输入上拉，下降沿触发
void GpioWakeupInit(void)
{
//    BYTE GpioBank = GPIO_BANK_A;
    uint32_t Port = GPIOC2;

    //SetGpioInput(GpioBank, Port);
    GpioSetRegOneBit(GPIO_C_IE, Port);

    //no pu, no pd
    //SetGpioNoPull(GpioBank, Port);
    GpioClrRegOneBit(GPIO_B_PU, Port);
    GpioClrRegOneBit(GPIO_B_PD, Port); 

    //set triger type
    //SetGpioTriger(GpioBank, Port, GPIO_PTRIGER_NEGEDGE);

    //enable int
    //EnableSepGpioInt(GpioBank, Port);
    GpioIntEn(GPIO_C_INT, Port, GPIO_NEG_EDGE_TRIGGER);
    //GpioIntEn(GPIO_A_INT, Port, GPIO_NEG_EDGE_TRIGGER);
    
    //enable gpio irqc
    NVIC_EnableIRQ(GPIO_IRQn);
}


static void CLoseAllModuleAndSetGpio(void)
{
	
    GpioSetRegBits(GPIO_A_IE, 0xFFFFFFFF);
    GpioSetRegBits(GPIO_B_IE, 0xFFFFFFFF);
    GpioSetRegBits(GPIO_C_IE, 0x7FFF);
    
    GpioClrRegBits(GPIO_A_OE, 0xFFFFFFFF);
    GpioClrRegBits(GPIO_B_OE, 0xFFFFFFFF);
    GpioClrRegBits(GPIO_C_OE, 0x7FFF);
    											
    GpioSetRegBits(GPIO_A_PU, 0xFFFFFFFF);
    GpioClrRegBits(GPIO_A_PD, 0x0);
    
    GpioSetRegBits(GPIO_B_PU, 0xFFFFFFFF);
    GpioClrRegBits(GPIO_B_PD, 0x0);
    
    GpioSetRegBits(GPIO_C_PU, 0x7FFF);
    GpioClrRegBits(GPIO_B_PD, 0x0);	
}

void do_cmd_help()
{
	printf("\r\n【Power Comsuption Test, Comand line syntax】:\r\n");
	printf("Test normal run:--------n dpll(rc) clk_division.	e.g. \"n dpll 8\" means free running at dpll clock, divided by 8.\r\n");
	printf("Test sleep:-------------s dpll(rc) clk_division.	e.g. \"s rc 128\" means sleeping at rc clock, divided by 128.\r\n");
	printf("Test lp sleep:----------lp sleep	                This sleep mode use Rc clock, divided by 256, the power comsuption will be 2.5mA\r\n");
	printf("Test deep sleep:--------d\r\n");
	printf("Test powerdwon:---------For Powerdown tesging, just use the powerkey!\r\n");
	printf("Help:-------------------h(H/?)\r\n\r\n");
}

void do_normal_run_test(uint8_t* buf, uint32_t len)
{
		int32_t ret;
		char *p, *tmp;
		bool dpll_clk;
	
		//wait fuart tx fifo empty
		FUART_TX_FIFO_FLUSH();
	
		if((p = strstr((const char*)buf, "dpll"))!= NULL)
		{
				dpll_clk = 1;
				p += 4;
		}
		else if((p = strstr((const char*)buf, "rc"))!= NULL)
		{
				dpll_clk = 0;
				p += 2;
				ClkSwitchDpllToRc();
				ClkDpllClose();
		}
		else
		{
				DBG("\r\nBad Parameters!\r\n");
				return;
		}
		
		SKIP_SPACE(p);
		
		ret = strtol((const char *)p,&tmp,10);
		
		if(ret > 256)
			ret = 256;
		else if(ret <= 0)
			ret = 1;
		
		//set clock division
		ClkModuleDivSet(ret);
		
		
		//re-set timer1 for clock changed
		Timer1Close();
		Timer1Set(2000);    //2ms		
		//NVIC_EnableIRQ(TMR1_IRQn);
		
		//re-Init fuart and systick	
		SysTickDeInit();
		FuartInit(57600, 8, 0, 1);
		SysTickInit();
		
		DBG("\r\nret = %d", ret);
		
		DBG("\r\n\r\n************************************************************\r\n");
		if(dpll_clk)
			DBG("\r\nSyteme is now free running at Clock:DPLL, Freq:96MHz/%d\r\n",ret); 
		else
			DBG("\r\nSyteme is now free running at Clock:RC, Freq:96MHz/%d\r\n",ret);

		DBG("Send \"exit\" to exit\r\n");	
		
		
		while(1)
		{	
				if(FuartRecv(buf,5, TIMEOUT_1000MS)>=4)
				{
						if(strncmp((const char*)buf,"exit",4) == 0)
						{
								if(!dpll_clk)
								{
									ClkPorRcToDpll(0);
									ClkDpllClkGatingEn(1);
								}								
								ClkModuleDivSet(1);
								
								Timer1Close();
								Timer1Set(2000);    //2ms	
								
								SysTickDeInit();
								FuartInit(57600, 8, 0, 1);
								SysTickInit();
								break;
						}
				}
				DBG(".");
		}
		DBG("exit!\r\n");
		DBG("************************************************************\r\n");
		
}

//sleep with RC clock, divided by 256
void lp_sleep_enter(void)
{
		ClkSel = ClkModuleClkSelGet();
		ClkDiv = ClkModuleDivGet();
		ClkSwitchDpllToRc();
		ClkDpllClose();
		//set clock division
		ClkModuleDivSet(256);
		
							
		//关闭timer1中断，防止timer1中断唤醒系统
		NVIC_DisableIRQ(TMR1_IRQn);
		SysTickDeInit();
		//进入Sleep，在此之前，可以选择关闭其他一些模块以降低功耗。
		__wfi();				
		__wfe();
}

void lp_sleep_wakeup(void)
{

		if(ClkSel)
		{
			ClkPorRcToDpll(0);
			ClkDpllClkGatingEn(1);	
		}
		ClkModuleDivSet(ClkDiv);
		
	
		//re-Init fuart and systick
		SysTickDeInit();
		FuartInit(57600, 8, 0, 1);
		SysTickInit();
	
		NVIC_EnableIRQ(TMR1_IRQn);
				
		
}

void do_lp_sleep_test(void)
{
		uint32_t Port = GPIOC2;
	
		DBG("\r\nSystem will goto lp sleep in 15 Sec ...\n");    
		DBG("Pull down GPIOC2 will wake up system!\n"); 
		//DBG("\r\nSyteme is now running at Clock:RC, Freq:96MHz/256\r\n");
		SHOW_DOT(15);
		DBG("lp Sleep\n");
		DBG("************************************************************\r\n\r\n");
		 
	
		//开启GPIO中断使能，GPIOB2下降沿触发
		GpioSetRegOneBit(GPIO_C_IE, Port);
		GpioClrRegOneBit(GPIO_C_PU, Port);
		GpioClrRegOneBit(GPIO_C_PD, Port); 
		NVIC_EnableIRQ(GPIO_IRQn);	
		GpioIntEn(GPIO_C_INT, GPIOC2, GPIO_NEG_EDGE_TRIGGER);
	
		lp_sleep_enter();
		
		lp_sleep_wakeup();
		
		FUART_CLR_BUFFER();
		DBG("Wake up from lp sleep!\r\n");  		
}


void do_sleep_test(uint8_t* buf, uint32_t len)
{
		uint32_t Port = GPIOC2;
		int32_t ret;		
		char *p, *tmp;
		bool dpll_clk;
	
		//wait fuart tx fifo empty
		FUART_TX_FIFO_FLUSH();
		
		//parse command line
		if((p = strstr((const char*)buf, "dpll"))!= NULL)
		{
				dpll_clk = 1;
				p += 4;;
		}
		else if((p = strstr((const char*)buf, "rc"))!= NULL)
		{
				dpll_clk = 0;
				p += 2;
				ClkSwitchDpllToRc();
				ClkDpllClose();
		}
		else
		{
				DBG("\r\nBad Parameters!\r\n");
				return;
		}
		
		SKIP_SPACE(p);
		
		//get clock division
		ret = strtol((const char *)p,&tmp,10);
		
		if(ret > 256)
			ret = 256;
		else if(ret <= 0)
			ret = 1;
		
		//set clock division
		ClkModuleDivSet(ret);
		
		//re-Init fuart and systick
		SysTickDeInit();
		FuartInit(57600, 8, 0, 1);
		SysTickInit();
		
		DBG("\r\nret = %d", ret);
		
	
		DBG("\r\n\r\n************************************************************\r\n");
		if(dpll_clk)
			DBG("\r\nSyteme is now running at Clock:DPLL, Freq:96MHz/%d\r\n",ret); 
		else
			DBG("\r\nSyteme is now running at Clock:RC, Freq:96MHz/%d\r\n",ret); 
		DBG("System will goto sleep in 15 Sec ...\n");    
		DBG("Pull down GPIOC2 will wake up system!\n");  
	
		
		SHOW_DOT(15);
		
		DBG("Sleep\n");
		DBG("************************************************************\r\n\r\n");
							
		//开启GPIO中断使能，GPIOB2下降沿触发
		GpioSetRegOneBit(GPIO_C_IE, Port);
		GpioClrRegOneBit(GPIO_C_PU, Port);
		GpioClrRegOneBit(GPIO_C_PD, Port); 
		NVIC_EnableIRQ(GPIO_IRQn);	
		GpioIntEn(GPIO_C_INT, GPIOC2, GPIO_NEG_EDGE_TRIGGER);
		
		
		//关闭timer1中断，防止timer1中断唤醒系统
		NVIC_DisableIRQ(TMR1_IRQn);
		SysTickDeInit();
		//进入Sleep，在此之前，可以选择关闭其他一些模块以降低功耗。
		__wfi();				
		__wfe();
		{
				ClkPorRcToDpll(0);
				ClkModuleDivSet(1);
				ClkDpllClkGatingEn(1);				
				SysTickDeInit();
				FuartInit(57600, 8, 0, 1);
				SysTickInit();
		}
		NVIC_EnableIRQ(TMR1_IRQn);
				
		DBG("Wake up from sleep!\r\n");  		
	
		FUART_CLR_BUFFER();
}

void do_deepsleep_test(uint8_t* buf, uint32_t len)
{
		uint32_t Port = GPIOC2;

		DBG("\r\n\r\n************************************************************\r\n");
		DBG("System will goto Deep sleep in 15 Sec. ...\r\n"); 
		DBG("Pull Up GPIOC2 will wake up system!\r\n");  

		SHOW_DOT(15);

		//set GPIOC2 input enable, pull down
		GpioSetRegOneBit(GPIO_C_IE, Port);
		GpioSetRegOneBit(GPIO_C_PU, Port);
		GpioSetRegOneBit(GPIO_C_PD, Port);  

		//配置GPIOC2为唤醒源
		SysSetWakeUpSrcInDeepSleep(WAKEUP_SRC_SLEEP_C2, WAKEUP_POLAR_C2_HI, WAKEUP_TMODE_1MS);
		DBG("DEEP SLEEP\r\n");
		DBG("************************************************************\r\n\r\n");
		//wait fuart tx fifo empty
		FUART_TX_FIFO_FLUSH();
		CLoseAllModuleAndSetGpio();
		SysGotoDeepSleep();

		//following code won't be excecuted, because wake up from deep sleep
		//the system will restart
		DBG("******************************************************\n");
		DBG("wake up!\r\n");  		
		DBG("******************************************************\r\n\r\n");
}

void do_powerdown_test()
{
		SysSetWakeUpSrcInPowerDown(WAKEUP_SRC_PD_POWERKEY);
		SysGotoPowerDown();
}


//Powerkey配置成硬开关模式
//enalbe uart clock will consume extra 400uA current
int32_t main(void)
{	
	ClkModuleDis((CLK_MODULE_SWITCH)(ALL_MODULE_CLK_SWITCH &(~(FSHC_CLK_EN))));	
	ClkModuleEn((CLK_MODULE_SWITCH)(FSHC_CLK_EN | FUART_CLK_EN));	//enable Fuart clock for debugging
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating
	
	CLoseAllModuleAndSetGpio();
	
	SysGetWakeUpFlag();             //get wake up flag, DO NOT remove this!!
	

#ifdef USE_POWERKEY_SLIDE_SWITCH
	ClkModuleEn(TIMER1_CLK_EN);
	SysPowerKeyInit(POWERKEY_MODE_SLIDE_SWITCH, 300);	//硬开关模式
#endif

#ifdef USE_POWERKEY_SOFT_PUSH_BUTTON
	ClkModuleEn(TIMER1_CLK_EN);
	SysPowerKeyInit(POWERKEY_MODE_PUSH_BUTTON, 2000); //软开关模式 2s
#endif
	
	
	ClkPorRcToDpll(0);              //clock src is 32768hz OSC
	ClkDpllClkGatingEn(1);

	CacheInit();
	SysTickInit();	
	
	GpioFuartRxIoConfig(FUART_RX_PORT);
	GpioFuartTxIoConfig(FUART_TX_PORT);
	FuartInit(57600, 8, 0, 1);
	DBG("------------------------------------------------------\n");
	DBG("          PowerKey&sleep example                      \n");
	DBG("       Mountain View Silicon Tech. Inc.               \n"); 
	DBG("------------------------------------------------------\r\n\r\n");
	
	DBG("Wakeup Source    :%s(0x%08X)\n", GetWakeupSrcName(gWakeUpFlag), gWakeUpFlag);
	//GPIO唤醒源配置
	//GpioWakeupInit();
	
	//配置timer1，用于powerkey的检测
	Timer1HaltModeSet(1);    //halt mode
	Timer1Set(2000);         //2ms
	NVIC_EnableIRQ(TMR1_IRQn);

	DelayMs(200);
	WdgDis();	//关闭Watchdog
	
	do_cmd_help();	
	
	DBG("\r\n【Default】 Clock:DPLL, Freq:96MHz\r\n"); 
	while(1)
	{
		uint8_t cmd_buf[MAX_CMD_LEN] = {0};
		uint16_t cmd_len;
		//int32_t ret;

		cmd_len =  FuartRecv(cmd_buf,MAX_CMD_LEN, TIMEOUT_200MS);
		if(cmd_len>0)
		{
			unsigned char cmd_head = cmd_buf[0];
			switch(cmd_head)
			{
				case 'N':
				case 'n':
					do_normal_run_test(&cmd_buf[1],cmd_len-1);
					DBG("\r\n【Default】 Clock:DPLL, Freq:96MHz\r\n");
					break;
				
				case 'S':
				case 's':
					do_sleep_test(&cmd_buf[1],cmd_len-1);
					DBG("\r\n【Default】 Clock:DPLL, Freq:96MHz\r\n");
					break;
				
				case 'D':
				case 'd':
					do_deepsleep_test(&cmd_buf[1],cmd_len-1);
					break;		

				case 'P':
				case 'p':
						if(cmd_buf[1] == 'd' || cmd_buf[1] == 'D')
						{
								do_powerdown_test();
						}
						break;
				case 'h':
				case 'H':
				case '?':
					do_cmd_help();
					continue;
				
				case 'l':
					if(!strncmp((const char*)&cmd_buf[0], "lp sleep", 8))
					{
							do_lp_sleep_test();
					}
					break;
					
				default:
					printf("\r\nUnknown command! See help for usage.\r\n");
					continue;
			
			}
		
		}
				
		DBG(".");
	}


}

//end of file
