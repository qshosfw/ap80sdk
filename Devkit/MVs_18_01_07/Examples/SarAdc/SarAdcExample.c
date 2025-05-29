#include <stdio.h>
#include "type.h"
#include "uart.h"
#include "clk.h"
#include "spi_flash.h"
#include "cache.h"
#include "gpio.h"
#include "adc.h"
#include "watchdog.h"
#include "uart.h"
#include "delay.h"

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define		ADC_KEY_COUNT				10 			//key count per adc channel
#define		ADC_KEY_FULL_VAL			4096		//

uint8_t UartRec;
#define 	SetKeyValue(x)  (UartRec = (x)) 
#define 	GetKeyValue() 	(UartRec)
#define 	ClrKeyValue() 	(UartRec = 0) 

//extern uint16_t SarAdcGetGpioVoltage(uint8_t Channel);

int32_t main(void)
{
	int16_t adcvol = 0;
	uint8_t	KeyIndex;
	uint8_t j = 0;

	ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating
	ClkPorRcToDpll(0);              //clock src is 32768hz OSC
	CacheInit();
	
	WaitMs(200);
	WdgDis();
	
	SarAdcLdoinVolInit();    //SarAdc绝对电压检测初始化相关参数
	/*
	 * Now, uart has been initialized automatically by OS.
	 * if you want to customize debug uart setting(defaut 115200-8-N-1),
	 * you can invoke the function at any place you want(for example here).
	 */
	GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);
	FuartInit(115200, 8, 0, 1);
	
	APP_DBG("*****************************\n");
	APP_DBG("SarAdc driver example!\n");
	APP_DBG("please input select:\n");
	APP_DBG("input a for performance relative voltage\n");
	APP_DBG("input b for performance absolute voltage\n");
	while(1)
	{
		j = GetKeyValue();
		switch(j)
		{
			case 'a':  //相对电压-----使用时请确保外部连接有ADC_KEY				
				j = 0;
				ClrKeyValue();
				SarAdcGpioSel(ADC_CHANNEL_B22);
				APP_DBG("---------------------------\n");
				APP_DBG("please press an key:\n");
				while(1)
				{
					adcvol = SarAdcChannelGetValue(ADC_CHANNEL_B22);

					if(adcvol >= (ADC_KEY_FULL_VAL - (ADC_KEY_FULL_VAL / ADC_KEY_COUNT)))
					{
						WaitMs(1);
						continue;	//no pressed key.
					}
					KeyIndex = (adcvol + (ADC_KEY_FULL_VAL / ADC_KEY_COUNT) / 2) * ADC_KEY_COUNT / ADC_KEY_FULL_VAL;
					APP_DBG("---------------------------\n");
					APP_DBG("input Key is %d\n", KeyIndex);
					break;
				}
				APP_DBG("***************************\n");
				APP_DBG("please input select:\n");
				break;
			case 'b': //绝对电压 
				j = 0;
				ClrKeyValue();
				SarAdcGpioSel(ADC_CHANNEL_B5);
				adcvol = SarAdcGetLdoinVoltage();
				APP_DBG("---------------------------\n");
				APP_DBG("the chip LDOIN voltage is %d mV\n", adcvol);			
				adcvol=  SarAdcGetGpioVoltage(ADC_CHANNEL_B5);
				APP_DBG("the GPIOB5 absolute voltage is %d mV\n", adcvol);
				APP_DBG("***************************\n");
				APP_DBG("please input select:\n");
				break;
			default:
				break;
		}
	}
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
        
        //if(!(c & 0x80))
        //{
        //    FuartSend(&c, 1);
        //}

        SetKeyValue(c);
		//UartRec = c;
	}

    //If data sent
	if(IsFuartTxIntSrc())
    {
		//clear interrupt
        FuartClrTxInt();
	}
}
