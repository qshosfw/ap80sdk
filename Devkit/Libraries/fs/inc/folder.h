/**
* @addtogroup �ļ�ϵͳ
* @{
* @defgroup folder folder
* @{
*/

//maintainer lilu
#ifndef __FOLDER_H__
#define __FOLDER_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "fsinfo.h"

	
/**
 * @brief �ļ��нṹ��ָ��
 */
typedef struct	_FOLDER
{
	uint16_t			FolderNum;							/**�ļ��к�*/
	uint16_t			ValidFolderNum;						/**��Ч�ļ��к�*/

	uint32_t	        ParentFoldStartSecNum;	            /**��Ŀ¼��ʼ������*/
	uint32_t			DirSecNum;							/**FDI����������*/
	uint8_t			    DirOffset;							/**FDI�����������е�ƫ����*/

	uint32_t			EndDirSecNum;						/**�ļ��������һ���ļ�Ŀ¼������������*/
	uint8_t			    EndDirOffset;						/**�ļ��������һ���ļ�Ŀ¼�����������е�ƫ����*/

	uint32_t			StartSecNum;
	uint8_t			    ShortName[9];						/**�ļ��ж��ļ���*/

	uint16_t			StartFileNum;						/**�ļ����е�һ���ļ���ȫ�̵ı��*/

	uint16_t			IncFileCnt;							/**��ǰ�ļ����а������ļ�����*/
	uint16_t			IncFolderCnt;						/**��ǰ�ļ����а������ļ��и���*/

	uint16_t			RecFileCnt;							/**��ǰ�ļ����а������ļ������������ļ��У�*/
	uint16_t			RecFolderCnt;                       /**��ǰ�ļ����а������ļ��и����������ļ��У�*/

	FS_CONTEXT*         FsContext;							/**�ļ�ϵͳ�����Ľṹ��ָ��*/

} FOLDER;

/**
 * @brief ���á���ȡ�ļ���ʱ��Ľṹ��
 */
typedef struct
{
	uint16_t Year;	//1980��2107
	uint8_t  Month;	//1��12
	uint8_t  Date;	//1��31
	uint8_t  Hour;	//0��23 
	uint8_t  Min;	//0��59
	uint8_t  Sec;	//0��59

} FOLDER_TIME;

/**
 * @brief ���ļ��кŴ��ļ���
 * @param *Folder 		�ɹ����ļ��к󷵻ص��ļ��нṹ�壬�ռ����ϲ㿪��
 * @param *ParentFolder	ParentFolder == NULL: �������豸�ϵĵ�FolderNum���ļ��С�
 *						ParentFolder != NULL: ��ParentFolder�ļ����еĵ�FolderNum���ļ��С�
 * @param FolderNum		�ļ��к�
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 */
bool FolderOpenByNum(FOLDER* Folder, FOLDER* ParentFolder, uint16_t FolderNum);

/**
 * @brief ����Ч�ļ��кţ��˳����ļ��к����ţ����ļ���
 * @param *Folder 		�ɹ����ļ��к󷵻ص��ļ��нṹ�壬�ռ����ϲ㿪��
 * @param *ParentFolder	ParentFolder == NULL: �������豸�ϵĵ�FolderNum���ļ��С�
 *						ParentFolder != NULL: ��ParentFolder�ļ����еĵ�ValidFolderNum���ļ��С�
 * @param FolderNum		��Ч�ļ��к�
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 */
bool FolderOpenByValidNum(FOLDER* Folder, FOLDER* ParentFolder, uint16_t ValidFolderNum);

/**
 * @brief ���ļ������ƴ��ļ���
 * @param *Folder 		�ɹ����ļ��к󷵻ص��ļ��нṹ�壬�ռ����ϲ㿪��
 * @param *ParentFolder	ParentFolder == NULL: �������豸�ϲ���FolderName�ļ��С�
 *						ParentFolder != NULL: ��ParentFolder�ļ��в���FolderName�ļ��С�
 * @param *FolderName	�ļ��ж��ļ���������С�ڵ���8�ֽ�
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 * @note ��Ŀ¼����Ϊ"\\"
 */
bool FolderOpenByName(FOLDER* Folder, FOLDER* ParentFolder, uint8_t* FolderName);

/**
 * @brief �����ļ��������ļ���
 * @param *Folder 		�ɹ����ļ��к󷵻ص��ļ��нṹ�壬�ռ����ϲ㿪��
 * @param *ParentFolder	ParentFolder == NULL: �������豸�ϲ���FolderName�ļ��С�
 *						ParentFolder != NULL: ��ParentFolder�ļ��в���FolderName�ļ��С�
 * @param *FolderName	�ļ��г��ļ�����Unicode 16����
 * @param Len			�ļ���������
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 */
bool FolderOpenByLfName(FOLDER* Folder, FOLDER* ParentFolder, uint8_t* FolderName, uint8_t Len);

/**
 * @breaf ��ȡ�ļ��еĳ��ļ���
 * @param *Folder			�ļ���ָ��
 * @param *LongFolderName	���صĳ��ļ����������ļ����Ϊ66���ֽڣ�����LongFolderName��������Ҫ��66���ֽ�
 * @param GetMaxLength		����ļ���������
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 * @note ���ڳ��ļ�����������:
 *			1,�ļ������ȳ���8byte
 *			2,�ļ����г��ֺ���
 *			3,�ļ����г���'.'
 */
bool FolderGetLongName(FOLDER* Folder, uint8_t* LongFolderName, uint8_t GetMaxLength);	//LongFileName[]: 66 Bytes

/**
 * @breaf �ļ��д���
 * @param *Folder		�ɹ������ļ��к󷵻ص��ļ��нṹ�壬�ռ����ϲ㿪��
 * @param *ParentFolder	��ParentFolder�ļ����д����ļ�������ΪNULL
 * @param *FolderName	�ļ��ж��ļ���
 */
bool FolderCreate(FOLDER* Folder, FOLDER* ParentFolder, uint8_t* FolderName);

/**
 * @breaf ���ļ����ļ��д���
 * @param *Folder			�ɹ������ļ��к󷵻ص��ļ��нṹ�壬�ռ����ϲ㿪��
 * @param *ParentFolder		��ParentFolder�ļ����д����ļ�������ΪNULL
 * @param *FolderLongName	�ļ��г��ļ��� FolderLongName[64]Ϊ���ļ�����,Unicode 16���룬����Ϊ128Byte ����λ��Ϊ0xFF
 * @param *FolderShortName	�ļ��ж��ļ��� FolderShortName[11]Ϊ���ļ�����,Utf-8���룬����Ϊ11Byte ����λ��Ϊ�ո�"0x20"�������λ��Ϊ0x20
 * @param Len				���ļ�����ʵ�ʳ��ȣ���λΪuint16_t
 */
bool FolderCreateByLongName(FOLDER* Folder, FOLDER* ParentFolder, uint16_t* FolderLongName, uint8_t* FolderShortName, uint8_t Len);

/**
 * @brief �޸��ļ��Ĵ���ʱ�䡢�޸�ʱ�䡢����ʱ��
 * @param *Folder		�ļ���ָ��
 * @param *CreateTime	�ļ�����ʱ��ṹ��ָ�룬����Ϊ1��
 * @param *ModifyTime	�ļ��޸�ʱ��ṹ��ָ�룬����Ϊ2��
 * @param *AccessTime	�ļ�����ʱ��ṹ��ָ�룬ֻ�����ڣ�������ʱ�䲿��
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 * @note ���ĳ��ʱ����Ϣ�����ָ��ΪNULL�����ʾ���ı��ʱ����Ϣ
 */
bool FolderSetTime(FOLDER* Folder, FOLDER_TIME* CreateTime, FOLDER_TIME* ModifyTime, FOLDER_TIME* AccessTime);
#ifdef  __cplusplus
}
#endif//__cplusplus

#endif

/**
 * @}
 * @}
 */
