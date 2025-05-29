/**
 **************************************************************************************
 * @file    task_bt.c
 * @brief   bt function
 * 
 * @author  Justin Xu
 * @version V1.1.0
 * 
 * $Created: 2014-02-17 15:47:04$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdio.h>
#include "type.h"
#include "app_config.h"
#include "bt_device_type.h"

static uint8_t BtAddr[6] = {136,4,11,1,35,15};                    //¿∂—¿µÿ÷∑
extern void BtRfTest(void);

void BTEntrance(void)
{
    BTDevicePinCFG();

	if(BTDeviceInit(BT_DEVICE_TYPE, (int8_t*)BtAddr))
	{
		DBG("\nEnter BT RF Test!\n");
		BtRfTest();
	}
	else
	{
		DBG("\nBTDeviceInit Fail!\n");
	}
}

void BTStackStatusCallBackFunc(uint8_t CBParameter)
{
}
void BTStackDataStatusCallBackFunc(uint8_t CBParameter, uint8_t* HistoryStatus/*This value will be AUTOMATICALLY reset to zero when new BT device connected*/)
{
}

uint32_t UserProcSppData(void)
{
    return 0;
}

typedef int(*FuncGet1of8RecInfo)(uint8_t RecIdx/*from 0*/, uint8_t *Data/*23B*/);
int FlshLoad1of8Dev(int OneFullRecBlockSize, int RecIdxFromZero, uint8_t* BtAddr, uint8_t* LinkKey, uint8_t* Property)
{
    return 0;
}

void FlashSave8DevRec(int OneFullRecBlockSize, int TotalRecNum, FuncGet1of8RecInfo GetOneInf)
{
}
