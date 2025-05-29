/**
 *****************************************************************************
 * @file     spi_code_flash.h
 * @author   Richard
 * @version  V1.0.6
 * @date     30-May-2013
 * @brief    spi code flash module driver interface
 *****************************************************************************
 * @attention 
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */
 
#ifndef __SPI_CODE_FLASH_H__
#define __SPI_CODE_FLASH_H__

//#include "OrioleReg.h"

#define     POSEDGE_SEL          (0)       /**<sampling at clk posedge */
#define     NEGEDGE_SEL          (1)       /**<sampling at clk negedge */
#define     DELAY_CHAIN_SEL      (4)       /**<sampling at the n clks */

#define     FLASH_GD             (0xC8)    /**<MID */
#define     FLASH_MX             (0xC2)    /**<MID */
#define     FLASH_WINBOUND       (0xEF)    /**<MID */
#define     FLASH_PCT            (0xBF)    /**<MID */
#define     FLASH_EON            (0x1C)    /**<MID */
#define     FLASH_BG             (0xE0)    /**<MID */
#define     FLASH_ESMT           (0x8C)    /**<MID */
#define			FLASH_FT			 (0x0E)    /**<MID */

#define     FLASH_HALF_PROTECT              (1)
#define     FLASH_THREE_QUARTER_PROTECT     (2)
#define     FLASH_SEVEN_EIGHTH_PROTECT      (3)
#define     FLASH_ALL_PROTECT               (4)

#define 		FLASH_GD_VERSION_A		(0)
#define 		FLASH_GD_VERSION_B		(1)
#define 		FLASH_GD_VERSION_C		(2)

/**
 * err code definition
 */

#pragma pack(1)

typedef enum {
	CHIP_ERASE,
	SECTOR_ERASE,
	BLOCK_ERASE
}ERASE_TYPE_ENUM;

/**
 * flash structure define 
 * each byte definition like:    A B CMP SEC/BP4 TB/BP3 BP2 BP1 BP0 <A:area protection support flag,B:CMP bit exist flag>
 */
typedef struct _SPI_CODE_FLASH_INFO
{
    __IO  uint8_t      Mid;             /**<Manufacturer's ID*/    
    __IO  uint16_t      Did;             /**<Device ID*/
    __IO  uint8_t      SqiModeFlag;     /**<bit0:0:spi mode, 1:sqi mode. r/w*/   
    __IO  uint8_t      SqiWriteFlag;    /**<bit1:1 support 4bit page programm, 0 not support. read only*/ 
    __IO  uint32_t     Capacity;        /**<flash capacity*/
    __IO  uint8_t      NoneAreaProtect;
    __IO  uint8_t      HalfAreaProtect;
    __IO  uint8_t      ThreeQuarterAreaProtect; 
    __IO  uint8_t      SevenEighthsAreaProtect;
    __IO  uint8_t      AllAreaProtect; 
//		__IO  uint8_t			GD_version;

}SPI_CODE_FLASH_INFO;

#pragma pack()


void SpiFlash1BitModeInit(uint8_t ChipMode);



/**
 * @brief  init flash
 * @param  NONE
 * @return SPI_CODE_FLASH_ERR_CODE
 *         @arg UNKNOWN_MID_ERR  MID can't indentify
 *         @arg UNKNOWN_DID_ERR  identify MID but can't identify DID
 *         @arg NONE_ERR  
 */
//typedef SPI_CODE_FLASH_ERR_CODE (*flash_init)(SPI_CODE_FLASH_INFO* FlashInfo,BYTE ChipMode);
//SPI_CODE_FLASH_ERR_CODE SpiCodeFlashInit(SPI_CODE_FLASH_INFO* FlashInfo,BYTE ChipMode);


/**
 * @brief  read data from flash 
 * @param  SPI_CODE_FLASH_INFO info for current flash
 * @param  StartAddr the data addr need to read for flash
 * @param  Buffer the data addr need to save for MCU's mem
 * @param  Length data length
 * @param  TimeOut read data but fifo always empty
 * @return SPI_CODE_FLASH_ERR_CODE
 *         @arg  FIFO_ALWAYS_EMPTY_ERR  function execute time out
 *         @arg  DATALEN_LESS_THEN_ZERO_ERR    read data but data len is < 0
 *         @arg  NONE_ERR
 */
//typedef SPI_CODE_FLASH_ERR_CODE (*flash_read)(SPI_CODE_FLASH_INFO* FlashInfo, DWORD StartAddr, BYTE* Buffer, DWORD Length, DWORD TimeOut);
//SPI_CODE_FLASH_ERR_CODE SpiCodeFlashRead(SPI_CODE_FLASH_INFO* FlashInfo, DWORD StartAddr, BYTE* Buffer, DWORD Length, DWORD TimeOut);

/**
 * @brief  write data into flash 
 * @param  SPI_CODE_FLASH_INFO info for current flash
 * @param  Addr the data addr need to write for flash
 * @param  Buffer the data addr need to read for MCU's mem
 * @param  Length data length
 * @param  TimeOut read data but fifo always empty
 * @return SPI_CODE_FLASH_ERR_CODE
 *         @arg  FIFO_ALWAYS_FULL_ERR  function execute time out
 *         @arg  DATALEN_LESS_THEN_ZERO_ERR    read data but data len is < 0
 *         @arg  NONE_ERR
 */
//typedef SPI_CODE_FLASH_ERR_CODE (*flash_write)(SPI_CODE_FLASH_INFO* FlashInfo, DWORD	Addr, BYTE	*Buffer, DWORD 	Length, DWORD TimeOut);	
//SPI_CODE_FLASH_ERR_CODE SpiCodeFlashWrite(SPI_CODE_FLASH_INFO* FlashInfo, DWORD	Addr, BYTE	*Buffer, DWORD 	Length, DWORD TimeOut);					


/**
 * @brief  flash erase
 * @param  EraseType:	see ERASE_TYPE_ENUM type 
 *										CHIP_ERASE:		Erase whole chip
 *										SECTOR_ERASE:	Erase One sector
 *										BLOCK_ERASE:	Erase One block				
 *				  					for CHIP_ERASE type, param 2 & param 3 are NOT cared.
 * @param  Index:
 *										for SECTOR_ERASE type: 	means sector number
 *										for BLOCK_ERASE type: 	means block number
 * @param  IsSuspend: during flash write or sector/block erase need suspend set it to 1
 * @return NONE
 */
//typedef void (*flash_erase)(SPI_FLASH_INFO* FlashInfo,ERASE_TYPE_ENUM erase_type, uint32_t index, bool IsSuspend);
//void SpiCodeFlashErase(SPI_FLASH_INFO* FlashInfo,ERASE_TYPE_ENUM erase_type, uint32_t index, bool IsSuspend);


//typedef SPI_CODE_FLASH_ERR_CODE (*flash_ioctrl)(SPI_CODE_FLASH_INFO* FlashInfo, IOCTL_FLASH_T Cmd, ...);
//SPI_CODE_FLASH_ERR_CODE SpiCodeFlashIOCtrl(SPI_CODE_FLASH_INFO* FlashInfo, IOCTL_FLASH_T Cmd, ...);


void SpiCodeFlashDeepPowerDown(SPI_CODE_FLASH_INFO* FlashInfo);
void SpiCodeFlashReleaseFromDeepPowerDown(SPI_CODE_FLASH_INFO* FlashInfo);
uint32_t SpiCodeFlashReadStatus(SPI_CODE_FLASH_INFO* FlashInfo);
void SpiCodeFlashWriteStatus(SPI_CODE_FLASH_INFO* FlashInfo,uint32_t status);
//SPI_CODE_FLASH_ERR_CODE SpiCodeFlashWriteLock(SPI_CODE_FLASH_INFO* FlashInfo, BYTE ProtectAreaSel);
//SPI_CODE_FLASH_ERR_CODE SpiCodeFlashWriteUnLock(SPI_CODE_FLASH_INFO* FlashInfo, BYTE* Buf, BYTE Len);
int SpiCodeFlashHpmModeSet(SPI_CODE_FLASH_INFO* FlashInfo, uint32_t HpmMode);

#if 0
typedef struct _FLASH_DRIVER_OP_ {
		flash_init	FlashInit;
		flash_read	FlashRead;
		flash_write	FlashWrite;
		flash_erase	FlashErase;
		flash_ioctrl	FlashIOCtrl;
}FLASH_DRIVER_OP;
 #endif


#endif  //__SPI_CODE_FLASH_H__

