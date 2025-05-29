//  maintainer: Halley
#include "type.h"
#include "app_config.h"
#include "os.h"
#include "msgq.h"
#include "app_msg.h"
#include "uart.h"
#include "key.h"
#include "clk.h"
#include "spi_flash.h"
#include "cache.h"
#include "gpio.h"
#include "dac.h"
#include "audio_adc.h"
#include "wakeup.h"
#include "timer.h"
#include "sys_app.h"
#include "power_management.h"
#include "rtc.h"
#include "adc.h"
#include "ir.h"
#include "host_hcd.h"
#include "host_stor.h"
#include "watchdog.h"
#include "mixer.h"
#include "breakpoint.h"
#include "dev_detect_driver.h"
#include "sys_vol.h"
#include "eq.h"
#include "lcd_seg.h"
#include "eq_params.h"
#include "sound_remind.h"
#include "sw_uart.h"
#include "debug.h"
#include "bt_stack_api.h"
#include "audio_decoder.h"
#include "get_bootup_info.h"
#include "fsinfo.h"
#include "sys_vol.h"
#include "micro_adjust.h"
#include "fat_file.h"
#include "task_decoder.h"
#include "presearch.h"

int32_t BtTaskHandle = 0;
int32_t MainTaskHandle = 0;
xTaskHandle AudioProcessHandle;

extern SwEqContext * p_gSwEq;
#ifdef FLASH_ERASE_IN_TCM
SPI_FLASH_INFO gFlashInfo;
#endif

#ifdef FUNC_SPI_UPDATE_EN
extern void BootUpgradeChk(void);
#endif
extern void AudioProcessTaskEntrance(void);

//extern uint8_t UpgradeFileFound;

extern void DetectMassTestCondition(void);
extern bool GetMassTestFlag(void);

extern bool FlshBTInfoAreaInit(void);

__attribute__((section(".driver.isr"))) void Timer1Interrupt(void)
{
	Timer1IntClr();
#if (defined(FUNC_USB_EN) || defined(FUNC_USB_AUDIO_EN) || defined(FUNC_USB_READER_EN) || defined(FUNC_USB_AUDIO_READER_EN))
	OTGLinkCheck();
#endif
	SystemPowerOffDetect();
}

////ע��boot�汾�������boot5.0�汾
//GPIOA0��Ϊ��ͨGPIOʹ��ʱ����main�ͷ����ʹ��
int32_t main(void)
{
	extern void GuiTaskEntrance(void);
//	extern void DecoderTaskEntrance(void);
	extern void BTEntrance(void);
	extern void ShellCommand(void);
	
	//boot5.0оƬ��GPIO��Ϊ��ͨGPIOʹ��ʱ������������д���
	//GpioA0SetMode(A0_FOR_GPIO);
	
#ifdef FUNC_5VIN_TRIM_LDO3V3   	
	SarAdcTrimLdo3V3();   //attention! only used in Power = 5V
#endif
		
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating
	CacheInit();
	
//	SysGotoPowerDown();

	SysGetWakeUpFlag(); //get wake up flag, DO NOT remove this!!

#ifdef USE_POWERKEY_SLIDE_SWITCH
	SysPowerKeyInit(POWERKEY_MODE_SLIDE_SWITCH, 500);//500ms
#endif
#ifdef USE_POWERKEY_SOFT_PUSH_BUTTON
	SysPowerKeyInit(POWERKEY_MODE_PUSH_BUTTON, 2000); //2s
#endif

	SpiFlashInfoInit();		//Flash RD/WR/ER/LOCK initialization
#ifdef FLASH_ERASE_IN_TCM
	SpiFlashGetInfo(&gFlashInfo);
#endif
 
	ClkPorRcToDpll(0); 		//clock src is 32768hz OSC
	
	//GD flash��ѡ��SYS CLK��DPLL80Mhz�����Կ���HPM�������ͺŽ���ر�HPM
	SpiFlashClkSet(FLASHCLK_SYSCLK_SEL, TRUE);
	
#ifdef FUNC_RADIO_DIV_CLK_EN
	ClkDpllClkGatingEn(1);
#endif
	SarAdcLdoinVolInit();    //do NOT remove this
	LcdCtrlRegInit();        //do NOT remove this
	/*
	 * Now, uart has been initialized automatically by OS.
	 * if you want to customize debug uart setting(defaut 115200-8-N-1),
	 * you can invoke the function at any place you want(for example here).
	 */
#ifdef FUNC_DEBUG_EN
//	SwUartTxInit(SWUART_GPIO_PORT_A, 24, 115200);
//	EnableSwUartAsFuart(TRUE);

	OsSetDebugFlag(1);
	GpioFuartRxIoConfig(FUART_RX_PORT);
	GpioFuartTxIoConfig(FUART_TX_PORT);
	OSDBGUartInit(115200, 8, 0, 1);
#else
	OsSetDebugFlag(0);
#endif


#if (!defined(FUNC_USB_AUDIO_EN) && !defined(FUNC_USB_READER_EN) && !defined(FUNC_USB_AUDIO_READER_EN))
#undef PC_PORT_NUM
#define PC_PORT_NUM    0
#endif

#ifdef FUNC_SPI_UPDATE_EN
	BootUpgradeChk();	//boot upgrade check upgrade for the last time
#endif

	Osc32kExtCapCalibrate();//32KHz external oscillator calibration
#ifdef FUNC_BREAKPOINT_EN
	BP_LoadInfo();// ��顢װ�أ������һ���ϵ磩����������Ϣ��DO NOT remove this code!!!
#endif

	OSStartKernel();                //start initialize os kernel

#ifdef MASS_PRODUCTION_TEST_FUNC
	DetectMassTestCondition();
#endif

	OSStartSchedule();              //Start the scheduler.

	OSTaskPrioSet(NULL, 4);

	FlashUnlock();
	//FlashLock(FLASH_LOCK_RANGE_HALF);		// if need, Lock range setting see function description
	
	SysVarInit();                  //ȫ�ֱ�����ʼ����ע����BP_LoadInfo֮�����

#if (defined(FUNC_ADC_KEY_EN) || defined(FUNC_IR_KEY_EN) || defined(FUNC_CODING_KEY_EN))
	KeyInit();
#endif
	//GpioSdIoConfig(SD_PORT_NUM);    // set sd port
#ifdef FUNC_CARD_EN
	IsCardLink();
#endif
#ifdef FUNC_USB_EN
	Usb1SetDetectMode((UDISK_PORT_NUM == 1), (PC_PORT_NUM == 1)); // usb1 port host/device mode set
	Usb2SetDetectMode((UDISK_PORT_NUM == 2), (PC_PORT_NUM == 2)); // usb2 port host/device mode set
	UsbSetCurrentPort(UDISK_PORT_NUM); // set usb host port
#endif
	InitDeviceDetect();                 // �ϵ�ʱ���豸״̬ɨ������

//#ifdef FUNC_RTC_EN
//	RtcInit(AlarmWorkMem, sizeof(AlarmWorkMem));
//	NVIC_EnableIRQ(RTC_IRQn);           // �����������ж�
//#endif

	// SystemOn();
//������ܿ��ƹ��ܴ򿪣��˴���������1�ε͵�ѹ���ͳ���豸�������
#ifdef FUNC_POWER_MONITOR_EN
	PowerMonitorInit();
#endif
	//DacNoUseCapacityToPlay();

	AudioAnaInit();	//ģ��ͨ����ʼ��
	DacVolumeSet(DAC_DIGITAL_VOL, DAC_DIGITAL_VOL);
	AdcVolumeSet(ADC_DIGITAL_VOL, ADC_DIGITAL_VOL);
	MixerInit((void*)PCM_FIFO_ADDR, PCM_FIFO_LEN);
    AudioOutputInit();//���ͨ����ʼ��
    
#ifdef FUNC_MIXER_SRC_EN
	MixerSrcEnable(TRUE);
#else
	MixerSrcEnable(FALSE);
#endif
	MixerSetFadeSpeed(MIXER_SOURCE_DEC, DEC_FADEIN_TIME, 10);

	EqStyleInit(p_gSwEq);
	EqStyleSelect(p_gSwEq, 44100, gSys.Eq);
//#ifdef FUNC_TREB_BASS_EN
//	TrebBassSet(gSys.TrebVal, gSys.BassVal);
//#endif
#ifdef FUNC_SOUND_REMIND
	SoundRemindInit();
#endif
#ifdef FUNC_SOFT_ADJUST_EN
    SoftAdjustInit(2);
#endif
	SetDriverTerminateFuc(GetQuickResponseFlag);
	SetFsTerminateFuc(GetQuickResponseFlag);
	
	APP_DBG("****************************************************************\n");
	APP_DBG("System Clock     :%d MHz(%d)\n", ClkGetCurrentSysClkFreq() / 1000000, ClkGetCurrentSysClkFreq());
	APP_DBG("Flash Clock      :%d MHz(%d)\n", ClkGetFshcClkFreq() / 1000000, ClkGetFshcClkFreq());
	APP_DBG("BOOT Version     :%d.%d.%d%c\n", GetBootVersion(), GetPatchVersion() / 10, GetPatchVersion() % 10,GetBootVersionEx());
	APP_DBG("SDK  Version     :%d.%d.%d\n", (GetSdkVer() >> 8) & 0xFF, (GetSdkVer() >> 16) & 0xFF, GetSdkVer() >> 24);
	APP_DBG("Free Memory      :%d(%d KB)\n", OSMMMIoCtl(MMM_IOCTL_GETSZ_INALL, 0), OSMMMIoCtl(MMM_IOCTL_GETSZ_INALL, 0) / 1024);
	APP_DBG("Code Size        :%d(%d KB)\n", GetCodeSize(), GetCodeSize() / 1024);
	APP_DBG("Code Encrypted   :%s\n", GetCodeEncryptedInfo());
	APP_DBG("Wakeup Source    :%s(0x%08X)\n", GetWakeupSrcName(gWakeUpFlag), gWakeUpFlag);
	APP_DBG("BT Lib Ver       :%s\n", GetLibVersionBt());
	APP_DBG("AudioDecoder Ver :%s\n", GetLibVersionAudioDecoder());
	APP_DBG("Driver Ver       :%s\n", GetLibVersionDriver());
	APP_DBG("FreeRTOS Ver     :%s\n", GetLibVersionFreertos());
	APP_DBG("FS Ver           :%s\n", GetLibVersionFs());
	APP_DBG("****************************************************************\n");

	FlshBTInfoAreaInit();	//BT data aera init. do NOT move this
	
#ifdef	CFG_SHELL_DEBUG
	OSTaskCreate(ShellCommand, "SHELL", 720, NULL, 3, NULL);
#endif	//CFG_SHELL_DEBUG
	OSTaskCreate(DecoderTaskEntrance, "Decoder", 2048, NULL, 3, NULL);
	OSTaskCreate(GuiTaskEntrance, "MainTask", 1280, NULL, 2, &MainTaskHandle);
	OSTaskCreate(AudioProcessTaskEntrance, "AudioProcessTask", 1024, NULL, 3, &AudioProcessHandle);
#ifdef FUNC_BT_EN
	OSTaskCreate(BTEntrance, "BT", 1920, NULL, 3, &BtTaskHandle);
#endif
	
	//init timer0/1 interrupt
	NVIC_EnableIRQ(TMR1_IRQn);
	Timer1Set(1000);

	DBG("Start Detect External Device(Keypad, U disk, SD card, FM,...)\n");
	OSQueueMsgIOCtl(MSGQ_IOCTL_DEL_PENDMSG, 0);
#ifndef FUNC_WATCHDOG_EN
	WdgDis();				// disable watch dog
#endif
	while(1)
	{
		//feed watch dog ever 10 ms
#ifdef FUNC_WATCHDOG_EN
		WdgFeed();
#endif
#ifdef FUNC_POWER_MONITOR_EN
		//ִ�е�ص�ѹ��⡢����豸�����⼰�丽������ʾ��ϵͳ�رմ����
		PowerMonitor();
#endif
#ifdef FUNC_SPI_UPDATE_EN
		if(UpgradeFileFound)
		{
			//upgrade file found,try to upgrade it
			BootUpgradeChk();
		}
#endif
		MsgRecv(10);    
#if (defined(FUNC_ADC_KEY_EN) || defined(FUNC_IR_KEY_EN) || defined(FUNC_CODING_KEY_EN))
		KeyScan();
#endif
#ifdef MASS_PRODUCTION_TEST_FUNC
		if(GetMassTestFlag())
		{
			continue;
		}
#endif
		DeviceDetect(); // �豸���ӿ�
	}
}


