#include "type.h"
#include "clk.h"
#include "watchdog.h"
#include "timer.h"
#include "lcd_seg.h"
#include "cache.h"

ST_LCD_SEG_CTRL g_st_lcd_ctrl;    
uint8_t LcdBuf[60];
uint8_t ShowData[8] = {0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00};//show com1+seg1

void LcdInit(void)
{
    uint8_t Voltage = 7;//LCD top voltage selection,thi pad is VDD33
    g_st_lcd_ctrl. LcdSegMask = 0x7FF;//SEG B20~B30
    g_st_lcd_ctrl. LcdComMask= 0xF000;//COM C0~C3
    g_st_lcd_ctrl. LcdBias = 0;
    g_st_lcd_ctrl. LcdDispData = LcdBuf;
    LcdsegInit(&g_st_lcd_ctrl,Voltage);
}

void LcdSetData(void)
{
    ShowData [0]=0x00;//show 1234
    ShowData [1]=0x68;
    ShowData [2]=0x00;
    ShowData [3]=0xFE;
    ShowData [4]=0x00;
    ShowData [5]=0xA6;
    ShowData [6]=0x00;
    ShowData [7]=0x28;
    LcdsegSetData(ShowData);
}

int32_t main(void)
{    
	ClkModuleEn(ALL_MODULE_CLK_SWITCH);	
	ClkModuleGateEn(ALL_MODULE_CLK_GATE_SWITCH);
	ClkPorRcToDpll(0);
    CacheInit();
	
	LcdCtrlRegInit();        //do NOT remove this
    
    LcdInit();
    LcdSetData();
    
    Timer1Set(2000);
    NVIC_EnableIRQ(TMR1_IRQn);
    
    while(1)
    {        
        WdgFeed();
    }
}

__attribute__((section(".driver.isr"))) void Timer1Interrupt(void)
{
	Timer1IntClr();
    LcdsegSetRefMem();
}
