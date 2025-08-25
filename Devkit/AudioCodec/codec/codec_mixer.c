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
	uint16_t 	MixedIndex;		//MixerBuf�е�ǰMIX����λ��

	int16_t*	InBuf;			//��δMIX��MixerBuf�е����ݻ�����ָ��
	uint16_t 	InCnt;			//��δMIX��MixerBuf�е����ݳ���

	bool		EmptyFlag;		//�ձ�־

} MIXER_INFO;

static MIXER_INFO MixerInfo;

static int16_t* MixerBuf = NULL;
static uint16_t N = 0;			//��λ����������
static uint16_t W = 0;			//PCM FIFO��Ӳ��дָ��

static void MixerProcess(void);


#define FIFO_LEN(StartIndex, EndIndex)	((EndIndex + N - StartIndex) % N)

//���ĳ��ͨ���Ƿ񲥿�
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


//ͨ��TXģ�齫PCM FIFO��дָ������ƶ���TXʵ�ʴ��͵���0����
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

        MixerCheckEmpty();		//���ĳ��ͨ���Ƿ񲥿�
		//W = (W + Cnt) % N;		//��©������DAC�����ʱ仯ʱ��Ӳ����ʵ��Wֵ��ͻ��
		//��ȡӲ����ʵ��Wλ��
		N = PcmFifoGetDepth();
		W = (PcmFifoGetReadAddr() + PcmFifoGetRemainSamples()) % N;
	}
}

//��������ͨ·�Ļ��������Ƿ������ݿ���MIX��MixerBuf��
//��Ҫ�������漸��������
//1. ���뻺������������
//2. MixerBuf���п���
static void MixerCheckMix(void)
{
 	uint16_t j;
	uint16_t n;		
	int16_t* x;				//MIXǰ��������ʱָ��
	int16_t* y;				//MIX�󻺳�����ʱָ��

	
	uint16_t M;
	

		//���ĳ��ͨ���Ƿ񲥿�
		MixerCheckEmpty();

		//����FIFO��ʣ���MIX�Ŀռ䳤��,����M��W�ĳ���
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
			//����������y[]��ԭ������MIX
            for(j = 0; j < n; j++)
            {
                y[2 * j + 1] = y[2 * j + 0] = x[j];
                 
            } 
		}
		else
		{
			//˫��������y[]��ԭ������MIX
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


//mix��tx����
static void MixerProcess(void)
{
	static volatile bool MixerRunFlag = FALSE;	//Mixerģ���ڷ�ֹ������ź���

	__DISABLE_FAULT_IRQ();
	if(MixerRunFlag)
	{
		__ENABLE_FAULT_IRQ();
		return;
	}
	MixerRunFlag = TRUE;
	__ENABLE_FAULT_IRQ();

	MixerCheckTx();		//���MixerBuf���Ƿ������ݿ��Դ��͵�PcmFifo	
	MixerCheckMix();	//���MixerBuf���Ƿ��пռ�Mix���µ�����	
	MixerCheckTx();		//�ٴμ��MixerBuf���Ƿ������ݿ��Դ��͵�PcmFifo
	MixerCheckMix();	//�ٴμ��MixerBuf���Ƿ��пռ�Mix���µ�����

	MixerRunFlag = FALSE;
}


/////////////////////////�����Ǳ�ģ�����Ľӿں���///////////////////////////////////////////////////////////
//Mixerģ���ʼ��
//PcmFifoAddr: ������PMEM��
void Codec_MixerInit(void* PcmFifoAddr, uint16_t PcmFifoSize)
{
	
	MixerBuf = PcmFifoAddr;
	N = PcmFifoSize/4;
	memset(&MixerInfo, 0, sizeof(MixerInfo));

	MixerInfo.MixedIndex = PcmFifoGetReadAddr();
    MixerInfo.InCnt = 0;

	
}


//��̬�ı�ĳ��ͨ���Ĳ��������ʽ
//SampleRate:  �����ʣ�����Դ0֧��9�ֲ����ʣ���������Դֻ֧��44.1KHZ������
//PcmFormat:   1--��������2--˫����
void Codec_MixerConfigFormat(uint8_t PcmFormat)
{
	MixerInfo.PcmFormat = PcmFormat;
}

//���ĳͨ���Ƿ���������µ�����
bool Codec_MixerIsDone(void)
{	
	MixerProcess();
    return (MixerInfo.InCnt == 0);
}

//PcmBuf:      PCM�������׵�ַ
//SampleCnt:   ��������
void Codec_MixerSetData(void* PcmBuf, uint16_t SampleCnt)
{
	MixerProcess();
	MixerInfo.InBuf = PcmBuf;
    MixerInfo.InCnt = SampleCnt;
	MixerInfo.EmptyFlag = FALSE;
	MixerProcess();
}

//��ȡĳ����Դ��Mixer��ʣ��Ĳ�������������Ϊ�ڸ�����Դ�������µĵ���
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

