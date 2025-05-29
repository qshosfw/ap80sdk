/**
 **************************************************************************************
 * @file    voice_app_upgrade_check.c
 * @brief   Voice Application For Firmware Upgrade Check
 * 
 * @author  Aissen Li
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include "app_config.h"
#include "type.h"
#include "clk.h"
#include "gpio.h"
#include "uart.h"
#include "cache.h"
#include "watchdog.h"
#include "fat_file.h"

#define ENCRYPT_FLAG_ADDR       (0xFF)
#define MVA_FILE_HEADER_SIZE    (0x16)
#define MAGIC_NUMBER_ADDR       (0x9C)
#define MAGIC_NUMBER_VALUE      (0xB0BEBDC9)

static FAT_FILE MVAFile;

int SdCardInit(void);

int main(void)
{
    ClkPorRcToDpll(0);
    CacheInit();
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);

	SysTickInit();

    GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);
	FuartInit(115200, 8, 0, 1);

    DBG("\n");
    DBG("/==========================================================================\\\n");
    DBG("|                 Voice Application For Firmware Upgrade Check             |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");

    WaitMs(200);
    WdgDis();

    GpioSdIoConfig(1);

    while(SdCardInit())
    {
        WaitMs(1000);
        DBG("Please insert SD card\n");
    }

    FSInit(DEV_ID_SD);

    //1. Check the runtime code is encrypted
    if(*(uint32_t*)MAGIC_NUMBER_ADDR == MAGIC_NUMBER_VALUE)
    {
        DBG("The runtime code is not encrypted.\n");
    }
    else
    {
        DBG("The runtime code is encrypted.\n");
    }

    //2. Check the MVA file is encrypted
    if(FileOpen(&MVAFile, "\\Upgrade.mva", FA_READ))
	{
		uint8_t Buffer[512];

		if(FileRead(Buffer, 1, sizeof(Buffer), &MVAFile) == sizeof(Buffer))
        {
            if(*(uint32_t*)&Buffer[MVA_FILE_HEADER_SIZE + MAGIC_NUMBER_ADDR] == MAGIC_NUMBER_VALUE)
            {
                uint8_t EncryptFlag = Buffer[MVA_FILE_HEADER_SIZE + ENCRYPT_FLAG_ADDR];

                switch(EncryptFlag)
                {
                case 0xFF:
                    DBG("The upgrade file is not encrypted\n");
                    break;
                case 0x55:
                    DBG("The upgrade file is encrypted\n");
                    break;
                default:
                    DBG("The upgrade file maybe broken\n");
                    break;
                }
            }
            else
            {
                DBG("Illegal upgrade file.\n");
            }
        }
        else
        {
            DBG("Read upgrade file failed\n");
        }

		FileClose(&MVAFile);
	}
	else
	{
		DBG("Upgrade file is not exist\n");
	}

    while(1);

    //Just make compiler happy ^_^
    //return 0;
}
