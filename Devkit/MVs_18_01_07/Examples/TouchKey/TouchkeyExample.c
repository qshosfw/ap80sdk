#include "type.h"
#include "uart.h"
#include "clk.h"
#include "spi_flash.h"
#include "cache.h"
#include "gpio.h"
#include "watchdog.h"
#include "uart.h"
#include "touchkey.h"
#include "delay.h"
#include <stdio.h> 

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

uint8_t UartRec;
#define 	SetKeyValue(x)  (UartRec = (x)) 
#define 	GetKeyValue() 	(UartRec)
#define 	ClrKeyValue() 	(UartRec = 0) 

int32_t main(void)
{
	uint16_t Dout = 0;
	uint8_t TkChannel = 0;
	
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);        //open all clk gating
	ClkPorRcToDpll(0);              //clock src is 32768hz OSC
	CacheInit();
	
	WaitMs(200);
	WdgDis();
	/*
	 * Now, uart has been initialized automatically by OS.
	 * if you want to customize debug uart setting(defaut 115200-8-N-1),
	 * you can invoke the function at any place you want(for example here).
	 */
	GpioFuartRxIoConfig(1);
	GpioFuartTxIoConfig(1);
	FuartInit(115200, 8, 0, 1);
	TouchKeyModuleRst();  //Reset touchkey module for PD signal
	TouchKeyPortInit(0xFF);  //0X3FC0  :gpioc[8:1]   0x3f  :gpioB[31:26]

	TouchKeyStartConvert(TkChannel); //first start the touchkey model 
	
	APP_DBG("*****************************\n");
	APP_DBG("TouchKey driver example!\n");
	
	while(1)
	{
		if(TouchKeyIsConvertOK())//转换完成
		{		
			Dout =  TouchKeyGetDout();//获取Dout
			
			APP_DBG("TkChannel %d,the dout is %d\n",TkChannel,Dout);

			if(TkChannel < 7) 
			{
				TkChannel++;
			}
			else 
			{
				TkChannel = 0;
			}
			
			TouchKeyStartConvert(TkChannel);
		}
		else
		{
			APP_DBG("No channel ready\n");
		}
		WaitMs(1);
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
