/*
*****************************************************************************
*					Mountain View Silicon Tech. Inc.
*	Copyright 2012, Mountain View Silicon Tech. Inc., ShangHai, China
*					All rights reserved.
*
* Filename:			msic.h
* Description:		define global MACRO in here
*
******************************************************************************
*/

#ifndef __MISCFG_H__
#define __MISCFG_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#define		MVsO18A_MPW_R1  	1
#define		MVsO18A_MASK_ROM  	2
#define		MVsO18B_FPGA_R0  	3
#define		MVsO18B_MASK_R0  	4

#define 	ASIC_VERSION		MVsO18B_MASK_R0
//#define		FPGA_TEST_ONLY

#define		DEVPROG_MEMTYPE		(1)//0 for OTP only,1 for FLASH,2 for SDRAM,3 for OTP+ROM,4 for FLASH only

////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> bootloader configuration>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define SYSCLK_RC48M2DPLL		(0)//1 for RC48MHz switch to DPLL and backforth,0 for otherwise
#define OTPCODE_VERSION			(4)//otp major version
#define OTPCODE_VERSION_MINOR	(2)//otp minor version
#define OTPCODE_VERSION_PATCH	(0)//otp patch version
#define	SW_DOWNLOAD_EN			(1)//0 OTP burner,1 for SW downloader
#define	UPGRADE_DEFAULT_DEV		(2)//((2 << 4) | 2) //PHY2 + SD1 by default
#define UPGRADE_NVM_ADDR		(176)//boot upgrade information at NVRAM address

#define UPGRADE_ERRNO_NOERR		(-1) //just initialization after boot up
#define UPGRADE_ERRNO_ENOENT	(-2) //no such file open by number
#define UPGRADE_ERRNO_EIO		(-5) //read/write error
#define UPGRADE_ERRNO_E2BIG		(-7) //too big than flash capacity
#define UPGRADE_ERRNO_EBADF		(-9) //no need to upgrade
#define UPGRADE_ERRNO_EFAULT	(-14) //address fault
#define UPGRADE_ERRNO_EBUSY		(-16) //flash lock fail
#define UPGRADE_ERRNO_ENODEV	(-19)//no upgrade device found
#define UPGRADE_ERRNO_ENODATA	(-61)//flash is empty
#define UPGRADE_ERRNO_EPROTOTYPE (-91) //bad head format("MVO\x12"
#define UPGRADE_ERRNO_ELIBBAD	(-80) //CRC error
#define UPGRADE_ERRNO_USBDEV	(-81) //no upgrade USB device
#define UPGRADE_ERRNO_SDDEV		(-82) //no upgrade SD device
#define UPGRADE_ERRNO_USBFILE	(-83) //no upgrade file found in USB device
#define UPGRADE_ERRNO_SDFILE	(-84) //no upgrade file found in SD device
#define UPGRADE_ERRNO_NOBALL	(-85) //no upgrade ball in USB & SD device
#define UPGRADE_SUCC_MAGIC		(0x57F9B3C8) //just a successful magic
#define UPGRADE_REQT_MAGIC		(0x9ab4d18e) //just a request magic


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> downloader configuration>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define DOWNLOADER_VERSION			(1)//downloader version
#define DOWNLOADER_VERSION_PATCH	(3)//downloader patch version
#define DWNLDR_ENCDECRYPT_EN		(0) //0 for plain text,1 for encryption text 
#define DWNLDR_ENCDECRYPT_KEY		(0) //32 bits encryption key(0 for plain text)
#define DWNLDR_IMGSIZE_ADDR			(0x80)
#define DWNLDR_PID2CRC_ADDR			(0x84)
#define DWNLDR_PID2CRC_NAME		("B5420BXX")
#define DWNLDR_CONSTDAT_ADDR		(0x8C)
#define DWNLDR_CONSTDAT_OFFSET	(0x080000)	//1MB offset from flash start address 0
#define DWNLDR_USRDAT_ADDR			(0x90)
#define DWNLDR_USRDAT_OFFSET	(0x100000)	//2MB offset from flash start address 0
#define DWNLDR_SDKVER_ADDR			(0x94)
#define DWNLDR_CODECRC_ADDR			(0x98)
#define DWNLDR_CODEMAGIC_ADDR		(0x9C)
#define DWNLDR_APPCTL_ADDR			(0x7F72)
#define DWNLDR_APPCTL_REMAP_MASK	(0xF)
#define DWNLDR_APPCTL_UPGRDEV_MASK	(0x3F << 4)
//#define	FLASH_CONSTDAT_TEST			//constant data test only

//the following setting is set the downloader name in the SW debug description table selection.
#if		(0 == DEVPROG_MEMTYPE)
#define DWNLDR_DESCRIP_NAME			("O18B.OTP_ONLY.1KB+31KB")
#elif	(1 == DEVPROG_MEMTYPE)
#define DWNLDR_DESCRIP_NAME			("MV18X_16MB_V1.4.1.FLM")
#elif	(2 == DEVPROG_MEMTYPE)
#define DWNLDR_DESCRIP_NAME			("O18B.OTP+SDRAM.V1.2")
#elif	(3 == DEVPROG_MEMTYPE)
#define DWNLDR_DESCRIP_NAME			("O18B.OTP+ROM.224KB.FPGA")
#elif	(4 == DEVPROG_MEMTYPE)
#define DWNLDR_DESCRIP_NAME			("O18B.FLASH_ONLY.MAX8MB")
#elif	(5 == DEVPROG_MEMTYPE)
#define DWNLDR_DESCRIP_NAME			("O18B.SDRAM_TEST.MAX8MB")
#endif //(0 == DEVPROG_MEMTYPE)

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //_MISCFG_H_

