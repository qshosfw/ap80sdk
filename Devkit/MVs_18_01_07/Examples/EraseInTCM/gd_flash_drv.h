/**
 *****************************************************************************
 * NAME:
 *	gd_flash_ctrl.h
 *
 * DESCRIPTION:
 *	The functions used only for GD flash
 *****************************************************************************
 */
#include "spi_code_flash.h"

typedef enum{
	HALF_OF_AREA = 1,			// 1/2
	THREE_QUARTERS_OF_AREA,		// 3/4
	SEVENTH_EIGHT_OF_AREA,		// 7/8
	ALL_OF_AREA
} PROTECT_AREA;

int FlashEraseInTCM(SPI_FLASH_INFO* FlashInfo,uint32_t Offset,uint32_t Size);

//SPI_CODE_FLASH_ERR_CODE GDFlashLock(SPI_CODE_FLASH_INFO* FlashInfo, PROTECT_AREA ProtectAreaSel);
//SPI_CODE_FLASH_ERR_CODE GDFlashUnlock(SPI_CODE_FLASH_INFO* FlashInfo);
