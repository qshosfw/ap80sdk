///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: app_config.h
///////////////////////////////////////////////////////////////////////////////
#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

//#include "type.h"	
//#include "msgq.h"
//#include "app_msg.h"
//#include "timer_api.h"
//#include "three_d_ws.h"	
//#include "rtc.h"
//#include "bt_device_type.h"
//#include "timeout.h"
//#include "delay.h"
//#include "chip_info.h"
	
//****************************************************************************************
//                  系统默认音量配置         
//****************************************************************************************
	#define MAX_VOLUME  32
	#define DEFAULT_VOLUME  26
	#define DEC_FADEIN_TIME					1000	//fadein时间	
	

//****************************************************************************************
//                  频谱采集配置        
//****************************************************************************************
	#define FUNC_SPEC_TRUM_EN                      	//假频谱功能开关
	

//****************************************************************************************
//                  系统采样率配置            
//****************************************************************************************
//使能该宏表示MIXER模块自动将各种采样率统一转换为44.1KHZ
//当做卡拉OK类的应用时，强烈建议使能该宏
//使能该宏后，MIXER模块对非44.1KHZ采样率的数据做采样率转换，高频信号会略有衰减
//	#define FUNC_MIXER_SRC_EN   					
													
//****************************************************************************************
//                  软件微调配置       
//****************************************************************************************
//使能该宏，则会在Normal模式输出时，进行软件微调
//  #define FUNC_SOFT_ADJUST_EN

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
//                  MIC功能配置                        
//****************************************************************************************
	#define FUNC_MIC_EN								//MIC功能
	#ifdef FUNC_MIC_EN
		#define FUNC_MIC_ALC_EN							//MIC ALC功能开关
		#define MAX_MICIN_VOLUME				16		//MIC音量可调的级数：0--16
		#define DEFAULT_MICIN_VOLUME			10		//MIC音量默认值
		#define MICIN_ANA_GAIN_VAL				20		//MIC输入通路模拟增益
		#define FUNC_MIC_DET_EN  						//MIC插拔检测定义
		#ifdef FUNC_MIC_DET_EN
			#define	MICIN_DETECT_PORT_IN		GPIO_A_IN
			#define	MICIN_DETECT_PORT_IE		GPIO_A_IE
			#define	MICIN_DETECT_PORT_OE		GPIO_A_OE
			#define	MICIN_DETECT_PORT_PU		GPIO_A_PU
			#define	MICIN_DETECT_PORT_PD		GPIO_A_PD
			#define MICIN_DETECT_BIT			(1 << 0)
		#endif

		#define FUNC_MIC_ECHO_EN						//MIC ECHO功能
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
//                  播放模式配置                        
//****************************************************************************************
	#define FUNC_PLAYER_EN   						//U盘/SD卡播放功能
		#define UDISK_PORT_NUM		        2		//USB端口定义
		#define	SD_PORT_NUM                 1		//SD卡端口定义
		#define FUNC_FOLDER_PLAY_EN         		//支持文件夹播放模式

//****************************************************************************************
//                 文件浏览模式配置        
//****************************************************************************************
	#define FUNC_BROWSER_EN  				//文件浏览功能

//****************************************************************************************
//                 蓝牙模式配置        
//****************************************************************************************
	#define FUNC_BT_EN       								//蓝牙功能
	#ifdef FUNC_BT_EN
		#define BT_DEVICE_TYPE  BTUartDeviceMTK662X				//BTUartDeviceRda5875 // bt chip type

		#define FUNC_BT_HF_EN									//bt hf call module
		#ifdef FUNC_BT_HF_EN
			#define MAX_BTHF_VOLUME 					(15)	//蓝牙最大音量等级
		#endif

		#define BT_RECONNECTION_FUNC							// 蓝牙自动重连功能
		#ifdef BT_RECONNECTION_FUNC
			#define RECONNECTION_TRY_COUNTS				(7)		// 自动重连次数
			#define RECONNECTION_PERIOD_TIME			(3)		// 重连间隔时间(in seconds)
		#endif

//		#define FAST_POWER_ON_OFF_BT							// 快速打开/关闭蓝牙功能
	#endif
//****************************************************************************************
//                 FM收音机模式配置        
//****************************************************************************************
	#define FUNC_RADIO_EN						//Fm Radio
	#ifdef FUNC_RADIO_EN
		#define FMIN_ANA_GAIN_VAL			9	//FM输入通道的模拟增益，如果作为LineIn使用需要设置为18，否则失真			
//		#define FUNC_RADIO_DIV_CLK_EN			//FM降频工作

		#define FUNC_RADIO_RDA5807_EN
//		#define FUNC_RADIO_RDA5876A_EN
//		#define FUNC_RADIO_BK1080_EN
//		#define FUNC_RADIO_QN8035_EN
//		#define FUNC_RADIO_KT0830_EN

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
		#define LINEIN_ANA_GAIN_VAL			9	//LINEIN输入通道的模拟增益
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
	#define FUNC_RECORD_EN   				//录音功能
	#ifdef FUNC_RECORD_EN
		#define FUNC_RECORD_FILE_DELETE_EN 		//支持录音删除功能
		#define FUNC_REC_FILE_REFLESH_EN 		//定时保存文件（避免掉电后录音数据不能保存的问题）
		#define FUNC_REC_PLAYBACK_EN 			//录音回放功能
		#define FUNC_PLAY_RECORD_FOLDER_EN		//播放模式播放record文件夹
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
		#define FUNC_APP_DEBUG_EN
		#define FUNC_FS_DEBUG_EN
		#define FUART_RX_PORT  1	//rx port  0--A1，1--B6，2--C4，0xFF--NO USE
		#define FUART_TX_PORT  1	//tx port  0--A0，1--B7，2--C3，0xFF--NO USE
	#endif

//****************************************************************************************
//                 WATCH DOG功能配置        
//****************************************************************************************
	#define FUNC_WATCHDOG_EN			//WATCHDOG功能


//****************************************************************************************
//                 3D音效功能配置        
//****************************************************************************************
	//AUDIO_3D_EFFECT_VAL note: Q1.15 format to represent value in range [0,1). For example, 16384 represents 0.5 for cross_gain.
	//0x5000 represents 0.65 for cross_gain.
	//0x7800 represents 0.9375 for cross_gain.
	#define FUNC_AUDIO_3D_EFFECT_EN
		#define AUDIO_3D_EFFECT_VAL         0x7800	// 3D效果参数
		#define AUDIO_3D_AUTO_LEVEL_CONTROL TRUE	// 3D效果参数
		#define LINEINMIC_OR_FMMIX_3D_EFFECT_EN		// 支持line_mic_guitar 和fm_mic_guitar 混播时的 3D效果

//****************************************************************************************
//                 语音提示音功能配置        
//****************************************************************************************
	#define FUNC_SOUND_REMIND		//语音提示

//****************************************************************************************
//                 断点记忆功能配置        
//****************************************************************************************
	#define FUNC_BREAKPOINT_EN			// 系统断点信息管理
	#ifdef FUNC_BREAKPOINT_EN
		#define FUNC_NVM_TO_FLASH_EN		// 掉电记忆功能（支持将NVM数据保存到FLASH中，防止掉电丢失）
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
			#define CHARGE_DETECT_PORT_PU			GPIO_E_PU
			#define CHARGE_DETECT_PORT_PD			GPIO_E_PD
			#define CHARGE_DETECT_PORT_IN			GPIO_E_IN
			#define CHARGE_DETECT_PORT_IE			GPIO_E_IE
			#define CHARGE_DETECT_PORT_OE			GPIO_E_OE
			#define CHARGE_DETECT_BIT				(1 << 3)


//****************************************************************************************
//                 按键功能配置        
//****************************************************************************************
	#define FUNC_KEY_BEEP_SOUND_EN    						//按键beep声功能

	//ADC KEY定义
	#define FUNC_ADC_KEY_EN								//ADC KEY宏开关  									
		#define ADC_KEY_PORT_CH1	ADC_CHANNEL_B22			//如果只使用1路ADC按键，则保留本行
		#define	ADC_KEY_PORT_CH2	ADC_CHANNEL_B5			//如果只使用2路ADC按键，则保留本行

	//IR KEY定义
	#define FUNC_IR_KEY_EN   							//IR KEY宏开关
		#define IR_KEY_PORT			IR_USE_GPIOC2			//IR使用的端口：IR_USE_GPIOA10, IR_USE_GPIOB7 or IR_USE_GPIOC2

	//CODING KEY定义
	#define FUNC_CODING_KEY_EN							//CODING KEY宏开关			
		#define 	CODING_KEY_A_PORT_IN	GPIO_B_IN	
		#define		CODING_KEY_A_PORT_OE	GPIO_B_OE 			
		#define		CODING_KEY_A_PORT_PU	GPIO_B_PU 
		#define		CODING_KEY_A_PORT_PD	GPIO_B_PD 					
		#define		CODING_KEY_A_PORT_INT	GPIO_B_INT 				
		#define		CODING_KEY_A_BIT		(1 << 21)	//GPIO_B[21] for signal A 
	
		#define		CODING_KEY_B_PORT_IN	GPIO_B_IN 				
		#define		CODING_KEY_B_PORT_OE	GPIO_B_OE 			
		#define		CODING_KEY_B_PORT_PU	GPIO_B_PU 
		#define		CODING_KEY_B_PORT_PD	GPIO_B_PD 					
		#define		CODING_KEY_B_BIT		(1 << 20)	//GPIO_B[20] for signal B


//****************************************************************************************
//                 音频菜单配置        
//****************************************************************************************
	#define FUNC_AUDIO_MENU_EN						//音量键功能复用选择：MIC音量、回声延时、回声强度、BASS调节、TREB调节
		#define		MENU_TIMEOUT		5000		//进入菜单调节后，如果5秒没有按MENU、VOL_UP、VOL_DW，则自动退出菜单模式

//****************************************************************************************
//                 系统模式值配置        
//****************************************************************************************
enum EnumModuleID
{
    MODULE_ID_UNKNOWN     = 0,	// idle

    MODULE_ID_PLAYER_SD,		// player
    MODULE_ID_PLAYER_USB,		// player
    MODULE_ID_BLUETOOTH,		// module bt
    MODULE_ID_RECORDER,			// recorder
    MODULE_ID_RADIO,			// FM/AM radio
    MODULE_ID_LINEIN,			// line in
    MODULE_ID_I2SIN,            // I2s in
    MODULE_ID_USB_AUDIO,		// usb audio
    MODULE_ID_USB_READER,		// usb audio
    MODULE_ID_USB_AUDIO_READER,	// usb audio

    //add more herr
    MODULE_ID_RTC,				// rtc

    MODULE_ID_BT_HF,

    // 以上应用模式，可以通过mode键循环切换进入
    //////////////////////////////////////////////////////////////
    // DO NOT MODIFY FOLLOWINGS
    MODULE_ID_END,				// end of moduleID
    // 以下应用模式不能通过mode键切换进入，而是根据实际用途 通过快捷键或特殊条件满足时才会进入

    //MODULE_ID_ALARM_REMIND,		// alarm remind
    MODULE_ID_POWEROFF,			// faked module ID for Power OFF
    MODULE_ID_STANDBY,			// faked module ID for StandBy mode
#ifdef FUNC_REC_PLAYBACK_EN
    MODULE_ID_REC_BACK_PLAY,	// 录音回放模式
#endif
    MODULE_ID_USER_PLUS,		// 用户扩展起始ID
};


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


//下面几个FIFO必须在PMEM中，即大于(VMEM_ADDR + 32 * 1024)
//PCM FIFO
#define PCM_FIFO_ADDR             		(VMEM_ADDR + 37 * 1024)		//pcm fifo 16KB
#define PCM_FIFO_LEN              		(16 * 1024)

//ADC FIFO
#define ADC_FIFO_ADDR         			(VMEM_ADDR + 53 * 1024)		//adc fifo 4KB
#define ADC_FIFO_LEN             		(4 * 1024)         

//BUART
#define BUART_RX_TX_FIFO_ADDR  			(VMEM_ADDR + 57 * 1024)		//buart fifo 7KB，RX、TX连续相邻
#define BUART_RX_FIFO_SIZE             	(6 * 1024)         
#define BUART_TX_FIFO_SIZE             	(1 * 1024)         


////////////////////内存管理操作/////////////////////////
#define APP_MMM_MALLOC(MemSize, MemType)          OSMalloc(MemSize, MemType)
#define APP_MMM_REALLOC(MemPtr, MemSize, MemType) OSRealloc(MemPtr, MemSize, MemType)
#define APP_MMM_FREE(MemPtr)                      OSFree(MemPtr)


#include "debug.h"

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif/*_APP_CONFIG_H_*/
