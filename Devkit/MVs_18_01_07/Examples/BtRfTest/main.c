#include <stdint.h>
#include <stdio.h>
#include "type.h"
#include "clk.h"
#include "uart.h"
#include "gpio.h"
#include "cache.h"
#include "delay.h"
#include "watchdog.h"
#include "app_config.h"
#include "timeout.h"
#include "bt_stack_api.h"

extern void  BTEntrance(void);
//extern void  SysTickInit(void);
//extern char* BTStackGetVersion(void);
//extern void  BTStackSetMemAllocHook(void*(*)(size_t size));

int32_t main(void)
{
	ClkPorRcToDpll(0);
	CacheInit();
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);	

	SysTickInit();

	WaitMs(200);
	WdgDis();

	GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);

	FuartInit(115200, 8, 0, 1);

	WaitMs(100);
	DBG("/==========================================================================\\\n");
	DBG("|                         BT RF Test APPLICATION                           |\n");
	DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
	DBG("\\==========================================================================/\n");

	DBG("\nBluetooth Library Version: %s\n\n", BTStackGetVersion());

	BTEntrance();

	return 0;
}
