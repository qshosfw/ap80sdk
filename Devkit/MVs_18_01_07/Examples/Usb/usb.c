#include "type.h"
#include "app_config.h"
#include "uart.h"
#include "clk.h"
#include "spi_flash.h"
#include "cache.h"
#include "gpio.h"
#include "dac.h"
#include "audio_adc.h"
#include "wakeup.h"
#include "timer.h"
#include "rtc.h"
#include "adc.h"
#include "host_hcd.h"
#include "watchdog.h"
#include "mixer.h"
#include "host_stor.h"
#include "sd_card.h"
#include "Sdio.h"
#include "device_hcd.h"
#include "device_stor.h"
#include "device_audio.h"
#include "audio_path.h"
#include "audio_adc.h"
#include "dac.h"
#include "pcm_fifo.h"
#include "pcm_transfer.h"
#include "watchdog.h"
#include "chip_info.h"

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

uint8_t Sec[1024];

//uint8_t ISODataBuf[192];
uint8_t *ISODataBuf = (uint8_t *)DEC_MEM_ADDR;
uint8_t ISOPacketLen;

uint8_t UartRec;
#define 	SetKeyValue(x)  (UartRec = (x)) 
#define 	GetKeyValue() 	(UartRec)
#define 	ClrKeyValue() 	(UartRec = 0) 

void LockSdClk(void)
{
}

void UnLockSdClk(void)
{
}

__attribute__((section(".driver.isr"))) void Timer1Interrupt(void)
{
	Timer1IntClr();
	
	OTGLinkCheck();
}

void HIDSendData(uint8_t *pBuffer,uint32_t RxLength)
{
	//TODO
	//for test
	uint8_t i;
	for(i=0;i<64;i++)
	pBuffer[i] = 64-i;
}

void HIDReceive(uint8_t *pBuffer)
{
	//TODO
	//for test
	uint8_t i;
	for(i=0;i<64;i++)
	{
		if(pBuffer[i] != i)
		{
			APP_DBG("error\n");
		}
	}
}

static __INLINE void CardDetectDelay(void)
{
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
}

bool IsCardLink(void)
{
	bool TempFlag;
	LockSdClk();
	GpioSdIoConfig(RESTORE_TO_GENERAL_IO);

	GpioClrRegBits(CARD_DETECT_PORT_PU, CARD_DETECT_BIT_MASK);
	GpioClrRegBits(CARD_DETECT_PORT_PD, CARD_DETECT_BIT_MASK);
	GpioClrRegBits(CARD_DETECT_PORT_OE, CARD_DETECT_BIT_MASK);

	GpioSetRegBits(CARD_DETECT_PORT_IE, CARD_DETECT_BIT_MASK);

	CardDetectDelay(); 

	if(GpioGetReg(CARD_DETECT_PORT_IN) & CARD_DETECT_BIT_MASK)
	{
		TempFlag = FALSE;
	}
	else
	{
		TempFlag = TRUE;
	}
	
	if(TempFlag)
	{
		GpioSdIoConfig(SD_PORT_NUM);
	}

	UnLockSdClk();

	return TempFlag;
}

void DeviceAudioOnCmd(void)
{

}

void UDiskTest()
{
	Usb2SetDetectMode(1, 0);
	UsbSetCurrentPort(2);
	while(1)
	{
		if(!UsbHost2IsLink())
		{
			continue;
		}
		if(!HostStorInit())
		{
			continue;
		}
		APP_DBG("Hardware initialize success.\n");
		break;
	}
	memset(Sec, 0, 1024);
	if(HostStorReadBlock(0, Sec, 1))
	{
		uint32_t i;
		for(i = 0; i < 512; i++)
		{
			if(!(i % 16) && i != 0)
			{
				APP_DBG("\n");
			}
			APP_DBG("%02X ", Sec[i]);
		}		
	}
	else
	{
		APP_DBG("Read Error!\n");
	}
}

void DiskReaderTest()
{
	GpioSdIoConfig(1);
	if(SdCardInit())
	{
		APP_DBG("GetCardHandle() error!\n");		
	}
	APP_DBG("Card OK!\n");
	
	Usb1SetDetectMode(0, 1);
	UsbSetCurrentPort(1);
	
	APP_DBG("MODULE_ID_USB_READER\n");
	DeviceAudioSetMode(USB_DEVICE_READER);
	DeviceStorInit();
	gDeviceVID = 0x0000;
	gDevicePID = 0x1000;
	gDeviceString_Manu = "MVSILICON";				// max length: 32bytes
	gDeviceString_Product = "USB CARD READER";		// max length: 32bytes
	gDeviceString_SerialNumber = "20130617B002";	// max length: 32bytes
	
	gDeviceString_MassStor_Vendor = "MVSI";				// max length: 8bytes
	gDeviceString_MassStor_Product = "USB CARD READER";	// max length: 16bytes
	gDeviceString_MassStor_Ver = "V2.0";				// max length: 4bytes
		
	UsbDeviceInit();
	
	while(1)
	{
		DeviceStorProcess();
	}
}

//静音使能
void DeviceAudioMuteEn(void)
{
	APP_DBG("DeviceAudioMuteEn()\n");
	//MixerMute(MIXER_SOURCE_USB);
}


//静音取消
void DeviceAudioMuteDis(void)
{
	APP_DBG("DeviceAudioMuteDis()\n");
	//MixerUnmute(MIXER_SOURCE_USB);
}

//设置音量
void DeviceAudioSetVolume(void)
{
	//SetSysVol();
}


void DacInitialize()
{
	CodecDacInit(1);                                    //dac init 
	
	CodecDacChannelSel(DAC_CH_DECD_L | DAC_CH_DECD_R);
    DacConfig(MDAC_DATA_FROM_DEC, USB_MODE);
    DacAdcSampleRateSet(44100, USB_MODE);
    DacVolumeSet(0xef0, 0xef0); 
    
    PcmFifoPause();
    PcmFifoInitialize(PCM_FIFO_ADDR-PMEM_ADDR, PCM_FIFO_LEN, 0, 0);
    PcmTxSetPcmDataMode(DATA_MODE_STEREO_LR);
    PcmFifoPlay();
	
	PhubPathClose(ALL_PATH_CLOSE);
	PhubPathSel(PCMFIFO_MIX_DACOUT);
	DacSoftMuteSet(FALSE, FALSE); 
}

void DacSetData(uint8_t *Buf, uint8_t Len)
{
	//if(PcmTxIsTransferDone())
	{                                             
		PcmTxTransferData(Buf, Buf, Len/4); 
	}
}

void OnDeviceAudioRcvIsoPacket(void)
{		
	ISOPacketLen = UsbDeviceIsoRcv(ISODataBuf, 192);
	DacSetData(ISODataBuf, ISOPacketLen);
}

void OnDeviceAudioSendIsoPacket(void)
{
	UsbDeviceIsoSend(ISODataBuf, ISOPacketLen);
}

void AudioTest()
{
	ISOPacketLen = 192;
	NVIC_EnableIRQ(USB_IRQn);
	UsbSetCurrentPort(PC_PORT_NUM);		// 选择USB端口
	
	// 单独USB声卡模式，PC识别出USB声卡
	APP_DBG("MODULE_ID_USB_AUDIO\n");
	DeviceAudioSetMode(USB_DEVICE_AUDIO);
	gDeviceVID = 0x0000;
	gDevicePID = 0x0201;
	gDeviceString_Manu = "MVSILICON";					//max length: 32bytes
	gDeviceString_Product = "MV USB AUDIO";				//max length: 32bytes
	gDeviceString_SerialNumber = "20130617A002";		//max length: 32bytes
	
	//USB声卡模式下，回调函数指针初始化
	gFpDeviceAudioMuteEn = DeviceAudioMuteEn;			// PC端执行MUTE命令时，回调此函数
	gFpDeviceAudioMuteDis = DeviceAudioMuteDis;			// PC端执行UNMUTE命令时，回调此函数
	gFpDeviceAudioSetVolume = DeviceAudioSetVolume;		// PC端改变音量时，回调此函数
	
	UsbDeviceInit();
	
	DacInitialize();
	
	while(1)
	{
		DeviceAudioProcess();
	}
}

int32_t main(void)
{
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating
	
	ClkPorRcToDpll(0); 		//clock src is 32768hz OSC
	CacheInit();	
	SysTickInit();
	
	//init timer0/1 interrupt
	NVIC_EnableIRQ(TMR1_IRQn);
	Timer1Set(1000);
	
	WdgDis();
	GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);
	FuartInit(115200, 8, 0, 1);

    APP_DBG("/==========================================================================\\\n");
    APP_DBG("|              				USB TESTBENCH                    			|\n");
    APP_DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    APP_DBG("\\==========================================================================/\n");

	APP_DBG("please input command as the following\n");
	APP_DBG("Command   Discription\n");
	APP_DBG("u   		U盘测试读取0扇区\n");
	APP_DBG("r   		读卡器测试\n");
	APP_DBG("a   		声卡测试n");
	APP_DBG("\n");
	while(1)
	{
		while(!GetKeyValue());
		switch(GetKeyValue())
		{
			case 'u':
				UDiskTest();				
				break;
			case 'r':
				DiskReaderTest();
				break;
			case 'a':
				AudioTest();
				break;
			default:
				break;
		}
		ClrKeyValue();
		APP_DBG("\n");
		APP_DBG("please input command as the following\n");
		APP_DBG("Command   Discription\n");
		APP_DBG("u   		U盘测试读取0扇区\n");
		APP_DBG("r   		读卡器测试\n");
		APP_DBG("a   		声卡测试n");
		APP_DBG("\n");
	}
}

void FuartInterrupt(void)
{
    if(IsFuartRxIntSrc())
    {
        uint8_t c;
        FuartRecvByte(&c);
        FuartClrRxInt(); 
        SetKeyValue(c);
	}
	if(IsFuartTxIntSrc())
    {
        FuartClrTxInt();
	}
}

