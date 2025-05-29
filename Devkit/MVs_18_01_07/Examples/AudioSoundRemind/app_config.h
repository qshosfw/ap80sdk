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
//                  ϵͳĬ����������         
//****************************************************************************************
	#define MAX_VOLUME  32
	#define DEFAULT_VOLUME  29
	#define DEC_FADEIN_TIME					1000	//fadeinʱ��	

	#define ADC_DIGITAL_VOL				VOLUME_ADC_DIGITAL
	#define DAC_DIGITAL_VOL				VOLUME_DAC_DIGITAL
	
//****************************************************************************************
//                  ���ͨ��ѡ��       
//****************************************************************************************
    //ͬһʱ��ֻ��ʹ�����е�һ��
    #define OUTPUT_CHANNEL_DAC                      //DAC���
//    #define OUTPUT_CHANNEL_I2S                    //��׼I2S���
//    #define OUTPUT_CHANNEL_DAC_I2S                //��׼I2S�Լ�DACͬʱ���
//    #define OUTPUT_CHANNEL_CLASSD                 //�����STϵ��CLASSD��I2S���
//    #define OUTPUT_CHANNEL_DAC_CLASSD             //�����STϵ��CLASSD��I2S�Լ�DAC���:��������ܲ��Ų����ʵ���32KHZ,�����Ҫȫ��֧�֣����Կ���ת��������
                                                   
//****************************************************************************************
//                  Decoder ��Ӧ��ʱ����
//****************************************************************************************
	#define DECODER_TASK_WAIT_TIMEOUT		10*1000	// 10S

//****************************************************************************************
//                  ����ƵԴʹ�õ�MIXERģ�������ͨ��������             
//****************************************************************************************
	#define MIXER_SOURCE_DEC				0		//U��/SD������
	#define MIXER_SOURCE_BT					0		//��������
	#define MIXER_SOURCE_USB				0		//USB��������
	#define MIXER_SOURCE_MIC				1		//MIC
	#define MIXER_SOURCE_ANA_MONO			2		//������ģ�����룬������GUITAR��������GUITAR+LINEIN
	#define MIXER_SOURCE_ANA_STERO			3		//˫����ģ�����룬˫����LINEIN��˫����FM����ʱMIC������


//****************************************************************************************
//                  BASS/TREB��������                        
//****************************************************************************************
	#define FUNC_TREB_BASS_EN					//�ߵ������ڹ���
	#ifdef FUNC_TREB_BASS_EN
		#define MAX_TREB_VAL 			10		//�������ڵļ�����0--10
		#define MAX_BASS_VAL 			10		//�������ڵļ�����0--10
		#define DEFAULT_TREB_VAL		5		//�������ڵ�Ĭ�ϼ���
		#define DEFAULT_BASS_VAL		5 		//�������ڵ�Ĭ�ϼ���
	#endif
//****************************************************************************************
//                 EQ��������        
//****************************************************************************************
	#define EQ_STYLE_MODE				EQ_MODE
	
//****************************************************************************************
//                 DC Blocker��������        
//****************************************************************************************
	#define FUNC_DC_BLOCKER

//****************************************************************************************
//                 Silence detector ��������        
//****************************************************************************************
	#ifdef FUNC_DC_BLOCKER
//		#define FUNC_SILENCE_DETECTOR

		#define SILENCE_THRSHOLD				100
		#define SILENCE_COUNT					200
	#endif

//****************************************************************************************
//                  MIC��������  
//****************************************************************************************
	#define FUNC_MIC_EN								//MIC����
	#ifdef FUNC_MIC_EN
		#define FUNC_MIC_ALC_EN							//MIC ALC���ܿ���
		#define MAX_MICIN_VOLUME				16		//MIC�����ɵ��ļ�����0--16
		#define DEFAULT_MICIN_VOLUME			12		//MIC����Ĭ��ֵ
		#define MICIN_ANA_GAIN_VAL				VOLUME_ADC_ANA_MIC		//MIC����ͨ·ģ������(22.9dB),GIAN ��Ӧ��ϵ�� sys_vol.h
		#define FUNC_MIC_DET_EN  						//MIC��μ�ⶨ��
		#ifdef FUNC_MIC_DET_EN
			#define	MICIN_DETECT_PORT_IN		GPIO_A_IN
			#define	MICIN_DETECT_PORT_IE		GPIO_A_IE
			#define	MICIN_DETECT_PORT_OE		GPIO_A_OE
			#define	MICIN_DETECT_PORT_PU		GPIO_A_PU
			#define	MICIN_DETECT_PORT_PD		GPIO_A_PD
			#define MICIN_DETECT_BIT			(1 << 0)
		#endif

//		#define FUNC_MIC_ECHO_EN						//MIC ECHO����
		#ifdef FUNC_MIC_ECHO_EN
			#define MAX_ECHO_DELAY_STEP			30		//MIC������ʱ�ɵ��ļ�����0--30
			#define DEFAULT_DELAY_STEP			22		//MIC������ʱĬ��ֵ
			#define DELAY_ONE_STEP				400		//MIC������ʱÿ��һ����Ӧ�Ĳ�����������
			#define MAX_ECHO_DEPTH_STEP			16		//MIC����ǿ�ȿɵ��ļ�����0--16
			#define DEFAULT_DEPTH_STEP			12		//MIC����ǿ��Ĭ��ֵ
			#define DEPTH_ONE_STEP				1024	//MIC����ǿ��ÿ��һ����Ӧ��ǿ�Ȳ���	
		#endif
	#endif

//****************************************************************************************
//					String format convert
//****************************************************************************************
	#define FUNC_STRING_CONVERT_EN 			// �ַ�������ת��

//****************************************************************************************
//					��������
//****************************************************************************************
	#define FUNC_SPI_UPDATE_EN

//****************************************************************************************
//                  ����ģʽ����                        
//****************************************************************************************
	#define FUNC_USB_EN						// U�̲��Ź���
	#ifdef FUNC_USB_EN
		#define UDISK_PORT_NUM		        2		// USB�˿ڶ���
	#endif
	#define FUNC_CARD_EN						// SD�����Ź���	
	#ifdef FUNC_CARD_EN
		#define	SD_PORT_NUM                 1		// SD���˿ڶ���
	#endif

	#if (SD_PORT_NUM == 1)
		//SD_CLK����ΪӲ�����ţ�A3
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
		#define FUNC_LRC_EN							// LRC����ļ�����
		//#define FUNC_FFFB_WITHOUT_SOUND     		//��ѡ���������Ƿ񲥷�������Ĭ�ϳ�����
		//#define FUNC_FFFB_END_WITHOUT_AUTO_SWITCH	//��ѡ������β���߿��˵������Ƿ�ֹͣseek��Ĭ�ϱ��ֿ������
	#endif

//****************************************************************************************
//                 �ļ����ģʽ����        
//****************************************************************************************
	#define FUNC_BROWSER_EN  				//�ļ��������

//****************************************************************************************
//                 FM������ģʽ���� 
//****************************************************************************************
	#define FUNC_RADIO_EN						//Fm Radio
	#ifdef FUNC_RADIO_EN
		#define FMIN_ANA_GAIN_VAL			VOLUME_ADC_ANA_LIN23	//FM����ͨ����ģ������(10.8dB)�������ΪLineInʹ����Ҫ����Ϊ18(-3.65dB)������ʧ��
												//GIAN ��Ӧ��ϵ�� sys_vol.h
		//#define FUNC_RADIO_DIV_CLK_EN			//FM��Ƶ����

		#define FUNC_RADIO_RDA5807_EN
//		#define FUNC_RADIO_QN8035_EN

		#define MAX_RADIO_CHANNEL_NUM    50
		#define RADIO_SEEK_PREVIEW_SUPPORT		//֧����̨ʱ��ÿ�ѵ�һ����̨���Զ����ż�����
		#define RADIO_DELAY_SWITCH_CHANNEL		//�ӳٴ����̨�л����ϲ����������л���̨���������
		#define FM_CHL_SEARCH_TIME_UNIT 100		//��̨ʱƵ��ɨ����ms
		#define FM_PERVIEW_TIMEOUT 1000			//FM��̨Ԥ��ʱ�䣬��ֵ����С��MIN_TIMER_PERIOD������ȷ�Ȼ�������(unit ms)
	#endif


//****************************************************************************************
//                 LINEINģʽ���� 
//****************************************************************************************
	#define FUNC_LINEIN_EN   					// Linein
	#ifdef FUNC_LINEIN_EN
		#define LINEIN_ANA_GAIN_VAL			VOLUME_ADC_ANA_LIN1	//LINEIN����ͨ����ģ������(-3.83),GIAN ��Ӧ��ϵ�� sys_vol.h
		#define LINEIN_DETECT_PORT_IN		GPIO_A_IN
		#define LINEIN_DETECT_PORT_OE		GPIO_A_OE
		#define LINEIN_DETECT_PORT_IE		GPIO_A_IE
		#define LINEIN_DETECT_PORT_PU		GPIO_A_PU
		#define LINEIN_DETECT_PORT_PD		GPIO_A_PD
		#define LINEIN_DETECT_BIT_MASK		(1 << 1)
	#endif
        
//****************************************************************************************
//                 I2Sģʽ����        
//****************************************************************************************
//  #define FUNC_I2SIN_EN

	#define FUNC_I2S_MASTER_EN				//enbale:master mode, disable: slave mode
        #define I2SIN_MCLK_IO_PORT      1   //mclk port: 0--B2, 1--C8, 0XFF--NO USE
        #define I2SIN_I2S_IO_PORT       1   //i2s port: 0-- lrck: B3, bclk: B4, din: B6, do: B5;
                                            //          1-- lrck: B24, bclk: B25, din: B27, do: B26
                                            //          2-- lrck: C9, bclk: C10, din: C12, do: C11  
                                            //          0xff--NO USE

//****************************************************************************************
//                 USBһ��ͨģʽ����        
//****************************************************************************************
//	#define FUNC_USB_AUDIO_EN				//������USB����ģʽ
//	#define FUNC_USB_READER_EN				//������USB������ģʽ
	#define FUNC_USB_AUDIO_READER_EN		//һ��ͨģʽ
		#define PC_PORT_NUM			        1

//****************************************************************************************
//                 ¼��ģʽ����        
//****************************************************************************************
	#if (defined(FUNC_USB_EN) || defined(FUNC_CARD_EN))
		#define FUNC_PLAY_RECORD_FOLDER_EN		//����ģʽ����record�ļ���
		#define FUNC_RECORD_EN   				//¼������
		#ifdef FUNC_RECORD_EN
			#define FUNC_RECORD_FILE_DELETE_EN 		//֧��¼��ɾ������
			#define FUNC_REC_FILE_REFRESH_EN 		//��ʱ�����ļ�����������¼�����ݲ��ܱ�������⣩
			#define FUNC_REC_PLAYBACK_EN 			//¼���طŹ���

			#define RECORD_FORMAT		MP3_FORMAT	// MP3_FORMAT , WAV_FORMAT
			#define RECORD_BITRATE		96			// ˫�������֧��320kbps�����������֧��192kbps
			#define RECORD_CHANNEL		1			// ������ 1 ���� 2������2ʱ����MEM��������Ҫȥ��һЩ����
		#endif
	#endif

//****************************************************************************************
//                 RTC��������        
//****************************************************************************************
	#define FUNC_RTC_EN						//rtc����
	#ifdef FUNC_RTC_EN
		#define FUNC_RTC_ALARM 				//alarm����
		#define FUNC_RTC_LUNAR
	#endif

//****************************************************************************************
//                 UART DEBUG��������        
//****************************************************************************************
	#define FUNC_DEBUG_EN
	#ifdef FUNC_DEBUG_EN
		#define FUART_RX_PORT  1	//rx port  0--A1��1--B6��2--C4��0xFF--NO USE
		#define FUART_TX_PORT  1	//tx port  0--A0��1--B7��2--C3��0xFF--NO USE
	#endif

//****************************************************************************************
//                 WATCH DOG��������        
//****************************************************************************************
	#define FUNC_WATCHDOG_EN			//WATCHDOG����
	
//****************************************************************************************
//                 �ϵ���书������        
//****************************************************************************************
	#define FUNC_BREAKPOINT_EN			// ϵͳ�ϵ���Ϣ����
	#ifdef FUNC_BREAKPOINT_EN
		#define BP_SAVE_TO_NVM				//���ϵ���Ϣ���浽NVM ��
		#define BP_SAVE_TO_FLASH			// ������书�ܣ�֧�ֽ��ϵ���Ϣ���浽FLASH�У���ֹ���綪ʧ��
		//#define BP_SAVE_TO_EEPROM			// ������书�ܣ�֧�ֽ��ϵ���Ϣ���浽EEPROM�У���ֹ���綪ʧ��		
		#define FUNC_MATCH_PLAYER_BP		// ��ȡFSɨ����벥��ģʽ�ϵ���Ϣ��ƥ����ļ����ļ���ID��
	#endif


//****************************************************************************************
//                 POWER MONITOR��������        
//****************************************************************************************
// ������ܼ���(�����ڴ����ϵͳ)�Ĺ��ܺ��ѡ���
// ���ܼ��Ӱ�������ص�ѹ��⼰�͵�ѹ���ϵͳ��Ϊ�Լ����ָʾ��
// ��ص�ѹ��⣬��ָLDOIN����˵ĵ�ѹ��⹦��(���ϵͳһ�㶼�ǵ��ֱ�Ӹ�LDOIN�ܽŹ���)
// �ù��ܺ�򿪺�Ĭ�ϰ�����ص�ѹ��⹦�ܣ��йص�ص�ѹ���������ɶ�������������power_monitor.c�ļ�

//USE_POWERKEY_SLIDE_SWITCH ��USE_POWERKEY_SOFT_PUSH_BUTTON �����겻Ҫͬʱ����
	#define USE_POWERKEY_SLIDE_SWITCH     //for slide switch case ONLY
//	#define USE_POWERKEY_SOFT_PUSH_BUTTON //for soft push button case ONLY

	#define FUNC_POWER_MONITOR_EN
	
//		#define	OPTION_CHARGER_DETECT		//�򿪸ú궨�壬֧��GPIO������豸���빦��
			//�����˿�����
			#define CHARGE_DETECT_PORT_PU			GPIO_A_PU
			#define CHARGE_DETECT_PORT_PD			GPIO_A_PD
			#define CHARGE_DETECT_PORT_IN			GPIO_A_IN
			#define CHARGE_DETECT_PORT_IE			GPIO_A_IE
			#define CHARGE_DETECT_PORT_OE			GPIO_A_OE
			#define CHARGE_DETECT_BIT				(1 << 3)    


////////////////////�ڴ�������/////////////////////////
#define APP_MMM_MALLOC(MemSize, MemType)          OSMalloc(MemSize, MemType)
#define APP_MMM_REALLOC(MemPtr, MemSize, MemType) OSRealloc(MemPtr, MemSize, MemType)
#define APP_MMM_FREE(MemPtr)                      OSFree(MemPtr)

/////////////////////FLASH ����/////////////////////////

//#define FLASH_ERASE_IN_TCM			//FLASH�����������TCM��,Ŀǰֻ������GD��FLASH
#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)
#define	FS_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif/*_APP_CONFIG_H_*/
