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
//                  Decoder 相应超时设置
//****************************************************************************************
	#define DECODER_TASK_WAIT_TIMEOUT		10*1000	// 10S

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
//                  BASS/TREB功能配置                        
//****************************************************************************************
	#define FUNC_TREB_BASS_EN					//高低音调节功能
	#ifdef FUNC_TREB_BASS_EN
		#define MAX_TREB_VAL 			10		//高音调节的级数：0--10
		#define MAX_BASS_VAL 			10		//低音调节的级数：0--10
		#define DEFAULT_TREB_VAL		5		//高音调节的默认级数
		#define DEFAULT_BASS_VAL		5 		//高音调节的默认级数
	#endif
//****************************************************************************************
//                 EQ功能配置        
//****************************************************************************************
	#define EQ_STYLE_MODE				EQ_MODE
	
//****************************************************************************************
//                 DC Blocker功能配置        
//****************************************************************************************
	#define FUNC_DC_BLOCKER

//****************************************************************************************
//                 Silence detector 功能配置        
//****************************************************************************************
	#ifdef FUNC_DC_BLOCKER
//		#define FUNC_SILENCE_DETECTOR

		#define SILENCE_THRSHOLD				100
		#define SILENCE_COUNT					200
	#endif

//****************************************************************************************
//                  MIC功能配置  
//****************************************************************************************
	#define FUNC_MIC_EN								//MIC功能
	#ifdef FUNC_MIC_EN
		#define FUNC_MIC_ALC_EN							//MIC ALC功能开关
		#define MAX_MICIN_VOLUME				16		//MIC音量可调的级数：0--16
		#define DEFAULT_MICIN_VOLUME			12		//MIC音量默认值
		#define MICIN_ANA_GAIN_VAL				VOLUME_ADC_ANA_MIC		//MIC输入通路模拟增益(22.9dB),GIAN 对应关系见 sys_vol.h
		#define FUNC_MIC_DET_EN  						//MIC插拔检测定义
		#ifdef FUNC_MIC_DET_EN
			#define	MICIN_DETECT_PORT_IN		GPIO_A_IN
			#define	MICIN_DETECT_PORT_IE		GPIO_A_IE
			#define	MICIN_DETECT_PORT_OE		GPIO_A_OE
			#define	MICIN_DETECT_PORT_PU		GPIO_A_PU
			#define	MICIN_DETECT_PORT_PD		GPIO_A_PD
			#define MICIN_DETECT_BIT			(1 << 0)
		#endif

//		#define FUNC_MIC_ECHO_EN						//MIC ECHO功能
		#ifdef FUNC_MIC_ECHO_EN
			#define MAX_ECHO_DELAY_STEP			30		//MIC回声延时可调的级数：0--30
			#define DEFAULT_DELAY_STEP			22		//MIC回声延时默认值
			#define DELAY_ONE_STEP				400		//MIC回声延时每调一级对应的采样点数差异
			#define MAX_ECHO_DEPTH_STEP			16		//MIC回声强度可调的级数：0--16
			#define DEFAULT_DEPTH_STEP			12		//MIC回声强度默认值
			#define DEPTH_ONE_STEP				1024	//MIC回声强度每调一级对应的强度差异	
		#endif
	#endif

//****************************************************************************************
//					String format convert
//****************************************************************************************
	#define FUNC_STRING_CONVERT_EN 			// 字符串编码转换

//****************************************************************************************
//					升级功能
//****************************************************************************************
	#define FUNC_SPI_UPDATE_EN

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
//                 文件浏览模式配置        
//****************************************************************************************
	#define FUNC_BROWSER_EN  				//文件浏览功能

//****************************************************************************************
//                 FM收音机模式配置 
//****************************************************************************************
	#define FUNC_RADIO_EN						//Fm Radio
	#ifdef FUNC_RADIO_EN
		#define FMIN_ANA_GAIN_VAL			VOLUME_ADC_ANA_LIN23	//FM输入通道的模拟增益(10.8dB)，如果作为LineIn使用需要设置为18(-3.65dB)，否则失真
												//GIAN 对应关系见 sys_vol.h
		//#define FUNC_RADIO_DIV_CLK_EN			//FM降频工作

		#define FUNC_RADIO_RDA5807_EN
//		#define FUNC_RADIO_QN8035_EN

		#define MAX_RADIO_CHANNEL_NUM    50
		#define RADIO_SEEK_PREVIEW_SUPPORT		//支持搜台时，每搜到一个电台都自动播放几秒钟
		#define RADIO_DELAY_SWITCH_CHANNEL		//延迟处理电台切换，合并连续按键切换电台的特殊情况
		#define FM_CHL_SEARCH_TIME_UNIT 100		//搜台时频率扫描间隔ms
		#define FM_PERVIEW_TIMEOUT 1000			//FM搜台预览时间，该值不能小于MIN_TIMER_PERIOD，否则精确度会有问题(unit ms)
	#endif


//****************************************************************************************
//                 LINEIN模式配置 
//****************************************************************************************
	#define FUNC_LINEIN_EN   					// Linein
	#ifdef FUNC_LINEIN_EN
		#define LINEIN_ANA_GAIN_VAL			VOLUME_ADC_ANA_LIN1	//LINEIN输入通道的模拟增益(-3.83),GIAN 对应关系见 sys_vol.h
		#define LINEIN_DETECT_PORT_IN		GPIO_A_IN
		#define LINEIN_DETECT_PORT_OE		GPIO_A_OE
		#define LINEIN_DETECT_PORT_IE		GPIO_A_IE
		#define LINEIN_DETECT_PORT_PU		GPIO_A_PU
		#define LINEIN_DETECT_PORT_PD		GPIO_A_PD
		#define LINEIN_DETECT_BIT_MASK		(1 << 1)
	#endif
        
//****************************************************************************************
//                 I2S模式配置        
//****************************************************************************************
//  #define FUNC_I2SIN_EN

	#define FUNC_I2S_MASTER_EN				//enbale:master mode, disable: slave mode
        #define I2SIN_MCLK_IO_PORT      1   //mclk port: 0--B2, 1--C8, 0XFF--NO USE
        #define I2SIN_I2S_IO_PORT       1   //i2s port: 0-- lrck: B3, bclk: B4, din: B6, do: B5;
                                            //          1-- lrck: B24, bclk: B25, din: B27, do: B26
                                            //          2-- lrck: C9, bclk: C10, din: C12, do: C11  
                                            //          0xff--NO USE

//****************************************************************************************
//                 USB一线通模式配置        
//****************************************************************************************
//	#define FUNC_USB_AUDIO_EN				//单独的USB声卡模式
//	#define FUNC_USB_READER_EN				//单独的USB读卡器模式
	#define FUNC_USB_AUDIO_READER_EN		//一线通模式
		#define PC_PORT_NUM			        1

//****************************************************************************************
//                 录音模式配置        
//****************************************************************************************
	#if (defined(FUNC_USB_EN) || defined(FUNC_CARD_EN))
		#define FUNC_PLAY_RECORD_FOLDER_EN		//播放模式播放record文件夹
		#define FUNC_RECORD_EN   				//录音功能
		#ifdef FUNC_RECORD_EN
			#define FUNC_RECORD_FILE_DELETE_EN 		//支持录音删除功能
			#define FUNC_REC_FILE_REFRESH_EN 		//定时保存文件（避免掉电后录音数据不能保存的问题）
			#define FUNC_REC_PLAYBACK_EN 			//录音回放功能

			#define RECORD_FORMAT		MP3_FORMAT	// MP3_FORMAT , WAV_FORMAT
			#define RECORD_BITRATE		96			// 双声道最高支持320kbps，单声道最高支持192kbps
			#define RECORD_CHANNEL		1			// 必须是 1 或者 2，等于2时可能MEM不够，需要去除一些功能
		#endif
	#endif

//****************************************************************************************
//                 RTC功能配置        
//****************************************************************************************
	#define FUNC_RTC_EN						//rtc功能
	#ifdef FUNC_RTC_EN
		#define FUNC_RTC_ALARM 				//alarm功能
		#define FUNC_RTC_LUNAR
	#endif

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
//                 POWER MONITOR功能配置        
//****************************************************************************************
// 定义电能监视(适用于带电池系统)的功能宏和选项宏
// 电能监视包括，电池电压检测及低电压后的系统行为以及充电指示等
// 电池电压检测，是指LDOIN输入端的电压检测功能(电池系统一般都是电池直接给LDOIN管脚供电)
// 该功能宏打开后，默认包含电池电压检测功能，有关电池电压检测的其它可定义参数，请详见power_monitor.c文件

//USE_POWERKEY_SLIDE_SWITCH 和USE_POWERKEY_SOFT_PUSH_BUTTON 两个宏不要同时定义
	#define USE_POWERKEY_SLIDE_SWITCH     //for slide switch case ONLY
//	#define USE_POWERKEY_SOFT_PUSH_BUTTON //for soft push button case ONLY

	#define FUNC_POWER_MONITOR_EN
	
//		#define	OPTION_CHARGER_DETECT		//打开该宏定义，支持GPIO检测充电设备插入功能
			//充电检测端口设置
			#define CHARGE_DETECT_PORT_PU			GPIO_A_PU
			#define CHARGE_DETECT_PORT_PD			GPIO_A_PD
			#define CHARGE_DETECT_PORT_IN			GPIO_A_IN
			#define CHARGE_DETECT_PORT_IE			GPIO_A_IE
			#define CHARGE_DETECT_PORT_OE			GPIO_A_OE
			#define CHARGE_DETECT_BIT				(1 << 3)    


////////////////////内存管理操作/////////////////////////
#define APP_MMM_MALLOC(MemSize, MemType)          OSMalloc(MemSize, MemType)
#define APP_MMM_REALLOC(MemPtr, MemSize, MemType) OSRealloc(MemPtr, MemSize, MemType)
#define APP_MMM_FREE(MemPtr)                      OSFree(MemPtr)

/////////////////////FLASH 操作/////////////////////////

//#define FLASH_ERASE_IN_TCM			//FLASH擦除代码放在TCM中,目前只适用于GD的FLASH
#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)
#define	FS_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif/*_APP_CONFIG_H_*/
