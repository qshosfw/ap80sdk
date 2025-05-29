///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2014, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: IrExample.c
///////////////////////////////////////////////////////////////////////////////
#include "type.h"
#include "clk.h"
#include "gpio.h"
#include "uart.h"
#include "cache.h"
#include "timeout.h"
#include "watchdog.h"  
#include "Timer.h"
#include <stdio.h> 

#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)
#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define 	IR_KEY_SCAN_TIME			5
#define 	IR_KEY_JITTER_TIME			30
#define		IR_KEY_CP_TIME				1000	//CP condition is 1s
#define 	IR_KEY_CPH_TIME				350

#define		IR_KEY_SUM					21		//IR KEY SUM

typedef enum _IR_KEY_STATE
{
    IR_KEY_STATE_IDLE,
    IR_KEY_STATE_JITTER,
    IR_KEY_STATE_PRESS_DOWN,
    IR_KEY_STATE_CP

} IR_KEY_STATE;

//static TIMER			IrKeyHoldTimer;
//static TIMER			IrKeyWaitTimer;
static TIMER			IrKeyScanTimer;
//static IR_KEY_STATE		IrKeyState;

static uint32_t 		IrKeyVal;

//IrKeyVal[31:16]: key value
//IrKeyVal[15:0]: manu id
#define IR_MANU_ID		0x7F80

// Customer IR key value table, fix it by case.
//static const uint8_t gIrVal[IR_KEY_SUM] =
//{
//	0xED,		// POWER
//	0xE5,		// MODE
//	0xE1,		// MUTE

//	0xFE,		// PLAY/PAUSE
//	0xFD,		// PRE
//	0xFC,		// NEXT

//	0xFB,		// EQ
//	0xFA,		// VOL-
//	0xF9,		// VOL+

//	0xF8,		// 0
//	0xF7,		// REPEAT
//	0xF6,		// SCN

//	0xF5,		// 1
//	0xE4,		// 2
//	0xE0,		// 3

//	0xF3,		// 4
//	0xF2,		// 5
//	0xF1,		// 6

//	0xFF,		// 7
//	0xF0,		// 8
//	0xE6,		// 9
//};

uint16_t Timer100usCnt;
bool IsIrShortPrs; 
bool IsIrContinuePrs;
bool IsIrIgnoreLeader = TRUE;
uint32_t sIrWakeupKeyVal = 0;
bool IsIrKeyWakeupFlag = FALSE;

void IrDecdInterrupt(void)
{
	static uint8_t RcvBits;
	static bool	IsRcvData = FALSE;
//	uint16_t TimeNow;
	uint16_t TrigPeriod;
	static uint32_t sIrKeyVal;	  

	TrigPeriod = Timer100usCnt;
	Timer100usCnt = 0;

	if(IsIrIgnoreLeader)
	{
		IsRcvData = TRUE;
	}  
	
	if ((TrigPeriod > 123) && (TrigPeriod < 145))	//12.3ms ~ 14.5ms (13.5ms)
	{										   	//the first index code
		RcvBits = 0;
		sIrKeyVal = 0;
		IsRcvData = TRUE;  
	}
	else if ((TrigPeriod > 98) && (TrigPeriod < 123))	//9.8ms ~ 12.3ms
	{											//the continuous index code
		if((!IsRcvData) || (RcvBits == 0))
		{
			IsIrShortPrs = FALSE;
			IsIrContinuePrs = TRUE;
		}
	}
	else if(IsRcvData)
	{
		sIrKeyVal >>= 1;
		if ((TrigPeriod > 0) && (TrigPeriod < 15))	//0~1.5ms (1.12ms)
		{								
			sIrKeyVal &= ~0x80000000;		//logic 0
		}
		else if ((TrigPeriod > 15) && (TrigPeriod < 28))	//1.5ms~2.8ms (2.24ms)
		{											
			sIrKeyVal |= 0x80000000;		//logic 1
		} 
		else 
		{
			IsRcvData = FALSE;
			return;
		} 

		IsIrIgnoreLeader = FALSE;

		if((++RcvBits) == 32)
		{
			IrKeyVal = sIrKeyVal;
			sIrWakeupKeyVal = sIrKeyVal;
			IsIrShortPrs = TRUE;
			IsIrContinuePrs = FALSE;
			IsRcvData = FALSE;
			DBG("Key: %08LX\n", IrKeyVal);
		}
	}	

//	DBG(("*RcvBits:%u\n",(uint8_t)RcvBits));
}

__attribute__((section(".driver.isr"))) void GpioInterrupt(void)
{   
	IrDecdInterrupt();	
	GpioIntClr(GPIO_C_INT, GPIOC2);
}

__attribute__((section(".driver.isr"))) void Timer1Interrupt(void)
{
	Timer1IntClr();
	Timer100usCnt++; //for remote 
}	  

void IrGpioInterruptInit(void)
{
    uint32_t Port = GPIOC2;

	GpioSetRegOneBit(GPIO_C_IE, Port);
	GpioClrRegOneBit(GPIO_C_OE, Port);
	GpioSetRegOneBit(GPIO_C_PU, Port);
	GpioClrRegOneBit(GPIO_C_PD, Port);  

	//enable int
	GpioIntEn(GPIO_C_INT,Port, GPIO_NEG_EDGE_TRIGGER);
    
    //enable gpio irqc
    NVIC_EnableIRQ(GPIO_IRQn);
}

//get ir key index
//static uint8_t GetIrKeyIndex(void)
//{
//	static uint8_t KeyIndex = 0xFF;
//	static uint8_t IrCphCnt = 0;

//	if(IsIrShortPrs || IsIrContinuePrs)
//	{
//		//fast response
//		if(IsIrShortPrs)
//		{
//			IrCphCnt = 0;
//		}
//		if(IrCphCnt < 5)
//		{
//			IrCphCnt++;
//		}
//		TimeOutSet(&IrKeyHoldTimer, 70 * IrCphCnt);
//		
//		IsIrShortPrs = FALSE;
//		IsIrContinuePrs = FALSE;

//		if(((IrKeyVal >> 24) & 0x000000FF) + ((IrKeyVal >> 16) & 0x000000FF) != 0xFF)
//		{
//			KeyIndex = 0xFF;
//			return 0xFF;
//		}

//		if((IrKeyVal & 0x0000FFFF) == IR_MANU_ID)
//		{
//			for(KeyIndex = 0; KeyIndex < IR_KEY_SUM; KeyIndex++)
//			{
//				if(((IrKeyVal >> 24) & 0x000000FF) == gIrVal[KeyIndex])
//				{
//					return KeyIndex;
//				}
//			}
//		}
//		KeyIndex = 0xFF;
//		return 0xFF;
//	}
//	else if(!IsTimeOut(&IrKeyHoldTimer))
//	{
//		return KeyIndex;
//	}
//	else
//	{
//		IrKeyVal = 0;
//		return 0xFF;
//	}
//}	 

int32_t IrKeyInit(void)
{
//	IrKeyState = IR_KEY_STATE_IDLE;
	TimeOutSet(&IrKeyScanTimer, 0);	
	NVIC_EnableIRQ(TMR1_IRQn);
	Timer1HaltModeSet(1); //halt mode
	Timer1Set(100);// 100us 	
	IrGpioInterruptInit();
	return 0;
}

int32_t main(void)
{   
//    uint8_t Buf[10];
    
    ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating

	ClkPorRcToDpll(0);                                  //clock src is 32768hz OSC 
    CacheInit();    
	/*
	 * Now, uart has been initialized automatically by OS.
	 * if you want to customize debug uart setting(defaut 115200-8-N-1),
	 * you can invoke the function at any place you want(for example here).
	 */
	GpioFuartRxIoConfig(0);
	GpioFuartTxIoConfig(0);
	FuartInit(115200, 8, 0, 1);		       
	IrKeyInit();       
    APP_DBG("\n");
    APP_DBG("/==========================================================================\\\n");
    APP_DBG("|                   Ir Software Decode TESTBENCH                           |\n");
    APP_DBG("| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |\n");
    APP_DBG("\\==========================================================================/\n");
    APP_DBG("\n");

    while(1)
    { 
        WdgFeed();
    }
}

