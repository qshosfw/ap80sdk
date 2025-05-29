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

#define FUNC_RTC_ALARM //开启闹钟

#define FUNC_RTC_LUNAR //显示农历

uint8_t AlarmWorkMem[MEM_SIZE_PER_ALARM * MAX_ALARM_NUM];
RTC_CONTROL sRtcControl;
TIMER RtcTime;

uint8_t UartRec;
#define 	SetKeyValue(x)  (UartRec = (x)) 
#define 	GetKeyValue() 	(UartRec)
#define 	ClrKeyValue() 	(UartRec = 0) 

//通过串口显示农历
static void DisplayLunarDate(void)
{
	//农历年份名称
	const uint8_t LunarYearName[12][2] = {"鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪"};
	
	//农历月份名称
	const uint8_t LunarMonthName[12][2] = {"正", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊"};
	
	//农历日期名称
	const uint8_t LunarDateName[30][4] = {"初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十", 
									  "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十",
									  "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"};
	
	//农历天干名称
	const uint8_t HeavenlyStemName[10][2] = {"甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"};
	
	//农历地支名称
	const uint8_t EarthlyBranchName[12][2] = {"子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"};
	
	RTC_LUNAR_DATE LunarDate;

	SolarToLunar(&sRtcControl.DataTime, &LunarDate); //调用农历转换API函数	
	APP_DBG("农历 %d年 ", (uint32_t)LunarDate.Year);
	APP_DBG("%-.2s%-.2s年 ", HeavenlyStemName[GetHeavenlyStem(LunarDate.Year)], 
						  EarthlyBranchName[GetEarthlyBranch(LunarDate.Year)]);
	APP_DBG("%-.2s年 ", LunarYearName[GetEarthlyBranch(LunarDate.Year)]);
	if(LunarDate.IsLeapMonth)
	{
		APP_DBG("闰");
	}
	APP_DBG("%-.2s月", LunarMonthName[LunarDate.Month - 1]);
	
	if(LunarDate.MonthDays == 29)
	{
		APP_DBG("(小)");
	}
	else
	{
		APP_DBG("(大)");
	}
	
	APP_DBG("%-.4s ", LunarDateName[LunarDate.Date - 1]);

	if((LunarDate.Month == 1) && (LunarDate.Date == 1))			//春节
	{
		APP_DBG("春节");
	}
	else if((LunarDate.Month == 1) && (LunarDate.Date == 15))	//元宵节
	{
		APP_DBG("元宵节");
	}
	else if((LunarDate.Month == 5) && (LunarDate.Date == 5))	//端午节
	{
		APP_DBG("端午节");
	}
	else if((LunarDate.Month == 7) && (LunarDate.Date == 7))	//七夕情人节
	{
		APP_DBG("七夕情人节");
	}
	else if((LunarDate.Month == 7) && (LunarDate.Date == 15))	//中元节
	{
		APP_DBG("中元节");
	}
	else if((LunarDate.Month == 8) && (LunarDate.Date == 15))	//中秋节
	{
		APP_DBG("中秋节");
	}
	else if((LunarDate.Month == 9) && (LunarDate.Date == 9))	//重阳节
	{
   		APP_DBG("重阳节");
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == 8))	//腊八节
	{
	 	APP_DBG("腊八节");
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == 23))	//小年
	{
		APP_DBG("小年");
	}
	else if((LunarDate.Month == 12) && (LunarDate.Date == LunarDate.MonthDays))	//除夕
	{
		APP_DBG("除夕");
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
		APP_DBG("闹钟号:%d ", sRtcControl.AlarmNum);
		switch(sRtcControl.AlarmMode)
		{
				case ALARM_MODE_ONCE_ONLY:
					APP_DBG("闹钟模式:单次(once only) ");
					APP_DBG("%04d-%02d-%02d (星期%02d) ", 
					sRtcControl.AlarmTime.Year, 
					sRtcControl.AlarmTime.Mon, 
					sRtcControl.AlarmTime.Date, 
					sRtcControl.AlarmTime.WDay);
					break;
	
				case ALARM_MODE_PER_DAY:
					APP_DBG("闹钟模式:每天一次(every day)");
					break;
	
				case ALARM_MODE_PER_WEEK:
					APP_DBG("闹钟模式:每周一次(every week) 星期%02d", sRtcControl.AlarmTime.WDay);
					break;
				case ALARM_MODE_WORKDAY:
					APP_DBG("闹钟模式:工作日(weekday)");
					break;
				case ALARM_MODE_USER_DEFINED:
					APP_DBG("闹钟模式:客户自定义 每周%02d", sRtcControl.AlarmData);
					break;
	
				default:
					APP_DBG(("模式错误(mode error)\n"));
					break;
		}	
			
		APP_DBG("RtcTime(%02d:%02d:%02d) ",
	        sRtcControl.AlarmTime.Hour, sRtcControl.AlarmTime.Min, sRtcControl.AlarmTime.Sec);	
	}
#endif
	APP_DBG("\n");
}

// RTC初始化
bool RtcInitialize(void)
{
	APP_DBG("RtcInitialize\n");
	
	RtcInit(AlarmWorkMem, sizeof(AlarmWorkMem));//硬件初始化，不要删除
	NVIC_EnableIRQ(RTC_IRQn);           // 打开闹钟提醒中断

	memset(&sRtcControl, 0, sizeof(RTC_CONTROL));

	// 读取eprom或nvram中的：时间日期等信息
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
	
	//RTC 初始化
	if(!RtcInitialize())
	{
		return FALSE;
	}
	
	RtcGetCurrTime(&RtcPreDataTime);	
	while(1)
	{		
		WdgFeed();//feed watch dog ever 10 ms
		
		//通过串口显示时间信息
		if(RtcPreDataTime.Sec != sRtcControl.DataTime.Sec)
		{
			memcpy(&RtcPreDataTime, &sRtcControl.DataTime, sizeof(RTC_DATE_TIME));
			DispRtcTimer();
		}
		
		//闹钟相关处理
		{
			if(sRtcControl.CurAlarmNum)
			{
				APP_DBG("\nRTC ALARM(%d) COME!\n\n", sRtcControl.CurAlarmNum);
				RtcAlarmArrivedProcess();
				//响玲处理，客户可以自行添加修改
				sRtcControl.CurAlarmNum = 0;
			}
		}

		RtcGetCurrTime(&sRtcControl.DataTime);//获取当前时间	
		
		if((GetKeyValue() == 'A') || (GetKeyValue() == 'a'))
		{
			//收到到相关消息后做时间调整
			APP_DBG("***********set Rtc Time\n");
			RtcGetCurrTime(&sRtcControl.DataTime);
			sRtcControl.DataTime.Min += 1;//当前分钟加1
			RtcSetCurrTime(&sRtcControl.DataTime);
			DispRtcTimer();
			ClrKeyValue();
		}
		if((GetKeyValue() == 'B') || (GetKeyValue() == 'b'))
		{
			APP_DBG("************Set Alarm Time\n");
			//搜到相关消息后做闹钟调整
			RtcGetCurrTime(&sRtcControl.DataTime);
			memcpy(&sRtcControl.AlarmTime, &sRtcControl.DataTime, sizeof(RTC_DATE_TIME));
			sRtcControl.AlarmTime.Min += 1;// 当前分钟加1
			sRtcControl.AlarmTime.Sec = 0; // 闹钟秒清零
			sRtcControl.AlarmNum = 2;
			sRtcControl.AlarmMode = ALARM_MODE_PER_DAY;
			if(sRtcControl.AlarmMode == ALARM_MODE_WORKDAY)
			{
				sRtcControl.AlarmData = 0x3E;//定义为每周1-5响铃闹钟，必须设置为0x3E
			}
			else if(sRtcControl.AlarmMode == ALARM_MODE_USER_DEFINED)
			{
				sRtcControl.AlarmData = 0x41;//定义为每周日周六响铃闹钟，可以设置为其他任意值
			}
			else
			{
				//AlarmData by pass,根据模式设置
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
	// 闹钟提醒中断
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
