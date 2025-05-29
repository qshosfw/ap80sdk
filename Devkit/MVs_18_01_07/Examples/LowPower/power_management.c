///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: power_management.c
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "type.h"
#include "app_config.h"
#include "uart.h"
#include "adc.h"
#include "clk.h"
#include "gpio.h"
#include "wakeup.h"

#ifdef FUNC_POWER_MONITOR_EN

#define LDOIN_SAMPLE_COUNT			100		//��ȡLDOIN����ʱ����ƽ���Ĳ�������
#define LDOIN_SAMPLE_PERIOD			50		//��ȡLDOIN����ʱ��ȡ����ֵ�ļ��(ms)
#define LOW_POWEROFF_TIME			10000		//�͵���ػ��������ʱ��(ms)


//���¶��岻ͬ�ĵ�ѹ����¼��Ĵ�����ѹ(��λmV)���û���������ϵͳ��ص��ص�������
#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
#define LDOIN_VOLTAGE_FULL			4096
#define LDOIN_VOLTAGE_CHARGE_FULL   0xc00  // 13.9v
#define LDOIN_VOLTAGE_HIGH			0xa62     //12v
#define LDOIN_VOLTAGE_MID           0x981    //11v
#define LDOIN_VOLTAGE_LOW			0x75e    //8.5V
#define LDOIN_VOLTAGE_OFF			0x608	//7V���ڴ˵�ѹֵ����ػ�powerdown״̬
#else
#define LDOIN_VOLTAGE_FULL			4200
#define LDOIN_VOLTAGE_HIGH			3600
#define LDOIN_VOLTAGE_LOW			3500
#define LDOIN_VOLTAGE_OFF			3300	//���ڴ˵�ѹֵ����ػ�powerdown״̬
#endif

//��ѹ���ʱ��ͬ����ʾ����
typedef enum _PWR_MNT_DISP
{
	PWR_MNT_DISP_NONE = 0,
	PWR_MNT_DISP_CHARGE,		 
	PWR_MNT_DISP_HIGH_V, 
	PWR_MNT_DISP_MID_V, 
	PWR_MNT_DISP_LOW_V, 
	PWR_MNT_DISP_EMPTY_V, 
	PWR_MNT_DISP_SYS_OFF
	 
} PWR_MNT_DISP;

//��Ҫ���ڵ�ѹ״̬��ʾ�ı���
static TIMER BlinkTimer;

//���ڵ�ѹ���ı���
TIMER PowerMonitorTimer;
uint32_t LdoinSampleSum = 0; 
uint16_t  LdoinSampleCnt = LDOIN_SAMPLE_COUNT;
uint32_t LdoinLevelAverage = 0;		//��ǰLDOIN��ѹƽ��ֵ


#ifdef	OPTION_CHARGER_DETECT
//Ӳ�����PC ����������״̬
//ʹ���ڲ���������PC����������ʱ������Ϊ�͵�ƽ����ʱ����Ϊ�ߵ�ƽ
bool IsInCharge(VOID)
{
//��Ϊ���룬��������

	GpioSetRegBits(CHARGE_DETECT_PORT_PU, CHARGE_DETECT_BIT);
	GpioSetRegBits(CHARGE_DETECT_PORT_PD, CHARGE_DETECT_BIT);
	GpioClrRegBits(CHARGE_DETECT_PORT_OE, CHARGE_DETECT_BIT);	   
	GpioSetRegBits(CHARGE_DETECT_PORT_IE, CHARGE_DETECT_BIT);
	WaitUs(2);
	if(GpioGetReg(CHARGE_DETECT_PORT_IN) & CHARGE_DETECT_BIT)
	{
		return TRUE;
	}   	

	return FALSE;
}
#endif
void PowerMonitorDisp(void)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN) ||defined(FUNC_TM1628_LED_EN))
	static uint8_t  ShowStep = 0;
	static bool IsToShow = FALSE;

	switch(PwrMntDisp)
	{			
		case PWR_MNT_DISP_CHARGE:
			//��˸���ICON,��ʾ���ڳ��
			if(IsTimeOut(&BlinkTimer))
			{
				TimeOutSet(&BlinkTimer, 500);
				switch(ShowStep)
				{

					case 0:
						DispIcon(ICON_BAT1, FALSE);
						DispIcon(ICON_BAT2, FALSE);
						DispIcon(ICON_BAT3, FALSE);
						break;
					case 1:
						DispIcon(ICON_BAT1, TRUE);
						DispIcon(ICON_BAT2, FALSE);
						DispIcon(ICON_BAT3, FALSE);
						break;
					case 2:
						DispIcon(ICON_BAT1, TRUE);
						DispIcon(ICON_BAT2, TRUE);
						DispIcon(ICON_BAT3, FALSE);
						break;
					case 3:
						DispIcon(ICON_BAT1, TRUE);
						DispIcon(ICON_BAT2, TRUE);
						DispIcon(ICON_BAT3, TRUE);
						break;
				}		
				if(ShowStep < 3)
				{
					ShowStep++;
				}
				else
				{
					ShowStep = 0;
				}
			}
			
			break;

		case PWR_MNT_DISP_HIGH_V:
			//DBG("BAT FULL\n");			
			#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
			DispIcon(ICON_BAT1, TRUE);
			DispIcon(ICON_BAT2, TRUE);
			DispIcon(ICON_BAT3, TRUE);
			#else
			DispIcon(ICON_BATFUL, TRUE);
			DispIcon(ICON_BATHAF, FALSE);
			#endif
			//������ʾ����������������ʾ����
			break;
			
		case PWR_MNT_DISP_MID_V:
			//DBG("BAT HALF\n");		
			#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
			DispIcon(ICON_BAT1, TRUE);
			DispIcon(ICON_BAT2, TRUE);
			DispIcon(ICON_BAT3, FALSE);
			#else
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			#endif
			//������ʾ2��������������ʾ����
			break;

		case PWR_MNT_DISP_LOW_V:
			#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
			DispIcon(ICON_BAT1, TRUE);
			DispIcon(ICON_BAT2, FALSE);
			DispIcon(ICON_BAT3, FALSE);
			#else
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			#endif
			//������ʾ1��������������ʾ����
			break;
			
		case PWR_MNT_DISP_EMPTY_V:
			//DBG("BAT EMPTY\n");				
			DispIcon(ICON_BATFUL, FALSE);

			if(IsTimeOut(&BlinkTimer))
			{
				TimeOutSet(&BlinkTimer, 300);
				if(IsToShow)
				{
				       #ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
					DispIcon(ICON_BAT1, TRUE);
					DispIcon(ICON_BAT2, FALSE);
					DispIcon(ICON_BAT3, FALSE);
					#else
					DispIcon(ICON_BATHAF, TRUE);
					#endif
				}
				else
				{
					#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
					DispIcon(ICON_BAT1, FALSE);
					DispIcon(ICON_BAT2, FALSE);
					DispIcon(ICON_BAT3, FALSE);
					#else
					DispIcon(ICON_BATHAF, FALSE);
					#endif
				}
				IsToShow = !IsToShow;
			}
			//������ʾ0��������������ʾ����
			break;
		
		case PWR_MNT_DISP_SYS_OFF:
			//DBG("BAT OFF\n");
			//ClearScreen();			//�����ʾ				
			//DispString(" LO ");
			break;
			
		default:
			break;
	}
#endif	
}
//���LDOIN�ĵ�ѹֵ��ִ�ж�Ӧ����Ĵ���
//PowerOnInitFlag: TRUE--��һ���ϵ�ִ�е�Դ��ؼ��
static void PowerLdoinLevelMonitor(bool PowerOnInitFlag)
{	
	bool PowerOffFlag = FALSE;
	//uint16_t AdcVal;	
	if(LdoinSampleCnt > 0)
	{
#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
		//LdoinSampleSum += SarAdcChannelGetValue(ADC_CHANNEL_B22);
		AdcVal = SarAdcChannelGetValue(ADC_CHANNEL_B22);
		LdoinSampleSum += AdcVal;  
#else
		LdoinSampleSum += SarAdcGetLdoinVoltage();		
#endif
		LdoinSampleCnt--;
	}

	//������LDOIN_SAMPLE_COUNT��������LDOINƽ��ֵ
	if(LdoinSampleCnt == 0)
	{
		LdoinLevelAverage = LdoinSampleSum / LDOIN_SAMPLE_COUNT;

		DBG("LODin 5V Volt: %lu\n", (uint32_t)LdoinLevelAverage);

		//Ϊ�������LDOIN����ʼ������
		LdoinSampleCnt = LDOIN_SAMPLE_COUNT;
		LdoinSampleSum = 0;

#ifdef	OPTION_CHARGER_DETECT
		if(IsInCharge())		//������Ѿ�����Ĵ���
		{		
			PowerMonitorDisp();
			return;
		}

		
		if(LdoinLevelAverage > LDOIN_VOLTAGE_HIGH)	  
		{
			//������ʾ�������������PowerMonitorDisp�������ʾ����
			//PowerMonitorDisp(PWR_MNT_DISP_HIGH_V);
			PwrMntDisp = PWR_MNT_DISP_HIGH_V;
			//DBG("bat full\n");
		}

		else if(LdoinLevelAverage > LDOIN_VOLTAGE_MID)
		{
			//������ʾ2�����������PowerMonitorDisp�������ʾ����
			//PowerMonitorDisp(PWR_MNT_DISP_MID_V);
			PwrMntDisp = PWR_MNT_DISP_MID_V;
		}
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_LOW)
		{
			//������ʾ1�����������PowerMonitorDisp�������ʾ����
			//PowerMonitorDisp(PWR_MNT_DISP_LOW_V);
			PwrMntDisp = PWR_MNT_DISP_LOW_V;
			//DBG("bat LOW\n");
		}
		else if(LdoinLevelAverage > LDOIN_VOLTAGE_OFF)
		{
			//������ʾ0�����������PowerMonitorDisp�������ʾ����
			//PowerMonitorDisp(PWR_MNT_DISP_EMPTY_V);
			PwrMntDisp = PWR_MNT_DISP_EMPTY_V;
		}
#endif
		
		if(PowerOnInitFlag == TRUE) 
		{		
			if(LdoinLevelAverage <= LDOIN_VOLTAGE_LOW)
			{
				PowerOffFlag = TRUE;
			}			
		}

		if((PowerOnInitFlag == FALSE) && (LdoinLevelAverage <= LDOIN_VOLTAGE_OFF))
		{
			PowerOffFlag = TRUE;
		}
		
		if(PowerOffFlag == TRUE)
		{
			PowerMonitorDisp();			
			//WaitMs(1000);
			//ֹͣ�����������̣���������ʾ����DAC���ع��ŵ�Դ�ȶ���
			DBG("PowerMonitor, PD\n");				
			//SystemOff();
			//while(1);
			//gSys.NextModuleID = MODULE_ID_POWEROFF;
			//MsgSend(MSG_COMMON_CLOSE);
		}		
	}
	PowerMonitorDisp();
}


//���ܼ��ӳ�ʼ��
//ʵ��ϵͳ���������еĵ͵�ѹ��⴦���Լ���ʼ������豸������IO��
void PowerMonitorInit(void)
{
	TimeOutSet(&PowerMonitorTimer, 0);	
	TimeOutSet(&BlinkTimer, 0);	
#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
	SarAdcGpioSel(ADC_CHANNEL_B22);
#endif
#ifdef OPTION_CHARGER_DETECT
	//���ϵͳ����ʱ������豸�Ѿ����룬�Ͳ�ִ��������ε͵�ѹ���ʹ������
	if(!IsInCharge())
#endif	
	{
		//ϵͳ���������еĵ͵�ѹ���
		//����ʱ��ѹ��⣬���С�ڿ�����ѹ���������豸���Ͳ������̣�ֱ�ӹػ�
		//������Ϊʱ50ms�������ж�Ӧ�Ĵ���
		while(LdoinSampleCnt)
		{
			LdoinSampleCnt--;
			#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
			LdoinSampleSum += SarAdcChannelGetValue(ADC_CHANNEL_B22);
			#else
			LdoinSampleSum += SarAdcGetLdoinVoltage();
			#endif
			WaitMs(5);
		}		
		//Ϊ��ߵ�LDOIN����ʼ������
		PowerLdoinLevelMonitor(TRUE);
	}
}

//ϵͳ��Դ״̬��غʹ���
//ϵͳ���������LDOIN���ڿ�����ѹ������ϵͳ���������м��LDOIN
void PowerMonitor(void)
{
	if(IsTimeOut(&PowerMonitorTimer))
	{
		TimeOutSet(&PowerMonitorTimer, LDOIN_SAMPLE_PERIOD);

#ifdef OPTION_CHARGER_DETECT
		if(IsInCharge())		//������Ѿ�����Ĵ���
		{
			if(LdoinLevelAverage >= LDOIN_VOLTAGE_CHARGE_FULL) 
			{
				//PowerMonitorDisp(PWR_MNT_DISP_HIGH_V);		//��ʾ���״̬
				PwrMntDisp = PWR_MNT_DISP_HIGH_V;
#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
				ChargeFull1On();
				ChargeFull2On();
				ChargeInOff();
				ChargeSWOff();
#endif
				//DBG("charger full\n");
			}
			else
			{
				//PowerMonitorDisp(PWR_MNT_DISP_CHARGE);		//��ʾ���״̬
				PwrMntDisp = PWR_MNT_DISP_CHARGE;
#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
				ChargeFull1Off();
				ChargeFull2Off();
				ChargeInOn();
				ChargeSWOn();
#endif
				//DBG("charger.....\n");
			}
		}
#ifdef FUNC_SENERTONG_TUOXIANG_DEMO_V1_0_EN
		else
		{
			ChargeFull1Off();
			ChargeFull2Off();
			ChargeInOff();
			ChargeSWOn();
		}
#endif
#endif		
		//û�в�����LDOIN_SAMPLE_COUNT��������������
		PowerLdoinLevelMonitor(FALSE);
	}
}
#ifdef LOW_POWER_SAVE_PLAYTIME_TO_FLASH
extern uint32_t AudioPlaySavePlayTimeToFlash(void);

void LowPowerDetProc(void)
{
	uint16_t RetVal;

	RetVal = SarAdcGetLdoinVoltage();

	if((RetVal < 4200) && (RetVal > 3400))	
   	{
	   	AudioPlaySavePlayTimeToFlash();
   	}
}
#endif
#endif	//end of FUNC_POWER_MONITOR_EN

/**
 * @brief 		system power off detect function
 *			this function should be called in timer INT function or systick function for periodically detecting
 *			when USE_POWERKEY_SLIDE_SWITCH, the system will be powered off directly
 *			when USE_POWERKEY_SOFT_PUSH_BUTTON, it wil send message:MSG_COMMON_CLOSE, for task's saving information, 
* 			and then power off system
 * @param	None
 * @return	None
 */
void SystemPowerOffDetect(void)
{

#ifdef USE_POWERKEY_SLIDE_SWITCH
#define SLIDE_SWITCH_ONTIME 100	
	static uint16_t  slide_switch_pd_cnt = SLIDE_SWITCH_ONTIME;//SLIDE_SWITCH_ONTIME*2ms(tmer1)����ʱ�䣬��PowerKeyDetect()����
	if(PowerKeyDetect())
	{
		if(slide_switch_pd_cnt-- == 0)
		{
			/*if slide switch, power down system directly*/
			APP_DBG("PowerKeyDetect->go to PowerDown\n");
			SysSetWakeUpSrcInPowerDown(WAKEUP_SRC_PD_POWERKEY);
			SysGotoPowerDown(); 
			while(1);
		}
	}
	else
	{
		slide_switch_pd_cnt = SLIDE_SWITCH_ONTIME;
	}
#endif 
	
#ifdef USE_POWERKEY_SOFT_PUSH_BUTTON
	if(PowerKeyDetect())
	{
		#if 0
		if(gSys.NextModuleID != MODULE_ID_POWEROFF)	/*MSG_COMMON_CLOSE only need send once*/
		{
			/*if use push button, send message, for task's saving info.*/
			APP_DBG("PowerKeyDetect->send message common close\n");
			gSys.NextModuleID = MODULE_ID_POWEROFF;
			MsgSend(MSG_COMMON_CLOSE);
		}
		#endif
		APP_DBG("PowerKeyDetect->go to PowerDown\n");
		SysSetWakeUpSrcInPowerDown(WAKEUP_SRC_PD_POWERKEY);
		SysGotoPowerDown(); 
		while(1);
	
	}
#endif	
	
}

// ����deep sleep��Ϊ�˽��͹��ģ�Ĭ������Ϊ����������
// �ͻ����Ը�����Ҫ����һЩ���ƻ�����
void SysGotoDeepSleepGpioCfg(void)
{
	GpioSetRegBits(GPIO_A_IE, 0xFFFFFFFF);
	GpioClrRegBits(GPIO_A_OE, 0xFFFFFFFF);
	GpioSetRegBits(GPIO_A_PU, 0xFFFFFFFF);
	GpioClrRegBits(GPIO_A_PD, 0xFFFFFFFF);

	GpioSetRegBits(GPIO_B_IE, 0xFFFFFFFF);
	GpioClrRegBits(GPIO_B_OE, 0xFFFFFFFF);
	GpioSetRegBits(GPIO_B_PU, 0xFFFFFFFF);
	GpioClrRegBits(GPIO_B_PD, 0xFFFFFFFF);

	GpioSetRegBits(GPIO_C_IE, 0x7FFF);
	GpioClrRegBits(GPIO_C_OE, 0x7FFF);
	GpioSetRegBits(GPIO_C_PU, 0x7FFF);
	GpioClrRegBits(GPIO_C_PD, 0x7FFF);

	// BT close, not arbitrarily modify
//	BTDevicePowerOff();
}


/**
 * @brief 		system power off flow
 *			user can add other functions before SysGotoPowerDown()
 *			
 * @param	None
 * @return	None
 */
void SystemPowerOffControl(void)
{
	APP_DBG("SystemPowerOffControl->system will power off soon!\n");
	
//	SysVarDeinit();
	SysSetWakeUpSrcInPowerDown(WAKEUP_SRC_PD_POWERKEY);
	SysGotoPowerDown(); 

	/*Never reach here,  expect power down fail*/
	APP_DBG("System Power Down Fail!");
	while(1);
}

/**
 * @brief 		system standby flow
 *			user can add other functions before SysGotoDeepSleep()
 *			
 * @param	None
 * @return	None
 */
void SystemStandbyControl(void)
{
	uint32_t Port = GPIOC2;
	APP_DBG("StandBy Mode\n");
//	SysVarDeinit();

	SysGotoDeepSleepGpioCfg();

	//set GPIOC2 input enable, pull down
	GpioSetRegOneBit(GPIO_C_IE, Port);
	GpioSetRegOneBit(GPIO_C_PU, Port);
	GpioSetRegOneBit(GPIO_C_PD, Port);  

	//set wakeup source GPIOC2, Wakeup Polarith: High wakeup, pin wakeup valid minimal time is 1ms
	SysSetWakeUpSrcInDeepSleep(WAKEUP_SRC_SLEEP_C2, WAKEUP_POLAR_C2_HI, WAKEUP_TMODE_1MS);
	SysGotoDeepSleep();

	/*Never reach here,  expect go to deepsleep fail*/
	APP_DBG("System Go to Deep Sleep Fail!");
	while(1);
}
