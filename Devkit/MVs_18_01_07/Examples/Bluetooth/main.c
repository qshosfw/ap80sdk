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

//����Ϊ����OS�Ĺ��̱���ʵ�е�3��API mmm_malloc��mmm_free��auxtmr_count_get��
//�����OS����ֱ����OS�����API�ԽӼ���
//����Э��ջ��Ҫ��MEM���������SPP��ֻ�����һ��25800Bytes�������SPP����������Σ�һ��25800Bytes,�ڶ���5608Bytes��
//��Ϊ����Э��ջֻ��Initʱ��mmm_malloc��UnInitʱmmm_free�����Բ���OSϵͳ��ʵ�����¼��ɡ�
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

//��������Ҫ�����ļ�ϵͳ���¼���ԭ��, DEMO�����ļ�ϵͳ��ֻʵ������������(BT)������ʵ�ּ����ļ������պ���
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
