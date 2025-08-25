/******************************************************************************
 * @file    mixer.c
 * @author  Orson
 * @version V1.0.0
 * @date    29-April-2014
 * @brief   audio mixer
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#include <string.h>
#include "type.h"
#include "delay.h"
#include "timeout.h"
#include "pcm_transfer.h"
#include "pcm_fifo.h"
#include "codec_mixer.h"
#include "audio_path.h"
#include "dac.h"
#include "clk.h"
#include "audio_adc.h"
#include "chip_info.h"
#include "rst.h"


typedef struct _MIXER_INFO
{
	uint16_t	SampleRate;

	uint16_t 	PcmFormat;
	uint16_t 	MixedIndex;		//MixerBuf中当前MIX到的位置

	int16_t*	InBuf;			//尚未MIX到MixerBuf中的数据缓冲区指针
	uint16_t 	InCnt;			//尚未MIX到MixerBuf中的数据长度

	bool		EmptyFlag;		//空标志

} MIXER_INFO;

static MIXER_INFO MixerInfo;

static int16_t* MixerBuf = NULL;
static uint16_t N = 0;			//单位：采样点数
static uint16_t W = 0;			//PCM FIFO的硬件写指针

static void MixerProcess(void);


#define FIFO_LEN(StartIndex, EndIndex)	((EndIndex + N - StartIndex) % N)

//检查某个通道是否播空
static void MixerCheckEmpty(void)
{
	uint16_t R = PcmFifoGetReadAddr();
	uint16_t M = MixerInfo.MixedIndex;
	if(FIFO_LEN(R, M) > FIFO_LEN(R, W))
	{
		MixerInfo.MixedIndex = R;
		if(!MixerInfo.EmptyFlag)
		{
			MixerInfo.EmptyFlag = TRUE;
			//DBG("E[%d]\n", SourceID);
		}	
	}
}


//通过TX模块将PCM FIFO的写指针向后移动，TX实际传送的是0数据
static void MixerCheckTx(void)
{
	uint16_t Cnt;

	Cnt = PcmFifoGetDepth() - PcmFifoGetRemainSamples();
	if(Cnt > 32)
	{
	 	Cnt -= 16;
		PcmTxSetGain(0);
		PcmTxDoneSignalClear();
		PcmTxTransferData((void*)VMEM_ADDR, (void*)VMEM_ADDR, Cnt);
		
		while(1)
		{
			if(PcmTxIsTransferDone())
			{
			 	break;
			}
		}
		PcmTxDoneSignalClear();

        MixerCheckEmpty();		//检查某个通道是否播空
		//W = (W + Cnt) % N;		//有漏洞，当DAC采样率变化时，硬件真实的W值会突变
		//获取硬件真实的W位置
		N = PcmFifoGetDepth();
		W = (PcmFifoGetReadAddr() + PcmFifoGetRemainSamples()) % N;
	}
}

//检查各输入通路的缓冲区中是否有数据空余MIX到MixerBuf中
//需要满足下面几个条件：
//1. 输入缓冲区中有数据
//2. MixerBuf中有空余
static void MixerCheckMix(void)
{
 	uint16_t j;
	uint16_t n;		
	int16_t* x;				//MIX前缓冲区临时指针
	int16_t* y;				//MIX后缓冲区临时指针

	
	uint16_t M;
	

		//检查某个通道是否播空
		MixerCheckEmpty();

		//计算FIFO中剩余可MIX的空间长度,即从M到W的长度
		M = MixerInfo.MixedIndex;
		n = (M <= W) ? FIFO_LEN(M, W) : FIFO_LEN(M, N);

		if(n > MixerInfo.InCnt)
		{
		 	n = MixerInfo.InCnt;
		}

		x = MixerInfo.InBuf;
		y = &MixerBuf[M * 2];
		
		if(MixerInfo.PcmFormat == MIXER_FORMAT_MONO)
		{	
			//单声道，与y[]中原有数据MIX
            for(j = 0; j < n; j++)
            {
                y[2 * j + 1] = y[2 * j + 0] = x[j];
                 
            } 
		}
		else
		{
			//双声道，与y[]中原有数据MIX
			for(j = 0; j < n; j++)
			{	
				y[2 * j + 0] = x[2 * j + 0];
				y[2 * j + 1] = x[2 * j + 1];

			}
		}

		MixerInfo.InBuf += (MixerInfo.PcmFormat == MIXER_FORMAT_MONO) ? n : (n * 2);
		MixerInfo.MixedIndex = (M + n) % N;
		MixerInfo.InCnt -= n; 
}


//mix与tx处理
static void MixerProcess(void)
{
	static volatile bool MixerRunFlag = FALSE;	//Mixer模块内防止重入的信号量

	__DISABLE_FAULT_IRQ();
	if(MixerRunFlag)
	{
		__ENABLE_FAULT_IRQ();
		return;
	}
	MixerRunFlag = TRUE;
	__ENABLE_FAULT_IRQ();

	MixerCheckTx();		//检查MixerBuf中是否有数据可以传送到PcmFifo	
	MixerCheckMix();	//检查MixerBuf中是否有空间Mix进新的数据	
	MixerCheckTx();		//再次检查MixerBuf中是否有数据可以传送到PcmFifo
	MixerCheckMix();	//再次检查MixerBuf中是否有空间Mix进新的数据

	MixerRunFlag = FALSE;
}


/////////////////////////下面是本模块对外的接口函数///////////////////////////////////////////////////////////
//Mixer模块初始化
//PcmFifoAddr: 必须在PMEM中
void Codec_MixerInit(void* PcmFifoAddr, uint16_t PcmFifoSize)
{
	
	MixerBuf = PcmFifoAddr;
	N = PcmFifoSize/4;
	memset(&MixerInfo, 0, sizeof(MixerInfo));

	MixerInfo.MixedIndex = PcmFifoGetReadAddr();
    MixerInfo.InCnt = 0;

	
}


//动态改变某个通道的采样率与格式
//SampleRate:  采样率，输入源0支持9种采样率，其他输入源只支持44.1KHZ采样率
//PcmFormat:   1--单声道，2--双声道
void Codec_MixerConfigFormat(uint8_t PcmFormat)
{
	MixerInfo.PcmFormat = PcmFormat;
}

//检查某通道是否可以设置新的数据
bool Codec_MixerIsDone(void)
{	
	MixerProcess();
    return (MixerInfo.InCnt == 0);
}

//PcmBuf:      PCM缓冲区首地址
//SampleCnt:   采样点数
void Codec_MixerSetData(void* PcmBuf, uint16_t SampleCnt)
{
	MixerProcess();
	MixerInfo.InBuf = PcmBuf;
    MixerInfo.InCnt = SampleCnt;
	MixerInfo.EmptyFlag = FALSE;
	MixerProcess();
}

//获取某输入源在Mixer内剩余的采样点数，折算为在该输入源采样率下的点数
uint16_t Codec_MixerGetRemainSamples(void)
{
	uint16_t n;
	uint16_t R = PcmFifoGetReadAddr();



	n = FIFO_LEN(R, MixerInfo.MixedIndex);
	if(n > FIFO_LEN(R, W))
	{
		n = 0;
	}

	n += MixerInfo.InCnt;	

	return n;
}

