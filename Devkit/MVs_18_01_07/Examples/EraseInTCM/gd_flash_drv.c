/**
 *****************************************************************************
 * NAME:
 *	gd_flash_ctrl.c
 *
 * DESCRIPTION:
 *	The functions used only for GD flash
 *****************************************************************************
 */

#include "type.h"
#include "stdio.h"
#include "spi_flash.h"
#include "gd_flash_drv.h"

#include "irqs.h"

extern void  LED_1MS_SCAN(void);

uint32_t GetIRQSource = 0;
extern uint8_t test_data[];
extern bool flash_erase_flag;

__attribute__((section("CACHE.4KBMEM")))
static void GDSpiCodeFlashSendCmd(uint8_t FlashCmd, uint8_t CmdLen, uint32_t AddrLen, uint8_t CmdType, uint16_t DataLen);

__attribute__((section("CACHE.4KBMEM")))
static void GDSpiCodeFlashResetCRM(void);

__attribute__((section("CACHE.4KBMEM")))
static bool GDSpiCodeFlashIsFifoEmpty(void);

__attribute__((section("CACHE.4KBMEM")))
static uint32_t GDSpiCodeFlashReadStatus(void);

__attribute__((section("CACHE.4KBMEM")))
static void GDSpiCodeFlashControllerBusyStatusSet(uint32_t BusyFlag);

__attribute__((section("CACHE.4KBMEM")))
static bool GDSpiCodeFlashIsBusy(void);

__attribute__((section("CACHE.4KBMEM")))
static int32_t GDSpiCodeFlashHpmModeSet(uint32_t HpmMode);

__attribute__((section("CACHE.4KBMEM")))	
void SpiCodeFlashErase(SPI_FLASH_INFO* FlashInfo,ERASE_TYPE_ENUM erase_type, uint32_t index, bool IsSuspend)
{
	uint8_t Cmd;
	uint8_t CmdLen;
	uint32_t Addr;
	uint32_t AddrLen;
	uint32_t CmdType;
	//uint8_t i;
	bool hpm_flag = (*(volatile unsigned long *) 0x40006040);//REG_FSHC_BYPASS_HPM;

	//uint16_t led_loop1=10;
		
	#if 0
	GetIRQSource = 0;
	for(i = 0; i < 16; i++)
	{
		GetIRQSource <<= 1;
		GetIRQSource |= NVIC_GetIRQCtlStat((IRQn_Type)i);
	}
	for(i = 0; i < 16; i++)
	{
		NVIC_DisableIRQ((IRQn_Type)i);
	}
	#else
	__DISABLE_FAULT_IRQ();
	#endif

	
	if((*(volatile unsigned long *)0x40006050) & 0x01)
	{
			GDSpiCodeFlashResetCRM( );
	}

		
	//增加index 合法性检查，防止误差code段,这里示例1M为code空间
//		if((erase_type == SECTOR_ERASE && index < 256) || (erase_type == BLOCK_ERASE && index < 16))
//			return;
	
	switch(erase_type)
	{
		case CHIP_ERASE:
			return ;	//SDK中禁止chip erase
			//Cmd = FLASH_CE;
			//Addr = 0;
			//AddrLen = 0;
			//CmdType = CMD_ONLY_1BIT;
			//break;
		case SECTOR_ERASE:
			Cmd = 0x20;//FLASH_SE;
			Addr = index * (4 * 1024);//SECTOR_SIZE;
			AddrLen = 0x17;//ADDR_LEN;
			CmdType = 0x60;//CMD_ADDR_1BIT;
			break;
		case BLOCK_ERASE:
			Cmd = 0xD8;//FLASH_BE_64;
			Addr = index * (44 * 1024);//BLOCK_SIZE;
			AddrLen = 0x17;//ADDR_LEN;
			CmdType = 0x60;//CMD_ADDR_1BIT;
			break;
		default:
			return;
	}
		
		
		
		
	CmdLen = 0x07;//CMD_LEN;
	
	if(AddrLen != 0)
	{
		(*(volatile unsigned long *) 0x40006014) = Addr; //REG_FSHC_ADDR_REG = Addr;
	}

	GDSpiCodeFlashSendCmd(0x06, 0x07, 0, 0xE0, 0);
	GDSpiCodeFlashSendCmd(Cmd, CmdLen, AddrLen, CmdType, 0);

	GDSpiCodeFlashControllerBusyStatusSet(1);
		
	if(!IsSuspend || erase_type == CHIP_ERASE)
    {
		while(!GDSpiCodeFlashIsBusy( ))
		{
			extern int led_loop1;
			if(led_loop1++>20)
			{
				led_loop1=0;
				//LED_1MS_SCAN();
				printf("uart send in flash erasing\r\n");
			}
		}
		GDSpiCodeFlashControllerBusyStatusSet(0);
    }
	
	if(!hpm_flag)
	{
			GDSpiCodeFlashHpmModeSet(1);
	}


	#if 0
	for(i = 15; i >= 0; i--)
	{
		if((GetIRQSource & 0x01) != 0)
		{
			NVIC_EnableIRQ((IRQn_Type)i);
		}
		GetIRQSource >>= 1;
	}
	#else
	__ENABLE_FAULT_IRQ();
	#endif
}

#if 0

__attribute__((section("CACHE.4KBMEM")))
SPI_CODE_FLASH_ERR_CODE GDFlashLock(SPI_FLASH_INFO* FlashInfo, PROTECT_AREA ProtectAreaSel)
{
	uint16_t   	Status;
	uint16_t	ProtectVal;
	uint8_t		DataLen;

	__DISABLE_FAULT_IRQ();
	
	if((*(volatile unsigned long *)0x40006050) & 0x01)
	{
		GDSpiCodeFlashResetCRM();
	}

	switch(ProtectAreaSel)
	{
		case HALF_OF_AREA:						//half code area protect
			ProtectVal = FlashInfo->HalfAreaProtect;
			break;

		case THREE_QUARTERS_OF_AREA:			//Three Quarter Area Protect
			ProtectVal = FlashInfo->ThreeQuarterAreaProtect;
			break;

		case SEVENTH_EIGHT_OF_AREA:				//Seven Eighths Area Protect
			ProtectVal = FlashInfo->SevenEighthsAreaProtect;
			break;

		case ALL_OF_AREA:						//All Area Protect
			ProtectVal = FlashInfo->AllAreaProtect;
			break;

		default:
			__ENABLE_FAULT_IRQ();
			return CODE_AREA_PROTECT_NOT_SUPPORT;        
	}

	if(ProtectVal == 0x7F)
	{
		__ENABLE_FAULT_IRQ();
		return CODE_AREA_PROTECT_NOT_SUPPORT;   
	}

	Status = GDSpiCodeFlashReadStatus(); 
	Status &= ~(0x0180);


	Status &= 0xBF83;     // ~(0x1F << 2) & clr cmp bit
	Status |= ((ProtectVal & 0x1F) << 2);                    
	if(ProtectVal & 0x40)
	{
		Status |= ((ProtectVal & 0x20) << 9);    
	}
	(*(volatile unsigned long *)0x40006004) = Status;
	DataLen = 1;

	GDSpiCodeFlashSendCmd(0x06, 0x07, 0, 0xE0, 0);
	GDSpiCodeFlashSendCmd(0x01, 0x07, 0, 0xA0, DataLen);

	GDSpiCodeFlashControllerBusyStatusSet(1);
	while(!GDSpiCodeFlashIsBusy());
	GDSpiCodeFlashControllerBusyStatusSet(0);
	GDSpiCodeFlashHpmModeSet(1);

	__ENABLE_FAULT_IRQ();
	return FLASH_NONE_ERR;
}

__attribute__((section("CACHE.4KBMEM")))
SPI_CODE_FLASH_ERR_CODE GDFlashUnlock(SPI_FLASH_INFO* FlashInfo)
{
	uint16_t    UnLockStatus;
	uint16_t    UnLockProtectVal;
	uint8_t		UnLockDataLen;

	__DISABLE_FAULT_IRQ();
	
	// 1
	if((*(volatile unsigned long*)0x40006050) & 0x01)
	{
		GDSpiCodeFlashResetCRM();
	}
	UnLockStatus = GDSpiCodeFlashReadStatus();  
	UnLockStatus &= ~(0x0180);

	// 2
	UnLockStatus &= 0xBF83;     // ~(0x1F << 2) & clr cmp bit

	UnLockProtectVal = FlashInfo->NoneAreaProtect;
	UnLockStatus |= ((UnLockProtectVal & 0x1F) << 2);                    
	if(UnLockProtectVal & 0x40)
	{
	    UnLockStatus |= ((UnLockProtectVal & 0x20) << 9);    
	}
	(*(volatile unsigned long *)0x40006004) = UnLockStatus;
	UnLockDataLen = 1;
	GDSpiCodeFlashSendCmd(0x06, 0x07, 0, 0xE0, 0);

	// 3
	GDSpiCodeFlashSendCmd(0x01, 0x07, 0, 0xA0, UnLockDataLen);

	GDSpiCodeFlashControllerBusyStatusSet(1);
	while(!GDSpiCodeFlashIsBusy());
	GDSpiCodeFlashControllerBusyStatusSet(0); 

	GDSpiCodeFlashHpmModeSet(1);

	__ENABLE_FAULT_IRQ();
	
	return FLASH_NONE_ERR;
}
#endif


__attribute__((section("CACHE.4KBMEM")))
int FlashEraseInTCM(SPI_FLASH_INFO* FlashInfo,uint32_t Offset,uint32_t Size)
{
	//uint8_t Mode;
	int i,Echo,Round;

	Echo = -22;//EINVAL;
	if(FlashInfo->Capacity < Offset + Size)
		return Echo;

	if(Size >= 64 * 1024 && FlashInfo->Mid != 0xBF){
		if(Offset & (0x10000 - 1)){
			/*
			* sector size align
			*/
			if(Size & (0x1000 - 1))
				Size = ((Size >> 12) + 1) << 12;

			if(Offset & (0x1000 - 1)){
				Offset = (Offset >> 12) << 12;
				Size += 0x1000;
			}
			/*
			* judge the offset and size 
			*/
			if(FlashInfo->Capacity < Offset + Size)
			   return Echo;

			Round = (((0x10000 - Offset) & 0xFFFF)) >> 12;
			for(i = 0;i < Round;i ++){
				SpiCodeFlashErase(FlashInfo, SECTOR_ERASE, Offset / 0x1000,0);
				Size -= 0x1000;
				Offset += 0x1000;
			}
		}

		if(Size >= 64 * 1024){
			Round = Size >> 16;
			for(i = 0;i < Round;i ++){
				SpiCodeFlashErase(FlashInfo,BLOCK_ERASE,Offset / 0x10000,0);
				Size -= 0x10000;
				Offset += 0x10000;
			}
		}
	}

	/*
	* sector size align
	*/
	if(Size & (0x1000 - 1))
		Size = ((Size >> 12) + 1) << 12;

	if(Offset & (0x1000 - 1)){
		Offset = (Offset >> 12) << 12;
		Size += 0x1000;
	}

	if(FlashInfo->Capacity < Offset + Size)
		return Echo;

	Round = Size >> 12;
	for(i = 0;i < Round;i ++){
		SpiCodeFlashErase(FlashInfo, SECTOR_ERASE, Offset / 0x1000,0);
		Offset += 0x1000;
	}
	Echo = 0;

	return Echo;
}


__attribute__((section("CACHE.4KBMEM")))
static void GDSpiCodeFlashSendCmd(uint8_t FlashCmd, uint8_t CmdLen, uint32_t AddrLen, uint8_t CmdType, uint16_t DataLen)
{
	 (*(volatile unsigned long *)0x40006010) = FlashCmd;
	 (*(volatile unsigned long *)0x40006018) = (CmdLen << 6) | AddrLen;
	 (*(volatile unsigned long *)0x4000601C) = CmdType;
	 (*(volatile unsigned long *)0x4000600C) = DataLen;

	 (*(volatile unsigned long *)0x40006020) = 0x01;
	 (*(volatile unsigned long *)0x40006020) = 0x00;

	 while((*(volatile unsigned long *)0x4000602C) & 0x01);
}

__attribute__((section("CACHE.4KBMEM")))
static void GDSpiCodeFlashResetCRM(void)
{
	GDSpiCodeFlashSendCmd(0xFF, 0x07, 0, 0xE0, 0);
}
__attribute__((section("CACHE.4KBMEM")))
static bool GDSpiCodeFlashIsFifoEmpty(void)
{
	return ((*(volatile unsigned long*)0x40006024) & 0x01);	
}
__attribute__((section("CACHE.4KBMEM")))
static uint32_t GDSpiCodeFlashReadStatus(void)
{
	uint32_t	Status = 0;
	uint32_t	Tmp;

	if((*(volatile unsigned long *)0x40006050) & 0x01)
	{
		GDSpiCodeFlashResetCRM();
	}

	GDSpiCodeFlashSendCmd(0x05, 0x07, 0, 0x0C0, 0);
	while(GDSpiCodeFlashIsFifoEmpty());
	Status = *(volatile unsigned long *)0x40006000;
	
	GDSpiCodeFlashSendCmd(0x35, 0x07, 0, 0xC0, 0);
	while(GDSpiCodeFlashIsFifoEmpty());
	Tmp = *(volatile unsigned long *)0x40006000;

	Status = (Tmp << 8) | Status;

	 return Status;
}
__attribute__((section("CACHE.4KBMEM")))
static void GDSpiCodeFlashControllerBusyStatusSet(uint32_t BusyFlag)
{
	if(BusyFlag)
    {
	    *(volatile unsigned long *)0x40006008 |= 0x08;
    }
    else
    {
        *(volatile unsigned long *)0x40006008 &= ~0x08;   
    }
}
__attribute__((section("CACHE.4KBMEM")))
static bool GDSpiCodeFlashIsBusy(void)
{
	uint32_t		    Status;	

	Status = GDSpiCodeFlashReadStatus();
	if(Status & 0x01)
	{
		return FALSE;	
	}
	else
	{
		return TRUE;
	}
}
__attribute__((section("CACHE.4KBMEM")))
static int32_t GDSpiCodeFlashHpmModeSet(uint32_t HpmMode)
{
	int32_t Echo = 0;

	switch(HpmMode)
	{
	case 0:
		(*(volatile unsigned long *)0x40006040) = 0;
		break;

	case 1:
		if((*(volatile unsigned long *)0x40006040) == 0)
			GDSpiCodeFlashSendCmd(0xA3,0x18 + 7,0,0xE0,0);
		break;

	case 2:
		GDSpiCodeFlashSendCmd(0xA3,0x18 + 7,0,0xE0,0);
		(*(volatile unsigned long *)0x40006040) = 0;
		break;

	case 3:
		(*(volatile unsigned long *)0x40006040) = 1;		
		(*(volatile unsigned long *)0x40006048) = 1;
		(*(volatile unsigned long *)0x40006048) = 0;
		break;

	default:
		Echo = -38;//ENOSYS;
		break;
	}

	return Echo;
}


