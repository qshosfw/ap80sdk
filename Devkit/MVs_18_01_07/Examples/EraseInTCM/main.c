//  maintainer: Halley
#include "type.h"
#include "app_config.h"
#include "os.h"
#include "msgq.h"
#include "uart.h"
#include "clk.h"
#include "spi_flash.h"
#include "cache.h"
#include "gpio.h"
#include "dac.h"
#include "audio_adc.h"
#include "wakeup.h"
#include "timer.h"
#include "adc.h"
#include "ir.h"
#include "host_hcd.h"
#include "host_stor.h"
#include "watchdog.h"
#include "mixer.h"
#include "fsinfo.h"
#include "spi_flash.h"
#include "gd_flash_drv.h"
#ifdef FUNC_SW_EQ_EN
#include "eq.h"
#include "eq_params.h"
#endif

int32_t BtTaskHandle = 0;
int32_t MainTaskHandle = 0;
xTaskHandle AudioProcessHandle;
		
#ifdef FUNC_SPI_UPDATE_EN
extern void BootUpgradeChk(void);
#endif
extern void AudioProcessTaskEntrance(void);
extern  void testFunction_1628Display(unsigned char	 cs_chip_sec);
//extern uint8_t UpgradeFileFound;

extern void DetectMassTestCondition(void);
extern bool GetMassTestFlag(void);

extern bool FlshBTInfoAreaInit(void);

extern const char* GetWakeupSrcName(unsigned int WakeupSrc);
extern uint32_t ClkGetFshcClkFreq(void);
extern uint8_t GetBootVersion(void);
extern uint8_t GetPatchVersion(void);
extern uint8_t GetBootVersionEx(void);
extern uint32_t GetSdkVer(void);
extern long GetCodeSize(void);
extern const char* GetCodeEncryptedInfo(void);

 SPI_FLASH_INFO	FlashInfo1;
uint8_t		no_det_OTP=0;
uint8_t test_data[1000];
int led_loop1=0;

#define SHOW_DOT(x)	do {	\
	int wait_cnt;						\
	TIMER t;								\
	wait_cnt = x;						\
	TimeOutSet(&t, 1000);		\
	while(wait_cnt -- >0){	\
		while(!IsTimeOut(&t));;	\
		TimeOutSet(&t, 1000);	\
		WdgFeed();					\
		DBG(".");	}				\
	DBG("\n");						\
}while(0)


__attribute__((section(".driver.isr"))) void Timer1Interrupt(void)
{
	Timer1IntClr();

#if (defined(FUNC_USB_EN) || defined(FUNC_USB_AUDIO_EN) || defined(FUNC_USB_READER_EN) || defined(FUNC_USB_AUDIO_READER_EN))
	//if(no_det_OTP==1)
	//OTGLinkCheck();
#endif
}


#define REG_GPIO ((volatile uint32_t*)(0X40002000))
__attribute__((section("CACHE.4KBMEM")))
void Cjx_GpioSetRegBits(uint8_t RegIndex, uint32_t mask) 
{ 
	__DISABLE_IRQ(); 
	REG_GPIO[RegIndex] |= mask; 
	__ENABLE_IRQ(); 
} 
__attribute__((section("CACHE.4KBMEM")))
void Cjx_GpioClrRegBits(uint8_t RegIndex, uint32_t mask) 
{ 
	__DISABLE_IRQ(); 
	REG_GPIO[RegIndex] &= (~mask); 
	__ENABLE_IRQ(); 
}

//__attribute__((section("CACHE.4KBMEM")))
//const uint8_t con_data[1024*2] = {0x15,};

SPI_FLASH_INFO FlashInfo;
int32_t main(void)
{
	ClkModuleDis((CLK_MODULE_SWITCH)(ALL_MODULE_CLK_SWITCH &(~(FSHC_CLK_EN))));	
	ClkModuleEn((CLK_MODULE_SWITCH)(FSHC_CLK_EN | FUART_CLK_EN));	//enable Fuart clock for debugging
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating
	
	SysGetWakeUpFlag();             //get wake up flag, DO NOT remove this!!
	

#ifdef USE_POWERKEY_SLIDE_SWITCH
	ClkModuleEn(TIMER1_CLK_EN);
	SysPowerKeyInit(POWERKEY_MODE_SLIDE_SWITCH, 300);	//硬开关模式
#endif

#ifdef USE_POWERKEY_SOFT_PUSH_BUTTON
	ClkModuleEn(TIMER1_CLK_EN);
	SysPowerKeyInit(POWERKEY_MODE_PUSH_BUTTON, 2000); //软开关模式 2s
#endif
	
	SpiFlashInfoInit();		//Flash RD/WR/ER/LOCK initialization
	SpiFlashGetInfo(&FlashInfo);
	
	ClkPorRcToDpll(0);              //clock src is 32768hz OSC
	ClkDpllClkGatingEn(1);

	CacheInit();
	
	SysTickInit();	
	
	GpioFuartRxIoConfig(FUART_RX_PORT);
	GpioFuartTxIoConfig(FUART_TX_PORT);
	FuartInit(115200, 8, 0, 1);
	DBG("------------------------------------------------------\n");
	DBG("          Flash Erase in TCM example                      \n");
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

	
	APP_DBG("****************************************************************\n");
	APP_DBG("System Clock     :%d MHz(%d)\n", ClkGetCurrentSysClkFreq() / 1000000, ClkGetCurrentSysClkFreq());
	APP_DBG("Flash Clock      :%d MHz(%d)\n", ClkGetFshcClkFreq() / 1000000, ClkGetFshcClkFreq());
	APP_DBG("BOOT Version     :%d.%d.%d%c\n", GetBootVersion(), GetPatchVersion() / 10, GetPatchVersion() % 10,GetBootVersionEx());
	APP_DBG("SDK  Version     :%d.%d.%d\n", (GetSdkVer() >> 8) & 0xFF, (GetSdkVer() >> 16) & 0xFF, GetSdkVer() >> 24);
	APP_DBG("Code Size        :%d(%d KB)\n", GetCodeSize(), GetCodeSize() / 1024);
	APP_DBG("Code Encrypted   :%s\n", GetCodeEncryptedInfo());
	APP_DBG("Wakeup Source    :%s(0x%08X)\n", GetWakeupSrcName(gWakeUpFlag), gWakeUpFlag);
	APP_DBG("****************************************************************\n");

	APP_DBG("The Flash erase function is in TCM, when flash is in erasing, \nApplication can aslo call other functions which is in TCM too\n");
	
	APP_DBG("\nBecause the Flash erase opreation will disable IRQs, while some \nfunctions like LED display executed in timer interrupt won't be called then\n");
	APP_DBG("\nIn this example, application can call LED display function(executed in TCM) \nin waiting for flash erasing done\n");
	while(1)
	{
#ifdef FUNC_SPI_UPDATE_EN
		if(UpgradeFileFound)
		{
			//upgrade file found,try to upgrade it
			BootUpgradeChk();
		}
#endif

		
		SHOW_DOT(10);
		FlashEraseInTCM(&FlashInfo,0x20000,64 * 1024);
	}
}


