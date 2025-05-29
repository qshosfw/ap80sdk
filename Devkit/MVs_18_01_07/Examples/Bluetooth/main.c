#include <stdint.h>
#include <stdio.h>
#include "type.h"
#include "clk.h"
#include "adc.h"
#include "dac.h"
#include "uart.h"
#include "gpio.h"
#include "cache.h"
//#include "debug.h"
#include "delay.h"
#include "watchdog.h"
#include "app_config.h"
#include "bt_device_type.h"
#include "timeout.h"
//#include "bt_stack_api.h"

//以下为不带OS的工程必须实行的3个API mmm_malloc、mmm_free、auxtmr_count_get。
//如果带OS，则直接与OS的相关API对接即可
//蓝牙协议栈需要的MEM，如果不带SPP，只会分配一次25800Bytes；如果带SPP，会分配两次，一次25800Bytes,第二次5608Bytes。
//因为蓝牙协议栈只有Init时才mmm_malloc，UnInit时mmm_free，所以不带OS系统简单实现如下即可。
static  uint32_t pos = 0;
uint8_t gBtStackMemBuffer[BT_STACK_MEM_SIZE_WITH_SPP];

void* BTStackMemAllocHookFunc(size_t size)
{
    void *p = gBtStackMemBuffer + pos;

    pos += size;

    if(pos > sizeof(gBtStackMemBuffer))
    {
        return (void*)0; 
    }

    return p;
}

bool CheckAllDiskLinkFlag()
{
    return TRUE;
}

//解码器需要调用文件系统如下几个原型, DEMO中无文件系统，只实现数据流解码(BT)，所以实现几个文件操作空函数
void*    FileOpen(const uint8_t *file_name, const uint8_t *mode)                       { return 0; }
uint32_t FileRead(void *buffer, uint32_t size, uint32_t count,  void* handle)          { return 0; }
uint32_t FileWrite(const void *buffer, uint32_t size, uint32_t count,  void* handle)   { return 0; }
int32_t  FileSeek(void* handle, int32_t offset, int32_t origin)                        { return 0; }
int32_t  FileTell(void* handle)                                                        { return 0; }
int32_t  FileEOF(void* handle)                                                         { return 0; }
int32_t  FileSof(void* handle)                                                         { return 0; }
int32_t  FileClose(void* handle)                                                       { return 0; }

extern void  BTEntrance(void);
//extern void  SysTickInit(void);
extern char* BTStackGetVersion(void);
extern void  BTStackSetMemAllocHook(void*(*)(size_t size));

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
    DBG("|                         BLUETOOTH APPLICATION                            |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");
    
    DBG("\nBluetooth Library Version: %s\n\n", BTStackGetVersion());

    CodecDacInit(TRUE);
    DacNoUseCapacityToPlay();   

    BTStackSetMemAllocHook(BTStackMemAllocHookFunc);
    BTEntrance();

	return 0;
}
