///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: app_config.h
//  maintainer: Halley
///////////////////////////////////////////////////////////////////////////////
#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "timer_api.h"
#include "drc.h"
#include "three_d.h"	
#include "virtual_bass.h"
#include "rtc.h"
#include "timeout.h"
#include "delay.h"
#include "chip_info.h"
#include "os.h"
#include "eq.h"

	
//****************************************************************************************
//                  系统默认音量配置         
//****************************************************************************************
	#define MAX_VOLUME  32
	#define DEFAULT_VOLUME  29
	#define DEC_FADEIN_TIME					1000	//fadein时间	

	#define ADC_DIGITAL_VOL				VOLUME_ADC_DIGITAL
	#define DAC_DIGITAL_VOL				VOLUME_DAC_DIGITAL
	
//****************************************************************************************
//                  输出通道选择       
//****************************************************************************************
    //同一时刻只能使能其中的一种
    #define OUTPUT_CHANNEL_DAC                      //DAC输出
//    #define OUTPUT_CHANNEL_I2S                    //标准I2S输出
//    #define OUTPUT_CHANNEL_DAC_I2S                //标准I2S以及DAC同时输出
//    #define OUTPUT_CHANNEL_CLASSD                 //以外接ST系列CLASSD的I2S输出
//    #define OUTPUT_CHANNEL_DAC_CLASSD             //以外接ST系列CLASSD的I2S以及DAC输出:该输出不能播放采样率低于32KHZ,如果需要全部支持，可以开启转采样功能


//****************************************************************************************
//                  各音频源使用的MIXER模块的输入通道号配置             
//****************************************************************************************
	#define MIXER_SOURCE_DEC				0		//U盘/SD卡播放
	#define MIXER_SOURCE_BT					0		//蓝牙播放
	#define MIXER_SOURCE_USB				0		//USB声卡播放
	#define MIXER_SOURCE_MIC				1		//MIC
	#define MIXER_SOURCE_ANA_MONO			2		//单声道模拟输入，单声道GUITAR、单声道GUITAR+LINEIN
	#define MIXER_SOURCE_ANA_STERO			3		//双声道模拟输入，双声道LINEIN、双声道FM，此时MIC不可用

//****************************************************************************************
//					String format convert
//****************************************************************************************
	#define FUNC_STRING_CONVERT_EN 			// 字符串编码转换

//****************************************************************************************
//                  播放模式配置                        
//****************************************************************************************
	#define FUNC_USB_EN						// U盘播放功能
	#ifdef FUNC_USB_EN
		#define UDISK_PORT_NUM		        2		// USB端口定义
	#endif
	#define FUNC_CARD_EN						// SD卡播放功能	
	#ifdef FUNC_CARD_EN
		#define	SD_PORT_NUM                 1		// SD卡端口定义
	#endif

	#if (SD_PORT_NUM == 1)
		//SD_CLK复用为硬件检测脚：A3
		#define CARD_DETECT_PORT_IN			GPIO_A_IN	
		#define CARD_DETECT_PORT_OE			GPIO_A_OE	
		#define CARD_DETECT_PORT_PU			GPIO_A_PU	
		#define CARD_DETECT_PORT_PD			GPIO_A_PD	
		#define CARD_DETECT_PORT_IE  		GPIO_A_IE
		#define CARD_DETECT_BIT_MASK		(1 << 20)
	#else
		#define CARD_DETECT_PORT_IN			GPIO_B_IN	
		#define CARD_DETECT_PORT_OE			GPIO_B_OE	
		#define CARD_DETECT_PORT_PU			GPIO_B_PU	
		#define CARD_DETECT_PORT_PD			GPIO_B_PD	
		#define CARD_DETECT_PORT_IE  		GPIO_B_IE
		#define CARD_DETECT_BIT_MASK		(GPIOB4)
	#endif

	#if (defined(FUNC_USB_EN) || defined(FUNC_CARD_EN))
		#define FUNC_LRC_EN							// LRC歌词文件解析
		//#define FUNC_FFFB_WITHOUT_SOUND     		//可选项，快进快退是否播放声音，默认出声音
		//#define FUNC_FFFB_END_WITHOUT_AUTO_SWITCH	//可选项，快进曲尾或者快退到曲首是否停止seek，默认保持快进快退
	#endif

//****************************************************************************************
//                 USB一线通模式配置        
//****************************************************************************************
//	#define FUNC_USB_AUDIO_EN				//单独的USB声卡模式
//	#define FUNC_USB_READER_EN				//单独的USB读卡器模式
	#define FUNC_USB_AUDIO_READER_EN		//一线通模式
		#define PC_PORT_NUM			        1

//****************************************************************************************
//                 UART DEBUG功能配置        
//****************************************************************************************
	#define FUNC_DEBUG_EN
	#ifdef FUNC_DEBUG_EN
		#define FUART_RX_PORT  1	//rx port  0--A1，1--B6，2--C4，0xFF--NO USE
		#define FUART_TX_PORT  1	//tx port  0--A0，1--B7，2--C3，0xFF--NO USE
	#endif

//****************************************************************************************
//                 WATCH DOG功能配置        
//****************************************************************************************
	#define FUNC_WATCHDOG_EN			//WATCHDOG功能

	
//****************************************************************************************
//                 断点记忆功能配置        
//****************************************************************************************
	#define FUNC_BREAKPOINT_EN			// 系统断点信息管理
	#ifdef FUNC_BREAKPOINT_EN
		#define BP_SAVE_TO_NVM				//将断点信息保存到NVM 中
		#define BP_SAVE_TO_FLASH			// 掉电记忆功能（支持将断点信息保存到FLASH中，防止掉电丢失）
		//#define BP_SAVE_TO_EEPROM			// 掉电记忆功能（支持将断点信息保存到EEPROM中，防止掉电丢失）		
		#define FUNC_MATCH_PLAYER_BP		// 获取FS扫描后与播放模式断点信息相匹配的文件、文件夹ID号
	#endif


//****************************************************************************************
//                 MEM分配        
//****************************************************************************************
//DEC
#define DEC_MEM_ADDR              		(VMEM_ADDR + 0 * 1024)		//decoder 28KB
#define DEC_MEM_MAX_SIZE          		(28 * 1024)            

//ENC
#define ENC_MEM_ADDR					(VMEM_ADDR + 1 * 1024)		//encoder 17KB mp3编码 定义方式 起始1k与pcm_sync.c中定义冲突  mp3只需16k  
#define ENC_MEM_SIZE					(17 * 1024)

//ENC BUF
#define MP3_OUT_SAMPLES_BUF             (VMEM_ADDR + 18 * 1024)		//record buf 19KB
#define MP3_OUT_SAMPLES_BUF_SIZE        (19 * 1024)

//ADPCM - ENCODE
#define ADPCM_ENC_MEM_ADDR				(VMEM_ADDR + 28 * 1024)		// adpcm for bt phone call recording
#define ADPCM_ENC_MEM_SIZE				(3*1024)

//ADPCM - ENCODE BUF
#define ADPCM_OUT_SAMPLES_BUF             (VMEM_ADDR + 31 * 1024)	//record buf 6KB
#define ADPCM_OUT_SAMPLES_BUF_SIZE        (6 * 1024)

//下面几个FIFO必须在PMEM中，即大于(VMEM_ADDR + 32 * 1024)
//PCM FIFO
#define PCM_FIFO_ADDR             		(VMEM_ADDR + 37 * 1024)		//pcm fifo 16KB
#define PCM_FIFO_LEN              		(16 * 1024)

//ADC FIFO
#define ADC_FIFO_ADDR         			(VMEM_ADDR + 53 * 1024)		//adc fifo 4KB
#define ADC_FIFO_LEN             		(4 * 1024)    

//I2S FIFO
#define I2SIN_FIFO_ADDR                 (VMEM_ADDR + 57 * 1024)
#define I2SIN_FIFO_LEN                  (3 * 1024)

//BUART
#define BUART_RX_TX_FIFO_ADDR  			(VMEM_ADDR + 60 * 1024)		//buart fifo 7KB，RX、TX连续相邻
#define BUART_RX_FIFO_SIZE             	(3 * 1024)         
#define BUART_TX_FIFO_SIZE             	(1 * 1024)         


////////////////////内存管理操作/////////////////////////
#define APP_MMM_MALLOC(MemSize, MemType)          OSMalloc(MemSize, MemType)
#define APP_MMM_REALLOC(MemPtr, MemSize, MemType) OSRealloc(MemPtr, MemSize, MemType)
#define APP_MMM_FREE(MemPtr)                      OSFree(MemPtr)

/////////////////////FLASH 操作/////////////////////////

//#define FLASH_ERASE_IN_TCM			//FLASH擦除代码放在TCM中,目前只适用于GD的FLASH

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif/*_APP_CONFIG_H_*/
