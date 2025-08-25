/////////////////////////////////////////////////////////////////////////
//                  Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc.,ShangHai,China
//                      All rights  reserved
//  Filename:fs_config.h
//maintainer lilu
/////////////////////////////////////////////////////////////////////////

/**
* @addtogroup �ļ�ϵͳ
* @{
* @defgroup fs_config fs_config
* @{
*/

#ifndef __FS_CONFIG_H__
#define __FS_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include	"type.h"

typedef enum
{
	FILE_SORT_BY_SHORT_NAME = 0,
	FILE_SORT_BY_CREATE_TIME
}FILE_SORT_TYPE;
///////////////////////////////////////����Ϊֻ�����������ò���
#define FAT_USE_LFN		1       //read only

#define FAT_NAME_MAX	128	    //read only

#define MAX_FS_CONTEXT_COUNT 1  //֧��ͬʱmount�ķ�����  read only
///////////////////////////////////////ֻ�����������ò���


///////////////////////////////////////����Ϊ�����ò���

#define FS_FILE_SORT_EN    0          //�Ƿ�֧������ 
#define FS_FILE_SORT_TYPE	FILE_SORT_BY_CREATE_TIME
#define FS_MAX_FILE_SORT_NUM	1024  //֧�ֵ������ļ������������� 2048   

///////////////////////////////////////�����ò���

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //_FS_CONFIG_H_

/**
 * @}
 * @}
 */
