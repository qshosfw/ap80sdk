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
//                  ϵͳĬ����������         
//****************************************************************************************
	#define MAX_VOLUME  32
	#define DEFAULT_VOLUME  26
	#define DEC_FADEIN_TIME					1000	//fadeinʱ��	
	

//****************************************************************************************
//                  Ƶ�ײɼ�����        
//****************************************************************************************
	#define FUNC_SPEC_TRUM_EN                      	//��Ƶ�׹��ܿ���
	

//****************************************************************************************
//                  ϵͳ����������            
//****************************************************************************************
//ʹ�ܸú��ʾMIXERģ���Զ������ֲ�����ͳһת��Ϊ44.1KHZ
//��������OK���Ӧ��ʱ��ǿ�ҽ���ʹ�ܸú�
//ʹ�ܸú��MIXERģ��Է�44.1KHZ�����ʵ�������������ת������Ƶ�źŻ�����˥��
//	#define FUNC_MIXER_SRC_EN   					
													
//****************************************************************************************
//                  ���΢������       
//****************************************************************************************
//ʹ�ܸú꣬�����Normalģʽ���ʱ���������΢��
//  #define FUNC_SOFT_ADJUST_EN

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
//                  MIC��������                        
//****************************************************************************************
	#define FUNC_MIC_EN								//MIC����
	#ifdef FUNC_MIC_EN
		#define FUNC_MIC_ALC_EN							//MIC ALC���ܿ���
		#define MAX_MICIN_VOLUME				16		//MIC�����ɵ��ļ�����0--16
		#define DEFAULT_MICIN_VOLUME			10		//MIC����Ĭ��ֵ
		#define MICIN_ANA_GAIN_VAL				20		//MIC����ͨ·ģ������
		#define FUNC_MIC_DET_EN  						//MIC��μ�ⶨ��
		#ifdef FUNC_MIC_DET_EN
			#define	MICIN_DETECT_PORT_IN		GPIO_A_IN
			#define	MICIN_DETECT_PORT_IE		GPIO_A_IE
			#define	MICIN_DETECT_PORT_OE		GPIO_A_OE
			#define	MICIN_DETECT_PORT_PU		GPIO_A_PU
			#define	MICIN_DETECT_PORT_PD		GPIO_A_PD
			#define MICIN_DETECT_BIT			(1 << 0)
		#endif

		#define FUNC_MIC_ECHO_EN						//MIC ECHO����
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
//                  ����ģʽ����                        
//****************************************************************************************
	#define FUNC_PLAYER_EN   						//U��/SD�����Ź���
		#define UDISK_PORT_NUM		        2		//USB�˿ڶ���
		#define	SD_PORT_NUM                 1		//SD���˿ڶ���
		#define FUNC_FOLDER_PLAY_EN         		//֧���ļ��в���ģʽ

//****************************************************************************************
//                 �ļ����ģʽ����        
//****************************************************************************************
	#define FUNC_BROWSER_EN  				//�ļ��������

//****************************************************************************************
//                 ����ģʽ����        
//****************************************************************************************
	#define FUNC_BT_EN       								//��������
	#ifdef FUNC_BT_EN
		#define BT_DEVICE_TYPE  BTUartDeviceMTK662X				//BTUartDeviceRda5875 // bt chip type

		#define FUNC_BT_HF_EN									//bt hf call module
		#ifdef FUNC_BT_HF_EN
			#define MAX_BTHF_VOLUME 					(15)	//������������ȼ�
		#endif

		#define BT_RECONNECTION_FUNC							// �����Զ���������
		#ifdef BT_RECONNECTION_FUNC
			#define RECONNECTION_TRY_COUNTS				(7)		// �Զ���������
			#define RECONNECTION_PERIOD_TIME			(3)		// �������ʱ��(in seconds)
		#endif

//		#define FAST_POWER_ON_OFF_BT							// ���ٴ�/�ر���������
	#endif
//****************************************************************************************
//                 FM������ģʽ����        
//****************************************************************************************
	#define FUNC_RADIO_EN						//Fm Radio
	#ifdef FUNC_RADIO_EN
		#define FMIN_ANA_GAIN_VAL			9	//FM����ͨ����ģ�����棬�����ΪLineInʹ����Ҫ����Ϊ18������ʧ��			
//		#define FUNC_RADIO_DIV_CLK_EN			//FM��Ƶ����

		#define FUNC_RADIO_RDA5807_EN
//		#define FUNC_RADIO_RDA5876A_EN
//		#define FUNC_RADIO_BK1080_EN
//		#define FUNC_RADIO_QN8035_EN
//		#define FUNC_RADIO_KT0830_EN

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
		#define LINEIN_ANA_GAIN_VAL			9	//LINEIN����ͨ����ģ������
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
	#define FUNC_RECORD_EN   				//¼������
	#ifdef FUNC_RECORD_EN
		#define FUNC_RECORD_FILE_DELETE_EN 		//֧��¼��ɾ������
		#define FUNC_REC_FILE_REFLESH_EN 		//��ʱ�����ļ�����������¼�����ݲ��ܱ�������⣩
		#define FUNC_REC_PLAYBACK_EN 			//¼���طŹ���
		#define FUNC_PLAY_RECORD_FOLDER_EN		//����ģʽ����record�ļ���
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
		#define FUNC_APP_DEBUG_EN
		#define FUNC_FS_DEBUG_EN
		#define FUART_RX_PORT  1	//rx port  0--A1��1--B6��2--C4��0xFF--NO USE
		#define FUART_TX_PORT  1	//tx port  0--A0��1--B7��2--C3��0xFF--NO USE
	#endif

//****************************************************************************************
//                 WATCH DOG��������        
//****************************************************************************************
	#define FUNC_WATCHDOG_EN			//WATCHDOG����


//****************************************************************************************
//                 3D��Ч��������        
//****************************************************************************************
	//AUDIO_3D_EFFECT_VAL note: Q1.15 format to represent value in range [0,1). For example, 16384 represents 0.5 for cross_gain.
	//0x5000 represents 0.65 for cross_gain.
	//0x7800 represents 0.9375 for cross_gain.
	#define FUNC_AUDIO_3D_EFFECT_EN
		#define AUDIO_3D_EFFECT_VAL         0x7800	// 3DЧ������
		#define AUDIO_3D_AUTO_LEVEL_CONTROL TRUE	// 3DЧ������
		#define LINEINMIC_OR_FMMIX_3D_EFFECT_EN		// ֧��line_mic_guitar ��fm_mic_guitar �첥ʱ�� 3DЧ��

//****************************************************************************************
//                 ������ʾ����������        
//****************************************************************************************
	#define FUNC_SOUND_REMIND		//������ʾ

//****************************************************************************************
//                 �ϵ���书������        
//****************************************************************************************
	#define FUNC_BREAKPOINT_EN			// ϵͳ�ϵ���Ϣ����
	#ifdef FUNC_BREAKPOINT_EN
		#define FUNC_NVM_TO_FLASH_EN		// ������书�ܣ�֧�ֽ�NVM���ݱ��浽FLASH�У���ֹ���綪ʧ��
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
			#define CHARGE_DETECT_PORT_PU			GPIO_E_PU
			#define CHARGE_DETECT_PORT_PD			GPIO_E_PD
			#define CHARGE_DETECT_PORT_IN			GPIO_E_IN
			#define CHARGE_DETECT_PORT_IE			GPIO_E_IE
			#define CHARGE_DETECT_PORT_OE			GPIO_E_OE
			#define CHARGE_DETECT_BIT				(1 << 3)


//****************************************************************************************
//                 ������������        
//****************************************************************************************
	#define FUNC_KEY_BEEP_SOUND_EN    						//����beep������

	//ADC KEY����
	#define FUNC_ADC_KEY_EN								//ADC KEY�꿪��  									
		#define ADC_KEY_PORT_CH1	ADC_CHANNEL_B22			//���ֻʹ��1·ADC��������������
		#define	ADC_KEY_PORT_CH2	ADC_CHANNEL_B5			//���ֻʹ��2·ADC��������������

	//IR KEY����
	#define FUNC_IR_KEY_EN   							//IR KEY�꿪��
		#define IR_KEY_PORT			IR_USE_GPIOC2			//IRʹ�õĶ˿ڣ�IR_USE_GPIOA10, IR_USE_GPIOB7 or IR_USE_GPIOC2

	//CODING KEY����
	#define FUNC_CODING_KEY_EN							//CODING KEY�꿪��			
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
//                 ��Ƶ�˵�����        
//****************************************************************************************
	#define FUNC_AUDIO_MENU_EN						//���������ܸ���ѡ��MIC������������ʱ������ǿ�ȡ�BASS���ڡ�TREB����
		#define		MENU_TIMEOUT		5000		//����˵����ں����5��û�а�MENU��VOL_UP��VOL_DW�����Զ��˳��˵�ģʽ

//****************************************************************************************
//                 ϵͳģʽֵ����        
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

    // ����Ӧ��ģʽ������ͨ��mode��ѭ���л�����
    //////////////////////////////////////////////////////////////
    // DO NOT MODIFY FOLLOWINGS
    MODULE_ID_END,				// end of moduleID
    // ����Ӧ��ģʽ����ͨ��mode���л����룬���Ǹ���ʵ����; ͨ����ݼ���������������ʱ�Ż����

    //MODULE_ID_ALARM_REMIND,		// alarm remind
    MODULE_ID_POWEROFF,			// faked module ID for Power OFF
    MODULE_ID_STANDBY,			// faked module ID for StandBy mode
#ifdef FUNC_REC_PLAYBACK_EN
    MODULE_ID_REC_BACK_PLAY,	// ¼���ط�ģʽ
#endif
    MODULE_ID_USER_PLUS,		// �û���չ��ʼID
};


//****************************************************************************************
//                 MEM����        
//****************************************************************************************
//DEC
#define DEC_MEM_ADDR              		(VMEM_ADDR + 0 * 1024)		//decoder 28KB
#define DEC_MEM_MAX_SIZE          		(28 * 1024)            

//ENC
#define ENC_MEM_ADDR					(VMEM_ADDR + 1 * 1024)		//encoder 17KB mp3���� ���巽ʽ ��ʼ1k��pcm_sync.c�ж����ͻ  mp3ֻ��16k  
#define ENC_MEM_SIZE					(17 * 1024)

//ENC BUF
#define MP3_OUT_SAMPLES_BUF             (VMEM_ADDR + 18 * 1024)		//record buf 19KB
#define MP3_OUT_SAMPLES_BUF_SIZE        (19 * 1024)


//���漸��FIFO������PMEM�У�������(VMEM_ADDR + 32 * 1024)
//PCM FIFO
#define PCM_FIFO_ADDR             		(VMEM_ADDR + 37 * 1024)		//pcm fifo 16KB
#define PCM_FIFO_LEN              		(16 * 1024)

//ADC FIFO
#define ADC_FIFO_ADDR         			(VMEM_ADDR + 53 * 1024)		//adc fifo 4KB
#define ADC_FIFO_LEN             		(4 * 1024)         

//BUART
#define BUART_RX_TX_FIFO_ADDR  			(VMEM_ADDR + 57 * 1024)		//buart fifo 7KB��RX��TX��������
#define BUART_RX_FIFO_SIZE             	(6 * 1024)         
#define BUART_TX_FIFO_SIZE             	(1 * 1024)         


////////////////////�ڴ�������/////////////////////////
#define APP_MMM_MALLOC(MemSize, MemType)          OSMalloc(MemSize, MemType)
#define APP_MMM_REALLOC(MemPtr, MemSize, MemType) OSRealloc(MemPtr, MemSize, MemType)
#define APP_MMM_FREE(MemPtr)                      OSFree(MemPtr)


#include "debug.h"

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif/*_APP_CONFIG_H_*/
