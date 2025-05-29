#include "type.h"
#include "clk.h"
#include "gpio.h"
#include "watchdog.h"
#include "timer.h"
#include "cache.h"
#include "led_example.h"

#define MAX_LED_PIN_NUM 7
uint8_t	gDispBuff[7] = {0,0,0,0,0,0,0};

int32_t main(void)
{    
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);
	ClkPorRcToDpll(0);
	CacheInit();          
    
    LedPinGpioInit();
    
    Timer1Set(2000);
    NVIC_EnableIRQ(TMR1_IRQn);
    
    gDispBuff[0] = NUM_1;//show 1234 in the led panel
    gDispBuff[1] = NUM_2;
    gDispBuff[2] = NUM_3;
    gDispBuff[3] = NUM_4;
    while(1)
    {        
        WdgFeed();
    }
}

//LED gpio init function
void LedPinGpioInit(void)
{
	GpioSetRegOneBit(LED_PIN1_PORT_PU, LED_PIN1_BIT);
	GpioSetRegOneBit(LED_PIN2_PORT_PU, LED_PIN2_BIT);
	GpioSetRegOneBit(LED_PIN3_PORT_PU, LED_PIN3_BIT);
	GpioSetRegOneBit(LED_PIN4_PORT_PU, LED_PIN4_BIT);
	GpioSetRegOneBit(LED_PIN5_PORT_PU, LED_PIN5_BIT);
	GpioSetRegOneBit(LED_PIN6_PORT_PU, LED_PIN6_BIT);
	GpioSetRegOneBit(LED_PIN7_PORT_PU, LED_PIN7_BIT);
	GpioClrRegOneBit(LED_PIN1_PORT_PD, LED_PIN1_BIT);
	GpioClrRegOneBit(LED_PIN2_PORT_PD, LED_PIN2_BIT);
	GpioClrRegOneBit(LED_PIN3_PORT_PD, LED_PIN3_BIT);
	GpioClrRegOneBit(LED_PIN4_PORT_PD, LED_PIN4_BIT);
	GpioClrRegOneBit(LED_PIN5_PORT_PD, LED_PIN5_BIT);
	GpioClrRegOneBit(LED_PIN6_PORT_PD, LED_PIN6_BIT);
	GpioClrRegOneBit(LED_PIN7_PORT_PD, LED_PIN7_BIT);
	GpioClrRegOneBit(LED_PIN1_PORT_OE, LED_PIN1_BIT);
	GpioClrRegOneBit(LED_PIN2_PORT_OE, LED_PIN2_BIT);
	GpioClrRegOneBit(LED_PIN3_PORT_OE, LED_PIN3_BIT);
	GpioClrRegOneBit(LED_PIN4_PORT_OE, LED_PIN4_BIT);
	GpioClrRegOneBit(LED_PIN5_PORT_OE, LED_PIN5_BIT);
	GpioClrRegOneBit(LED_PIN6_PORT_OE, LED_PIN6_BIT);
	GpioClrRegOneBit(LED_PIN7_PORT_OE, LED_PIN7_BIT);
	GpioClrRegOneBit(LED_PIN1_PORT_ICS_1MA7, LED_PIN1_BIT);
	GpioClrRegOneBit(LED_PIN2_PORT_ICS_1MA7, LED_PIN2_BIT);
	GpioClrRegOneBit(LED_PIN3_PORT_ICS_1MA7, LED_PIN3_BIT);
	GpioClrRegOneBit(LED_PIN4_PORT_ICS_1MA7, LED_PIN4_BIT);
	GpioClrRegOneBit(LED_PIN5_PORT_ICS_1MA7, LED_PIN5_BIT);
	GpioClrRegOneBit(LED_PIN6_PORT_ICS_1MA7, LED_PIN6_BIT);
	GpioClrRegOneBit(LED_PIN7_PORT_ICS_1MA7, LED_PIN7_BIT);
	GpioClrRegOneBit(LED_PIN1_PORT_ICS_2MA4, LED_PIN1_BIT);
	GpioClrRegOneBit(LED_PIN2_PORT_ICS_2MA4, LED_PIN2_BIT);
	GpioClrRegOneBit(LED_PIN3_PORT_ICS_2MA4, LED_PIN3_BIT);
	GpioClrRegOneBit(LED_PIN4_PORT_ICS_2MA4, LED_PIN4_BIT);
	GpioClrRegOneBit(LED_PIN5_PORT_ICS_2MA4, LED_PIN5_BIT);
	GpioClrRegOneBit(LED_PIN6_PORT_ICS_2MA4, LED_PIN6_BIT);
	GpioClrRegOneBit(LED_PIN7_PORT_ICS_2MA4, LED_PIN7_BIT);
}

//LED gpio init function
void LedAllPinGpioInput(void)
{
	GpioClrRegBits(GPIO_B_OE, (GPIOB2 | GPIOB3 | GPIOB4 | GPIOB5 | GPIOB6 | GPIOB7));
	GpioClrRegBits(GPIO_A_OE, GPIOA25);
	GpioClrRegBits(GPIO_B_PULLDOWN1, (GPIOB2 | GPIOB3 | GPIOB4 | GPIOB5 | GPIOB6 | GPIOB7));
	GpioClrRegBits(GPIO_B_PULLDOWN2, (GPIOB2 | GPIOB3 | GPIOB4 | GPIOB5 | GPIOB6 | GPIOB7));
	GpioClrRegBits(GPIO_A_PULLDOWN1, GPIOA25);
	GpioClrRegBits(GPIO_A_PULLDOWN2, GPIOA25);
}

//LED scanner function
void LedFlushDisp(void)
{
	static uint8_t ScanPinNum = 0;    
    
	ScanPinNum++;
	if(ScanPinNum >= (MAX_LED_PIN_NUM + 1))
	{
		ScanPinNum = 1;
	}

	LedAllPinGpioInput();
	switch(ScanPinNum)
	{
		case 1:
			if(gDispBuff[0] & SEG_A)
			{
				LED_PIN2_IN_ON;
			}

			if(gDispBuff[0] & SEG_B)
			{
				LED_PIN3_IN_ON;
			}

			if(gDispBuff[0] & SEG_E)
			{
				LED_PIN4_IN_ON;
			}

			if(gDispBuff[4] & SEG_A)
			{
				LED_PIN6_IN_ON;
			}	//PLAY

			if(gDispBuff[4] & SEG_D)
			{
				LED_PIN5_IN_ON;
			}	//SD

			if(gDispBuff[3] & SEG_C)
			{
				LED_PIN7_IN_ON;
			}
			LED_PIN1_OUT_HIGH;
			break;

		case 2:
			if(gDispBuff[0] & SEG_F)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[1] & SEG_A)
			{
				LED_PIN3_IN_ON;
			}

			if(gDispBuff[1] & SEG_B)
			{
				LED_PIN4_IN_ON;
			}

			if(gDispBuff[1] & SEG_E)
			{
				LED_PIN5_IN_ON;
			}

			if(gDispBuff[1] & SEG_D)
			{
				LED_PIN6_IN_ON;
			}

			if(gDispBuff[3] & SEG_G)
			{
				LED_PIN7_IN_ON;
			}
			LED_PIN2_OUT_HIGH;
			break;

		case 3:
			if(gDispBuff[0] & SEG_G)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[1] & SEG_F)
			{
				LED_PIN2_IN_ON;
			}

			if(gDispBuff[4] & SEG_E)
			{
				LED_PIN4_IN_ON;
			}  //K5 DOT

			if(gDispBuff[2] & SEG_B)
			{
				LED_PIN5_IN_ON;
			}

			if(gDispBuff[4] & SEG_B)
			{
				LED_PIN6_IN_ON;
			}  //K2 PAUSE

			if(gDispBuff[4] & SEG_G)
			{
				LED_PIN7_IN_ON;
			}  //K7 MP3
			LED_PIN3_OUT_HIGH;
			break;

		case 4:
			if(gDispBuff[0] & SEG_C)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[1] & SEG_G)
			{
				LED_PIN2_IN_ON;
			}

			if(gDispBuff[2] & SEG_F)
			{
				LED_PIN3_IN_ON;
			}

			if(gDispBuff[2] & SEG_C)
			{
				LED_PIN5_IN_ON;
			}

			if(gDispBuff[3] & SEG_E)
			{
				LED_PIN6_IN_ON;
			}

			if(gDispBuff[3] & SEG_B)
			{
				LED_PIN7_IN_ON;
			}
			LED_PIN4_OUT_HIGH;
			break;

		case 5:
			if(gDispBuff[0] & SEG_D)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[1] & SEG_C)
			{
				LED_PIN2_IN_ON;
			}

			if(gDispBuff[2] & SEG_G)
			{
				LED_PIN3_IN_ON;
			}

			if(gDispBuff[2] & SEG_A)
			{
				LED_PIN4_IN_ON;
			}
			LED_PIN5_OUT_HIGH;
			break;

		case 6:
			if(gDispBuff[2] & SEG_D)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[4] & SEG_C)
			{
				LED_PIN2_IN_ON;
			}  //K3 USB

			if(gDispBuff[2] & SEG_E)
			{
				LED_PIN3_IN_ON;
			}

			if(gDispBuff[3] & SEG_D)
			{
				LED_PIN4_IN_ON;
			}
			LED_PIN6_OUT_HIGH;
			break;

		case 7:
			if(gDispBuff[3] & SEG_F)
			{
				LED_PIN1_IN_ON;
			}

			if(gDispBuff[4] & SEG_F)
			{
				LED_PIN3_IN_ON;
			} //K6 FM

			if(gDispBuff[3] & SEG_A)
			{
				LED_PIN4_IN_ON;
			}
			LED_PIN7_OUT_HIGH;
			break;

		default:
			break;
	}
}

__attribute__((section(".driver.isr"))) void Timer1Interrupt(void)
{
	Timer1IntClr();
    LedFlushDisp();
}

