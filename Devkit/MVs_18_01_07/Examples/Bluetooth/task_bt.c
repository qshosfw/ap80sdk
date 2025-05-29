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
#include "rst.h"
#include "clk.h"
#include "dac.h"
#include "uart.h"
#include "gpio.h"
#include "irqs.h"
//#include "debug.h"
#include "app_config.h"
#include "chip_info.h"
#include "spi_flash.h"
#include "pcm_fifo.h"
#include "pcm_sync.h"
#include "pcm_transfer.h"
#include "audio_path.h"
#include "audio_decoder.h"
#include "bt_stack_api.h"
#include "bt_control_api.h"
#include "bt_device_type.h"


#define  PCM_FIFO_SBC_SAMPLES   3072    //Sbc解码器 播放端FiFo大小 单位DWORD
#define  MAX_BT_DEV_NAME_LENGTH 50      //蓝牙名称长度

static uint8_t BtDevName[MAX_BT_DEV_NAME_LENGTH] = "BT_112358";   //蓝牙名称
static uint8_t BtAddr[6] = {136,4,11,1,35,15};                    //蓝牙地址

static bool A2dpCodecInit(void)
{
    SongInfo *CurSongInfo;

    RstDecModule();

    if(audio_decoder_initialize((uint8_t*)VMEM_ADDR, GetSbcStreamHandle(), IO_TYPE_MEMORY, SBC_DECODER) != RT_SUCCESS)
    {
        return FALSE;
    }

    PcmTxInitWithDefaultPcmFifoSize((PCM_DATA_MODE)audio_decoder_get_song_info()->pcm_data_mode);
    PcmFifoInit(0, PCM_FIFO_SBC_SAMPLES * 4);
        
    NVIC_EnableIRQ(DECODER_IRQn);
    
    PhubPathClose(ALL_PATH_CLOSE);

    if(!PhubPathSel(PCMFIFO_MIX_DACOUT))
    {
        return FALSE;      
    }

    CodecDacChannelSel(DAC_CH_DECD_L|DAC_CH_DECD_R);
    DacConfig(MDAC_DATA_FROM_DEC, USB_MODE);

    CurSongInfo = audio_decoder_get_song_info();

    if(CurSongInfo)
    {
        DBG("CurSongInfo->sampling_rate %d\r\n",CurSongInfo->sampling_rate);
        DacAdcSampleRateSet(CurSongInfo->sampling_rate, USB_MODE);	 //九种采样率
    }

    SetSbcPlayCfgFlag(TRUE);
    PcmFifoPause();
    CodecDacMuteSet(FALSE, FALSE); 

    return TRUE;
}

static void DacAdjust(void)
{
    if(1 == GetSbcPlayCfgFlag())//A2DP
    { 
        if(GetRcvListItemNum()< 5 || PcmFifoGetRemainSamples()< PCM_FIFO_SBC_SAMPLES - 128)
        {
            DacSampRateAdjust(TRUE, -20000);//DacAdjustSlower();
        }
        else if(GetRcvListItemNum()>= 5 && (PcmFifoGetRemainSamples() > PCM_FIFO_SBC_SAMPLES - 50))
        {
            DacSampRateAdjust(TRUE, 5000);  //DacAdjustFaster();
        }
        else 
        {

        }
    }
    else //IDLE or HF/HS
    {
        DacSampRateAdjust(TRUE, 0);         //DacAdjustNormal();
    }
}

static void BtDecordSbcData(void)
{
    //decode and playing if have sbc data
    if(GetHasSbcDataFlag() && (RT_YES == audio_decoder_can_continue()))
    {                                             
        if(RT_SUCCESS == audio_decoder_decode())
        {                                         
            if(PcmFifoIsEmpty())
            {   
                DBG("E %d %d %d\n",GetRcvListItemNum(), PcmFifoGetRemainSamples(), BuartIOctl(BUART_IOCTL_RXFIFO_DATLEN_GET,0));
            }

            audio_decoder_clear_pcm_transfer_done();
            while(!audio_decoder_check_xr_done());
            audio_decoder_clear_xr_done();
            audio_decoder_start_pcm_transfer();
            while(RT_YES != audio_decoder_check_pcm_transfer_done());
        }
        else
        {
            if(audio_decoder_get_error_code() == SBC_ERROR_STREAM_EMPTY)
            {
                SetSbcDecDoneFlag();//inform bt stack that it can refill data now.
            }
            else
            {
                DBG("get_error_code() %d\r", audio_decoder_get_error_code());
            }
        }
    }
}

static void BtSbcDataProcess(void)
{
    //init sbc decorder
    if(0 == GetSbcPlayCfgFlag()&& GetHasSbcDataFlag())//decoding not started yet
    {
        if(A2dpCodecInit())
        {
            DBG("Sbc Decorder Init Ok!\n");
        }
        else
        {
            DBG("Sbc Decorder Init Fail!\n");
        }
    }

    //process sbc data
    BtDecordSbcData();

    // start play
    if(PcmFifoGetStatus()==1/*PCM FIFO PAUSE*/ && PcmFifoGetRemainSamples()/*单位DWORD*/ >= PCM_FIFO_SBC_SAMPLES - 128)
    {
        PcmFifoPlay(); 
    }

    // Adjust dac sampRate
    DacAdjust();
}

static void BTStackLoop(void)
{
    while(1)
    {                         
        BTStackRun();           //协议栈事件收发、处理等
        PcmSyncDataProcess();   //HF/HS 电话进入时Pcm数据处理
        BtSbcDataProcess();     //A2DP  蓝牙音频时获取Sbc数据并交Decorder处理        
    }
}

extern void ObexCbFunc(const ObexCallbackInfo *Info);

void BTEntrance(void)
{
    BTDevicePinCFG();

    if(BTDeviceInit(BT_DEVICE_TYPE, (int8_t*)BtAddr))
    {
    	if(!BTStackRunInit(BT_DEVICE_TYPE, BtDevName, BT_FEATURE_SUPPORTED_NORMAL))            
        {
            DBG("BtStackInit Fail!\n");
        }
    	else
    	{
    		DBG("BtStackInit Ok!\n");
    	}

        SpiFlashInfoInit();

        BTStackInitObex(ObexCbFunc);
        BTStackLoop();//Loop until exit
        BTStackRunEnd();              
    }
    else
    {
        DBG("BTDeviceInit Fail!\n");
    }     
}
