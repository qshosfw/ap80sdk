#include <stdio.h>
#include "string.h"
#include "type.h"
#include "uart.h"
#include "clk.h"
#include "spi_flash.h"
#include "cache.h"
#include "gpio.h"
#include "rtc.h"
#include "adc.h"
#include "watchdog.h"
#include "uart.h"
#include "timeout.h"
#include "rtc_control.h"

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define FUNC_RTC_ALARM //��������

#define FUNC_RTC_LUNAR //��ʾũ��

uint8_t AlarmWorkMem[MEM_SIZE_PER_ALARM * MAX_ALARM_NUM];
RTC_CONTROL sRtcControl;
TIMER RtcTime;

uint8_t UartRec;
#define 	SetKeyValue(x)  (UartRec = (x)) 
#define 	GetKeyValue() 	(UartRec)
#define 	ClrKeyValue() 	(UartRec = 0) 

//ͨ��������ʾũ��
static void DisplayLunarDate(void)
{
	//ũ���������
	const uint8_t LunarYearName[12][2] = {"��", "ţ", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��"};
	
	//ũ���·�����
	const uint8_t LunarMonthName[12][2] = {"��", "��", "��", "��", "��", "��", "��", "��", "��", "ʮ", "��", "��"};
	
	//ũ����������
	const uint8_t LunarDateName[30][4] = {"��һ", "����", "����", "����", "����", "����", "����", "����", "����", "��ʮ", 
									  "ʮһ", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "ʮ��", "��ʮ",
									  "إһ", "إ��", "إ��", "إ��", "إ��", "إ��", "إ��", "إ��", "إ��", "��ʮ"};
	
	//ũ���������
	const uint8_t HeavenlyStemName[10][2] = {"��", "��", "��", "��", "��", "��", "��", "��", "��", "��"};
	
	//ũ����֧����
	const uint8_t EarthlyBranchName[12][2] = {"��", "��", "��", "î", "��", "��", "��", "δ", "��", "��", "��", "��"};
	
	RTC_LUNAR_DATE LunarDate;

	SolarToLunar(&sRtcControl.DataTime, &LunarDate); //����ũ��ת��API����	
	APP_DBG("ũ�� %d�� ", (uint32_t)LunarDate.Year);
	APP_DBG("%-.2s%-.2s�� ", HeavenlyStemName[GetHeavenlyStem(LunarDate.Year)], 
						  EarthlyBranchName[GetEarthlyBranch(LunarDate.Year)]);
	APP_DBG("%-.2s�� ", LunarYearName[GetEarthlyBranch(LunarDate.Year)]);
	if(LunarDate.IsLeapMonth)
	{
		APP_DBG("��");
	}
	APP_DBG("%-.2s��", LunarMonthName[LunarDate.Month - 1]);
	
	if(LunarDate.MonthDays == 29)
	{
		APP_DBG("(С)");
	}
	else
	{
		APP_DBG("(��)");
	}
	
	APP_DBG("%-.4s ", LunarDateName[LunarDate.Date - 1]);

	if((LunarDate.Month == 1) && (LunarDate.Date == 1))			//����
	{
		APP_DBG("����");
	}
	else if((LunarDate.Month == 1) && (LunarDate.Date == 15))	//Ԫ����
	{
		APP_DBG("Ԫ����");
	}
	else if((LunarDate.Month == 5) && (LunarDate.Date == 5))	//�����
	{
		APP_DBG("�����");
	}
	else if((LunarDate.Month == 7) && (LunarDate.Date == 7))	//��Ϧ���˽�
	{
		APP_DBG("��Ϧ���˽�");
	}
	else if((LunarDate.Month == 7) && (LunarDate.Date == 15))	//��Ԫ��
	{
		APP_DBG("��Ԫ��");
	}
	else if((LunarDate.Month == 8) && (LunarDate.Date == 15))	//�����
	{
		APP_DBG("�����");
	}
	else if((LunarDate.Month == 9) && (LunarDate.Date == 9))	//������
	{
   		APP_DBG("������");
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == 8))	//���˽�
	{
	 	APP_DBG("���˽�");
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == 23))	//С��
	{
		APP_DBG("С��");
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == LunarDate.MonthDays))	//��Ϧ
	{
		APP_DBG("��Ϧ");
	}
	APP_DBG("\n");
}


void DispRtcTimer(void)
{
	RtcGetCurrTime(&sRtcControl.DataTime);
	
	if(RTC_STATE_SET_TIME == sRtcControl.State)
	{
		APP_DBG("RTC set Time....");
	}
	
	APP_DBG("RtcTime(%04d-%02d-%02d %02d:%02d:%02d) Week:%d ",
	        sRtcControl.DataTime.Year, sRtcControl.DataTime.Mon, sRtcControl.DataTime.Date,
	        sRtcControl.DataTime.Hour, sRtcControl.DataTime.Min, sRtcControl.DataTime.Sec,
	        sRtcControl.DataTime.WDay);
#ifdef FUNC_RTC_LUNAR
	DisplayLunarDate();
#endif

#ifdef FUNC_RTC_ALARM	
	if(RTC_STATE_SET_ALARM == sRtcControl.State)
	{
		APP_DBG("���Ӻ�:%d ", sRtcControl.AlarmNum);
		switch(sRtcControl.AlarmMode)
		{
				case ALARM_MODE_ONCE_ONLY:
					APP_DBG("����ģʽ:����(once only) ");
					APP_DBG("%04d-%02d-%02d (����%02d) ", 
					sRtcControl.AlarmTime.Year, 
					sRtcControl.AlarmTime.Mon, 
					sRtcControl.AlarmTime.Date, 
					sRtcControl.AlarmTime.WDay);
					break;
	
				case ALARM_MODE_PER_DAY:
					APP_DBG("����ģʽ:ÿ��һ��(every day)");
					break;
	
				case ALARM_MODE_PER_WEEK:
					APP_DBG("����ģʽ:ÿ��һ��(every week) ����%02d", sRtcControl.AlarmTime.WDay);
					break;
				case ALARM_MODE_WORKDAY:
					APP_DBG("����ģʽ:������(weekday)");
					break;
				case ALARM_MODE_USER_DEFINED:
					APP_DBG("����ģʽ:�ͻ��Զ��� ÿ��%02d", sRtcControl.AlarmData);
					break;
	
				default:
					APP_DBG(("ģʽ����(mode error)\n"));
					break;
		}	
			
		APP_DBG("RtcTime(%02d:%02d:%02d) ",
	        sRtcControl.AlarmTime.Hour, sRtcControl.AlarmTime.Min, sRtcControl.AlarmTime.Sec);	
	}
#endif
	APP_DBG("\n");
}

// RTC��ʼ��
bool RtcInitialize(void)
{
	APP_DBG("RtcInitialize\n");
	
	RtcInit(AlarmWorkMem, sizeof(AlarmWorkMem));//Ӳ����ʼ������Ҫɾ��
	NVIC_EnableIRQ(RTC_IRQn);           // �����������ж�

	memset(&sRtcControl, 0, sizeof(RTC_CONTROL));

	// ��ȡeprom��nvram�еģ�ʱ�����ڵ���Ϣ
	RtcGetCurrTime(&sRtcControl.DataTime);

	return TRUE;
}


int32_t main(void)
{
	static RTC_DATE_TIME 	RtcPreDataTime; 
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating

	ClkPorRcToDpll(0);              //clock src is 32768hz OSC
	CacheInit();
	/*
	 * Now, uart has been initialized automatically by OS.
	 * if you want to customize debug uart setting(defaut 115200-8-N-1),
	 * you can invoke the function at any place you want(for example here).
	 */
	GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);
	FuartInit(115200, 8, 0, 1);
	APP_DBG("*****************************\n");
	APP_DBG("Mvsilicon RTC example Projext\n");
	
	//RTC ��ʼ��
	if(!RtcInitialize())
	{
		return FALSE;
	}
	
	RtcGetCurrTime(&RtcPreDataTime);	
	while(1)
	{		
		WdgFeed();//feed watch dog ever 10 ms
		
		//ͨ��������ʾʱ����Ϣ
		if(RtcPreDataTime.Sec != sRtcControl.DataTime.Sec)
		{
			memcpy(&RtcPreDataTime, &sRtcControl.DataTime, sizeof(RTC_DATE_TIME));
			DispRtcTimer();
		}
		
		//������ش���
		{
			if(sRtcControl.CurAlarmNum)
			{
				APP_DBG("\nRTC ALARM(%d) COME!\n\n", sRtcControl.CurAlarmNum);
				RtcAlarmArrivedProcess();
				//���ᴦ���ͻ�������������޸�
				sRtcControl.CurAlarmNum = 0;
			}
		}

		RtcGetCurrTime(&sRtcControl.DataTime);//��ȡ��ǰʱ��	
		
		if((GetKeyValue() == 'A') || (GetKeyValue() == 'a'))
		{
			//�յ��������Ϣ����ʱ�����
			APP_DBG("***********set Rtc Time\n");
			RtcGetCurrTime(&sRtcControl.DataTime);
			sRtcControl.DataTime.Min += 1;//��ǰ���Ӽ�1
			RtcSetCurrTime(&sRtcControl.DataTime);
			DispRtcTimer();
			ClrKeyValue();
		}
		if((GetKeyValue() == 'B') || (GetKeyValue() == 'b'))
		{
			APP_DBG("************Set Alarm Time\n");
			//�ѵ������Ϣ�������ӵ���
			RtcGetCurrTime(&sRtcControl.DataTime);
			memcpy(&sRtcControl.AlarmTime, &sRtcControl.DataTime, sizeof(RTC_DATE_TIME));
			sRtcControl.AlarmTime.Min += 1;// ��ǰ���Ӽ�1
			sRtcControl.AlarmTime.Sec = 0; // ����������
			sRtcControl.AlarmNum = 2;
			sRtcControl.AlarmMode = ALARM_MODE_PER_DAY;
			if(sRtcControl.AlarmMode == ALARM_MODE_WORKDAY)
			{
				sRtcControl.AlarmData = 0x3E;//����Ϊÿ��1-5�������ӣ���������Ϊ0x3E
			}
			else if(sRtcControl.AlarmMode == ALARM_MODE_USER_DEFINED)
			{
				sRtcControl.AlarmData = 0x41;//����Ϊÿ���������������ӣ���������Ϊ��������ֵ
			}
			else
			{
				//AlarmData by pass,����ģʽ����
			}
				
			RtcSetAlarmTime(sRtcControl.AlarmNum, sRtcControl.AlarmMode, sRtcControl.AlarmData, &sRtcControl.AlarmTime);
			sRtcControl.State = RTC_STATE_SET_ALARM;
			DispRtcTimer();
			sRtcControl.State = RTC_STATE_IDLE;
			ClrKeyValue();
		}
	}
}


__attribute__((section(".driver.isr"), weak)) void RtcInterrupt(void)
{
	// ���������ж�
	sRtcControl.CurAlarmNum = RtcCheckAlarmFlag();
	RtcAlarmIntClear();
}

void FuartInterrupt(void)
{
    //If data received
    if(IsFuartRxIntSrc())
    {
        uint8_t c;
        
        //Get received byte
        FuartRecvByte(&c);

		//clear interrupt
        FuartClrRxInt();
		
        SetKeyValue(c);
	}

    //If data sent
	if(IsFuartTxIntSrc())
    {
		//clear interrupt
        FuartClrTxInt();
	}
}
