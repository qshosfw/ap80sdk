
/**
* @addtogroup �ļ�ϵͳ
* @{
* @defgroup fat_file fat_file
* @{
*/

//maintainer lilu
#ifndef __FAT_FILE_H__
#define __FAT_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "fs_config.h"
#include "folder.h"
#include "fsinfo.h"

#define	FA_READ				0x01
#define	FA_WRITE			0x02
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08

#define	SEEK_FILE_SET				0		/**FileSeek�����Ļ�׼λ�ã��ļ���ʼλ��*/
#define	SEEK_FILE_CUR				1		/**FileSeek�����Ļ�׼λ�ã��ļ���ǰλ��*/
#define	SEEK_FILE_END				2		/**FileSeek�����Ļ�׼λ�ã��ļ�ĩβλ��*/

typedef enum _FILE_TYPE
{
    FILE_TYPE_MP3,
    FILE_TYPE_WMA,
    FILE_TYPE_WAV,
    FILE_TYPE_SBC,
    FILE_TYPE_FLAC,
    FILE_TYPE_AAC,
    FILE_TYPE_AIF,
    FILE_TYPE_AMR,
    FILE_TYPE_UNKNOWN = 0xFF

} FILE_TYPE;



/**
 * @Brief	�ļ��ṹ��
 */
typedef struct _FAT_FILE
{
	uint16_t	FileNumInDisk;			/**�������豸�ϵ��ļ���*/
	uint16_t	FileNumInFolder;		/**�ڵ�ǰ�ļ����е��ļ���*/
	uint16_t	FolderNum;				/**�����ļ��еı��*/

	uint32_t	ParentFoldStartSecNum;	/**��Ŀ¼��ʼ������*/
	uint32_t 	DirSecNum;				/**�ļ�Ŀ¼������������*/
	uint32_t 	DirOffset;				/**FDI ���������е�ƫ����*/

	uint8_t		ShortName[11];			/**���ļ���*/
	uint8_t	    FileType;				/**�ļ�������*/
	uint32_t	Size;					/**�ļ������ֽ���*/
                                        
	uint32_t 	FptrIndex;              
	uint32_t 	FptrCluster;            
	uint32_t	StartClusNum;			/**�ļ���һ���غ�*/
	uint32_t	SecterOffset;			/** Sector offset in the cluster*/
                                        
	uint32_t    Fptr;					/**��ǰλ�����ļ���ƫ���ֽ���*/
	uint8_t     Flag;				    /**����ļ�״̬*/

	FS_CONTEXT* FsContext;				/**�ļ�ϵͳ������ָ��*/
} FAT_FILE;


/**
 * @Brief �ļ�ʱ��Ľṹ��
 */
typedef struct
{
	uint16_t Year;	//1980��2107
	uint8_t  Month;	//1��12
	uint8_t  Date;	//1��31
	uint8_t  Hour;	//0��23 
	uint8_t  Min;	//0��59
	uint8_t  Sec;	//0��59

} FILE_TIME;


/**
 * @brief ���ļ�
 * @param *File		�ɹ����ļ��󷵻ص��ļ��ṹ�壬�ռ����ϲ㿪��
 * @param file_name	����·�����ļ���   ��: \\mp3\\short.mp3
 * @param mode		��ģʽ FA_READ��FA_WRITE��FA_CREATE_NEW��FA_CREATE_ALWAYS
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 * @note ֧�ֳ��ļ���,����֧������
 */
bool FileOpen(FAT_FILE* File, const uint8_t* FileName, const uint8_t Mode);

/**
 * @brief ��ȡ�ļ�����
 * @param *FsContext	�ļ�ϵͳ������ָ��
 * @return �ļ����ͣ��μ�enum _FILE_TYPE
 */
uint8_t FileGetType(FS_CONTEXT* FsContext);

/**
 * @brief ��ѯ�ļ��ڲ�ָ���Ƿ񵽴��ļ�ĩβ
 * @param *File	�ļ��ṹ��ָ��
 * @return �ǣ�TRUE ��FALSE ��ע���˴�����ֵ����ӦΪbool����int������API�����ʷ����Ϊ������ǰ������δ��������
 */
int32_t FileEOF(FAT_FILE* File);

/**
 * @brief ���ļ��ڲ�ָ������ָ���ļ�ͷ���ָ����ļ��մ�ʱ��״̬
 * @param *File	�ļ��ṹ��ָ��
 * @return ��
 */
void FileRewind(FAT_FILE* File);

/**
 * @brief ���ļ��Ŵ��ļ�
 * @param *File		�ɹ����ļ��󷵻ص��ļ��ṹ�壬�ռ����ϲ㿪��
 * @param *Folder	Folder == NULL: �������豸�ϵĵ�FileNum���ļ���
 *					Folder != NULL: ��Folder�ļ����еĵ�FileNum���ļ��С�
 * @param FileNum	�ļ���
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 */
bool FileOpenByNum(FAT_FILE* File, FOLDER* Folder, uint16_t FileNum);

/**
 * @brief ���ļ������ļ�
 * @param *File		�ɹ����ļ��󷵻ص��ļ��ṹ�壬�ռ����ϲ㿪��
 * @param *Folder	��Folder�ļ������ļ���Folder����ΪNULL
 * @param *FileName	�ļ���
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 */
bool FileOpenByName(FAT_FILE* File, FOLDER* Folder, uint8_t* FileName);

/**
 * @brief �����ļ������ļ�
 * @param *File		�ɹ����ļ��󷵻ص��ļ��ṹ�壬�ռ����ϲ㿪��
 * @param *Folder	��Folder�ļ������ļ���Folder����ΪNULL
 * @param *FileName	�ļ�������ΪΪUnicode 16����
 * @param Len		�ļ�������
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 */
bool FileOpenByLfName(FAT_FILE* File, FOLDER* Folder, uint8_t* FileName, uint8_t Len);

/**
 * @brief �ļ��ڲ�ָ���ƶ�����
 * @param *File		�ļ�ָ��
 * @param Offset	�ƶ��ֽ���
 * @param Base		�ƶ���׼�㣬SEEK_FILE_SET SEEK_FILE_CUR SEEK_FILE_END
 * @return �ɹ���0 ʧ�ܣ���0���μ�fat_errno.h
 */
//�˴�����ֵʹ�õ�Ϊfat_errno.h����Ĵ����룬�´θĶ�Ӧ��������ͳһΪö�ٱ�����ͳһ��ǰ�ļ��ķ���ֵ����
int32_t FileSeek(FAT_FILE* File, int32_t Offset, uint8_t Base);

/**
 * @brief �ļ���ȡ
 * @param *buffer	��ȡ���ݻ�����
 * @param size		ÿ����λ�ֽ���
 * @param count		��λ������size * count ��Ϊ��ȡ�����ֽ�����
 * @param *File		�ļ�ָ��
 * @return ʵ�ʶ�ȡ���ֽ���
 */
uint32_t FileRead(void* buffer, uint32_t size, uint32_t count, FAT_FILE* File);

/**
 * @brief �ļ�д��
 * @param *buffer	д�����ݻ�����
 * @param size		ÿ����λ�ֽ���
 * @param count		��λ������size * count ��Ϊд������ֽ�����
 * @param File		�ļ�ָ��
 */
uint32_t FileWrite(const void* buffer, uint32_t size, uint32_t count, FAT_FILE* File);

/**
 * @brief ��ȡ�ļ���С
 * @param *File		�ļ�ָ��
 * @return ���ص�ǰ�ļ����ֽ���
 */
int32_t FileSof(FAT_FILE* File);

/**
 * @brief ��ȡ�ļ��ڲ�ָ���λ��
 * @param *File		�ļ�ָ��
 * @return �ļ��ڲ�ָ��λ��
 */
//�˴��ļ�����ӦΪuint32_t
int32_t FileTell(FAT_FILE* File);

/**
 * @brief �ļ����棬���ļ�ϵͳbuffer�е�DIR��Ϣ��FAT��Ϣ���̡�
 * @param *File	�ļ�ָ��
 * @return �ɹ���0 ʧ�ܣ���0���μ�fat_errno.h
 * @note ��FileClose������Ϊ�����File�ṹ�����ݣ���¼���лᶨʱ����FileSave����������̺�¼���ļ���СΪ0
 */
int32_t FileSave(FAT_FILE* File);

/**
 * @brief �ļ��رգ����ļ�ϵͳbuffer�е�DIR��Ϣ��FAT��Ϣ���̣����File�ṹ�����ݡ�
 * @param *File	�ļ�ָ��
 * @return �ɹ���0 ʧ�ܣ���0���μ�fat_errno.h
 */
int32_t FileClose(FAT_FILE* File);

/**
 * @brief ��ȡ�ļ����ļ���
 * @param *File			�ļ�ָ��
 * @param *LongFileName	���ڷ��س��ļ�����buffer
 * @param GetMaxLength	��󳤶�
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 * @note ���ļ������ڵ�������
 *			1,�ļ������ȳ���8byte 
 *			2,��չ�����ȳ���3byte
 *			3,�ļ�������չ���г��ֺ���
 *			4,�ļ����д���һ�����ϵ�'.'
 *			5,�ļ����޺�׺
 */
bool FileGetLongName(FAT_FILE* File, uint8_t* LongFileName, uint8_t GetMaxLength);

/**
 * @brief �ļ���գ����ļ���С���㣬�ͷ�����Ĵ���
 * @param *File �ļ�ָ��
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 */
bool FileFlush(FAT_FILE* File);

/**
 * @brief �ļ�ɾ��
 * @param *File		�ļ�ָ��
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 */
bool FileDelete(FAT_FILE* File);

/**
 * @brief �ļ�����
 * @param *File		�ɹ������ļ��󷵻ص��ļ��ṹ�壬�ռ����ϲ㿪��
 * @param *Folder	��Folder�ļ����д����ļ���Folder����ΪNULL
 * @param *FileName	���ļ��������磺"123.TXT", "ABC123.MP3"
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 * @note ���ļ������Ȳ��ܳ���8+3�ֽڣ����ļ����в���ͬʱ���ִ�Сд��ĸ��
 */
bool FileCreate(FAT_FILE* File, FOLDER* Folder, uint8_t* FileName);

/**
 * @brief �������ļ������ļ�
 * @param *File			�ɹ������ļ��󷵻ص��ļ��ṹ�壬�ռ����ϲ㿪��
 * @param *Folder		��Folder�ļ����д����ļ���Folder����ΪNULL
 * @param *LongFileName	���ļ�����Unicode 16���룬LongFileName����Ϊ128�ֽ� �ֽڿ���λ�ñ�����Ϊ0xFF
 * @param Len			���ļ�������
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 */
bool FileCreateByLfName(FAT_FILE* File, FOLDER* Folder, uint8_t* LongFileName, uint8_t Len);

/**
 * @brief �ļ�ϵͳ��ʽ��
 * @param ��
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 * @note ��ͬ�ڱ�׼�ĸ�ʽ���������ò������ı�����ĸ�ʽ��ֻ�ǽ������������ļ�ɾ�����ͷ����дأ��ò�����ʱ�ϳ�����ͬ���豸�Ϻ�ʱ���ܼ��뵽��ʮ��
 */
bool FSFormat(void);

/**
 * @brief �޸��ļ��Ĵ���ʱ�䡢�޸�ʱ�䡢����ʱ��
 * @param *File			�ļ�ָ��
 * @param *CreateTime	�ļ�����ʱ��ṹ��ָ�룬����Ϊ1��
 * @param *ModifyTime	�ļ��޸�ʱ��ṹ��ָ�룬����Ϊ2��
 * @param *AccessTime	�ļ�����ʱ��ṹ��ָ�룬ֻ�����ڣ�������ʱ�䲿��
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 * @note ���ĳ��ʱ����Ϣ�����ָ��ΪNULL�����ʾ���ı��ʱ����Ϣ
 */
bool FileSetTime(FAT_FILE* File, FILE_TIME* CreateTime, FILE_TIME* ModifyTime, FILE_TIME* AccessTime);

/**
 * @brief ��ȡ�ļ��Ĵ���ʱ�䡢�޸�ʱ�䡢����ʱ��
 * @param *File			�ļ�ָ��
 * @param *CreateTime	�ļ�����ʱ��ṹ��ָ�룬����Ϊ1��
 * @param *ModifyTime	�ļ��޸�ʱ��ṹ��ָ�룬����Ϊ2��
 * @param *AccessTime	�ļ�����ʱ��ṹ��ָ�룬ֻ�����ڣ�������ʱ�䲿��
 * @return �ɹ���TRUE ʧ�ܣ�FALSE
 * @note ���ĳ��ʱ����Ϣ�����ָ��ΪNULL�����ʾ����ȡ��ʱ����Ϣ
 */
bool FileGetTime(FAT_FILE* File, FILE_TIME* CreateTime, FILE_TIME* ModifyTime, FILE_TIME* AccessTime);

void SetFsTerminateFuc(TerminateFunc func);

extern TerminateFunc terminate_cur_fs_proc;

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //_FAT_FILE_H_

/**
 * @}
 * @}
 */
