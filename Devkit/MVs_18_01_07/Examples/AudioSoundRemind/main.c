/**
 **************************************************************************************
 * @file    audio_decoders_test.c
 * @brief   Audio Decoders Testbench
 * 
 * @author  Aissen Li
 * @version V2.0.0
 * 
 * $Id$
 * $Created: 2014-04-22 17:31:10$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "app_config.h"

#include "clk.h"
#include "dac.h"
#include "adc.h"
#include "dac.h"
#include "gpio.h"
#include "uart.h"
#include "debug.h"
#include "delay.h"
#include "cache.h"
#include "fsinfo.h"
#include "fat_file.h"
#include "fs_mount.h"
#include "watchdog.h"
#include "pcm_fifo.h"
#include "pcm_transfer.h"
#include "audio_path.h"
#include "audio_utility.h"
#include "audio_decoder.h"
#include "sd_card.h"
#include "timeout.h"
#include "rst.h"

#include "sound_remind.h"
#include "spi_flash.h"

#define  MIN_DAC_VOLUME         (0)
#define  MAX_DAC_VOLUME         (32)

static const uint16_t DAC_VOL_TABLE[] =
{
	0,
	26,		31,		36,		42,		49,		58,		67,		78,
	91,		107,	125,	147,	173,	204,	240,	282,
	332,	391,	460,	541,	636,	748,	880,	1035,
	1218,	1433,	1686,	1984,	2334,	2746,	3230,	3800//最高音量约930mVrms
};

extern uint8_t UpgradeFileFound;

#ifdef FUNC_SPI_UPDATE_EN
extern void BootUpgradeChk(void);
#endif

bool CheckAllDiskLinkFlag(void)
{
    return TRUE;
}

bool SdInitAndFsMount(void)
{
    //Deinit File System first.
    FSDeInit(DEV_ID_SD);

    GpioSdIoConfig(1);

    if(SdCardInit())
    {
        DBG(("SD initialize fail.\n"));
        return FALSE;
    }

    if(!FSInit(DEV_ID_SD))
    {
        DBG(("FS mount fail.\n"));
        return FALSE;
    }

    DBG(("SD initialize and FS mount success.\n\n"));

    return TRUE;
}

static uint8_t gKey = 0;

#define GetKeyValue()    gKey
#define SetKeyValue(v)   gKey = v

/**
 * @brief Fuart Interrupt Service Routine
 * @param[in]  NONE
 * @param[out] NONE
 */
void FuartInterrupt(void)
{
    //If data received
    if(IsFuartRxIntSrc())
    {
        uint8_t c;
        
        //Get received byte
        FuartRecvByte(&c);

		//clear interrupt
        FuartClrRxInt();
        
        SetKeyValue(c);
	}

    //If data sent
	if(IsFuartTxIntSrc())
    {
		//clear interrupt
        FuartClrTxInt();
	}
}

void help(void)
{
    DBG("[SHORT_KEY]: AS FOLLOWING\n");
    DBG("    [n] ............... PLAY SOUND REMIND.\n");
    DBG("    [r] ............... READ UPGRADE FILE FROM SD CARD.\n");
    DBG("\n");
}

/*****************************************************************************
 函 数 名  : 
 功能描述  : 
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 函数层级  :
*****************************************************************************/
int32_t main(void)
{
    uint16_t	DacVolume = MAX_DAC_VOLUME / 2;

    uint8_t		KeyValue;
    uint8_t		SoundCnt=0;
    
	SpiFlashInfoInit();		//Flash RD/WR/ER/LOCK initialization
    ClkPorRcToDpll(0);
	//GD flash，选择SYS CLK或DPLL80Mhz，可以开启HPM，其他型号建议关闭HPM
	SpiFlashClkSet(FLASHCLK_SYSCLK_SEL, TRUE);
    
    CacheInit();
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);	

    SysTickInit();

	// Disable Watchdog
    WaitMs(200);
	WdgDis();

    GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);

	FuartInit(115200, 8, 0, 1);

    WaitMs(100);
    DBG("/==========================================================================\\\n");
    DBG("|                   AUDIO CODEC (DECODERS) TESTBENCH                       |\n");
    DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    DBG("\\==========================================================================/\n");

    help();

#ifdef FUNC_SPI_UPDATE_EN
	BootUpgradeChk();	//boot upgrade check upgrade for the last time
#endif

    //codec,dac..etc config
    CodecDacInit(TRUE);
    CodecDacMuteSet(TRUE, TRUE);
    CodecDacChannelSel(DAC_CH_DECD_L | DAC_CH_DECD_R);
    DacAdcSampleRateSet(44100, USB_MODE);
    DacVolumeSet(DAC_VOL_TABLE[DacVolume], DAC_VOL_TABLE[DacVolume]);
    DacNoUseCapacityToPlay();
    DacConfig(MDAC_DATA_FROM_DEC, USB_MODE);
    PhubPathSel(PCMFIFO_MIX_DACOUT);
    CodecDacMuteSet(FALSE, FALSE);

    DecoderTaskInit();

    while(1)
    {        
#ifdef FUNC_SPI_UPDATE_EN
		if(UpgradeFileFound)
		{
			//upgrade file found,try to upgrade it
			BootUpgradeChk();
		}
#endif

        //__WFI();
        
        KeyValue = GetKeyValue();

        SetKeyValue(0);
        
        switch(KeyValue)
        {
	        case 'n' :
	            DBG("[KEY__INFO]: NEXT\n");

				SoundCnt++;
				if(SoundCnt >= 6)
					SoundCnt=0;

				switch(SoundCnt)
				{
					case 0:
	            		SoundRemind(SOUND_ON);
	            		break;
	            	
					case 1:
	            		SoundRemind(SOUND_OFF);
	            		break;
	            	
					case 2:
	            		SoundRemind(SOUND_BAT_LOW);
	            		break;
	            	
					case 3:
	            		SoundRemind(SOUND_CHARGING);
	            		break;
	            	
					case 4:
	            		SoundRemind(SOUND_BAT_FULL);
	            		break;
	            	
					case 5:
	            		SoundRemind(SOUND_START_WORKING);
	            		break;

	            	default:
	            		SoundCnt=0;
	            		break;
	            }
	            break;

	        case 'r'://read upgrade file from sd card (const data)
	            SdInitAndFsMount();
	            DBG("[KEY__INFO]: RESET\n");
	            break;

	        default:
	            break;
		}
		
    	DecoderTaskEntrance();
    }
}
