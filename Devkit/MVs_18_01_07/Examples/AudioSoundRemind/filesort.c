/////////////////////////////////////////////////////////////////////////
//                  Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc.,ShangHai,China
//                      All rights  reserved
//  Filename:filesort.c
//maintainer lilu
/////////////////////////////////////////////////////////////////////////
#include "app_config.h"
#include "fs_config.h"
#include "folder.h"
#include "fat_file.h"
#include "fsinfo.h"
#include "diskio.h"
#include "presearch.h"
#include "fs_filesort.h"

#if FS_FILE_SORT_EN == 1

//�����ڴ��С�����ƣ���һ���ļ������������ֻ֧�������豸��ǰ2048�������ļ�������
//�����˷�Χ�ĸ����Կ��Բ��ţ������ٰ����ļ������򣬶��ǰ��ո����������
//���֧�ֵ���������������֧��2048�����ͻ�Ҳ���Խ������ֵ��С���Լ�СSRAM��ʹ������
//����һ���ļ����������ʹ��12bit���������֧��2048���ļ�ʱ��ҪSRAM 1.5KB��


//�ڶ�����FS_FILE_SORT_EN���ǰ���£�����ͨ���ñ��������Ƿ�ʹ���ļ��������ܡ�
//�����Ҫ��ʹ��������Ҫ��̬��ֹ�ļ��������ܣ���ֻ��Ҫ���ñ������ó�FALSE
bool gFileSortEnable = TRUE;

#ifdef FS_FILE_SORT_TYPE
FILE_SORT_TYPE gFileSortType = FILE_SORT_BY_CREATE_TIME;
#endif

/////////////�ļ�����������Ҫ�õ��ļ���������//////////////////////////////////////////

//���ڴ������ǰ���ļ�Ŀ¼�ÿ���ļ�ռ��11�ֽ�
#if FS_FILE_SORT_TYPE == FILE_SORT_BY_SHORT_NAME
	typedef uint8_t FILE_NAME_TYPE[9];	//�ļ����������ͣ�������궨���д��
	FILE_NAME_TYPE FileNameTable[FS_MAX_FILE_SORT_NUM]__ATTRIBUTE__(AT(VMEM_ADDR + 2048));

	//���ڴ�������������飬ÿ���ļ�ռ��2�ֽ�
	uint16_t FileSortTableTemp[FS_MAX_FILE_SORT_NUM]__ATTRIBUTE__(AT(VMEM_ADDR + FS_MAX_FILE_SORT_NUM * 9 + 2048));
#endif

#if FS_FILE_SORT_TYPE == FILE_SORT_BY_CREATE_TIME
	typedef struct
	{
		uint16_t FileNumInDisk;
		uint8_t FileCreateTime[5];
	}FILE_CREATE_TIME_NODE;
	FILE_CREATE_TIME_NODE FileCreateTimeNodeBuf[FS_MAX_FILE_SORT_NUM];//__ATTRIBUTE__(AT(VMEM_ADDR + 2048));
	uint32_t CntInFileSortTableTemp;
#endif

//���ڴ��ѹ���������������飬ÿ���ļ�ռ��1.5�ֽ�
//FileSortTable[]�����е�m���ʾ�����ĵ�m��������ԭʼ�ļ���
//�洢�ĸ������£�
//   ---------------------------------------
//  |              FileNum1[11:4]           |  +0
//   ---------------------------------------
//  |   FileNum2[11:8]   |   FileNum1[3:0]  |  +1
//   ---------------------------------------
//  |		       FileNum2[7:0]	        |  +2
//   ---------------------------------------
//  |			   ... ...	                |  +3
//   ---------------------------------------
static uint8_t FileSortTable[(FS_MAX_FILE_SORT_NUM * 3) / 2 + 1];
//////////////////////////////////////////////////////////////////////////////////////////

static void FileSortByCreateTime(uint16_t file_num_in_disk, uint8_t* buf);//len �̶�Ϊ5byte

//Ԥ�����������ҵ�һ���µĸ����ļ��������ļ���������������
//��PreSearch()��������
void FileSortSearchNewFile(FS_CONTEXT* FsContext)
{
	if(gFileSortType == FILE_SORT_BY_SHORT_NAME)
	{
		if(FsContext->gFsInfo.FileSum <= FS_MAX_FILE_SORT_NUM)
		{
				memcpy(FileNameTable[FsContext->gFsInfo.FileSum - 1], &FsContext->gCurrentEntry->FileName[0], 11);
		}
	}
	else if(gFileSortType == FILE_SORT_BY_CREATE_TIME)
	{
//		memcpy(FileNameTable[FsContext->gFsInfo.FileSum - 1], &FsContext->gCurrentEntry->CrtTimeTenth, 5);//���ļ�����������ͬbuffer��������������
		FileSortByCreateTime(FsContext->gFsInfo.FileSum, &FsContext->gCurrentEntry->CrtTimeTenth);
	}
}

//�ȶ������ļ���ɨ��һ�飬���ļ����е������Ӵ�ת�ɿɱȽϴ�С��ѹ��BCD��ʽ��
//��������Ӵ�Ϊ��λ�����������Ӵ���һ���ֽھ��Ǹ�����ֵ0--9
//��������Ӵ����Ǹ�λ�����������Ӵ���һ���ֽ�Ϊ����λ����0x0A��ʾ2λ����0x0B��ʾ3λ����...
//�ӵڶ����ֽڿ�ʼ�洢�����Ӵ�ѹ��BCD��ʽ�����������ֽ����0x00
//��1: 4D 50 33 5F 30 34 XX XX: 4D 50 03 5F 04 00 XX XX
//��2: 4D 50 33 5F 30 30 35 XX: 4D 50 03 5F 05 00 00 XX
//��3: 41 30 42 30 30 43 30 35: 41 00 42 00 00 43 05 00
static void FileSortConvertName(FS_CONTEXT* FsContext)
{
	uint16_t Cnt = (FsContext->gFsInfo.FileSum > FS_MAX_FILE_SORT_NUM) ? FS_MAX_FILE_SORT_NUM : FsContext->gFsInfo.FileSum;
	uint16_t i;
	uint8_t j;
	uint8_t k;

	uint8_t * p;
	uint8_t NumCnt;
	uint8_t ZeroCnt;
	uint8_t Temp;

	for(i = 0; i < Cnt; i++)
	{
		p = FileNameTable[i];

		for(j = 0; j < 8; j++)
		{
			if((p[j] < '0') || (p[j] > '9'))
			{
				continue;	//��ǰλ�÷�����
			}

			//�ҵ�һ�������Ӵ�
			//�����Ӵ���ͷ��'0'����
			for(ZeroCnt = 0; j + ZeroCnt < 8; ZeroCnt++)
			{
				if(p[j + ZeroCnt] != '0')
				{
					break;
				}
				p[j + ZeroCnt] = 0x00;
			}

			//�����Ӵ���Ч���ָ���
			for(NumCnt = 0; j + ZeroCnt + NumCnt < 8; NumCnt++)
			{
				if((p[j + ZeroCnt + NumCnt] < '0') || (p[j + ZeroCnt + NumCnt] > '9'))
				{
					break;
				}
				Temp = p[j + ZeroCnt + NumCnt];
				p[j + ZeroCnt + NumCnt] = 0x00;
				p[j + NumCnt] = Temp - '0';
			}

			//�����2λ�������������������Ӵ�ת����ѹ��BCD��ʽ
			//��������Ǹ�λ�����������Ѿ���Ԥ����ʽ
			if(NumCnt > 1)
			{
				for(k = 0; k < (NumCnt + 1) / 2; k++)
				{
					if(2 * k + 1 == NumCnt)	//�����3λ����5λ����7λ����������ȡ���һλ������ֻȡ1���ֽڣ�����ȡ2���ֽ�
					{
						Temp = (p[j + 2 * k] << 4);
						p[j + 2 * k] = 0;
						p[j + k + 1] = Temp;

					}
					else
					{
						Temp = (p[j + 2 * k] << 4) | p[j + 2 * k + 1];
						p[j + 2 * k] = 0;
						p[j + 2 * k + 1] = 0;
						p[j + k + 1] = Temp;
					}
				}
				p[j] = NumCnt + 8;	//�����Ӵ��ĵ�1���ֽڱ�ʾ�������Ӵ���λ����1λ����00H-09H��2λ����0AH��3λ����0BH��8λ����10H
			}

			j += (NumCnt + ZeroCnt);
		}

	}
}


//���ַ����������㷨������ʱ����(n*logn)������
static void FileSortByName(FS_CONTEXT* FsContext)
{
	uint16_t Cnt = (FsContext->gFsInfo.FileSum > FS_MAX_FILE_SORT_NUM) ? FS_MAX_FILE_SORT_NUM : FsContext->gFsInfo.FileSum;
	uint16_t i;
	uint16_t j;

	int16_t Start;
	int16_t End;
	int16_t Cur;

	for(i = 0; i < Cnt; i++)
	{
		Start = -1;
		End = i;

		//���ַ�����
		while(Start + 1 < End)
		{
			Cur = (Start + End) / 2;
			if(memcmp(FileNameTable[i], FileNameTable[FileSortTableTemp[Cur]], 9) < 0)
			{
				End = Cur;
			}
			else
			{
				Start = Cur;
			}
		}

		//���½ڵ���뵽Endλ�ã�֮������нڵ����
		for(j = i; j > End; j--)
		{
			FileSortTableTemp[j] = FileSortTableTemp[j - 1];
		}

		FileSortTableTemp[End] = i;
	}
}

static void FileShortNameSortSaveResult(FS_CONTEXT* FsContext)
{
	uint16_t Cnt = (FsContext->gFsInfo.FileSum > FS_MAX_FILE_SORT_NUM) ? FS_MAX_FILE_SORT_NUM : FsContext->gFsInfo.FileSum;
	uint16_t i;
	uint16_t Temp;

	//�������б�ѹ����FileSortTable[]������
	for(i = 1; i <= Cnt; i++)
	{
		Temp = ((i - 1) / 2) * 3;

		FileSortTableTemp[i - 1]++;

		if(i % 2)	//����
		{
			FileSortTable[Temp + 0] = (uint8_t)(FileSortTableTemp[i - 1] >> 4);
			FileSortTable[Temp + 1] = (uint8_t)(FileSortTableTemp[i - 1] & 0x0F);
		}
		else		//ż��
		{
			FileSortTable[Temp + 1] |= (uint8_t)((FileSortTableTemp[i - 1] >> 4) & 0xF0);
			FileSortTable[Temp + 2] = (uint8_t)FileSortTableTemp[i - 1];
		}
	}
}

//����ʱ���5��byte����Ϊ [0]������ʱ��10msλ��[1:2]:����ʱ�䣬[3:4]:��������
//��buf1<buf2ʱ������ֵ<0
//��buf1==buf2ʱ������ֵ=0
//��buf1>buf2ʱ������ֵ>0
static int32_t compare_file_create_time(uint8_t* buf1, uint8_t* buf2)
{
	uint8_t time1_10ms = buf1[0], time2_10ms = buf2[0];
	uint16_t time1_time = *(uint16_t *)&buf1[1], time2_time = *(uint16_t *)&buf2[1];
	uint16_t time1_date = *(uint16_t *)&buf1[3], time2_date = *(uint16_t *)&buf2[3];
	
	if(time1_date < time2_date)
		return -1;
	else if(time1_date > time2_date)
		return 1;
	else
	{
		if(time1_time < time2_time)
			return -1;
		else if(time1_time > time2_time)
			return 1;
		else
		{
			if(time1_10ms < time2_10ms)
				return -1;
			else if(time1_10ms > time2_10ms)
				return 1;
			else
				return 0;
		}
	}
}

static void FileSortByCreateTime(uint16_t file_num_in_disk, uint8_t* buf)//len �̶�Ϊ5byte
{
	uint16_t i;
	int32_t Start, End, Mid;
	
	if(file_num_in_disk == 1)
		CntInFileSortTableTemp = 0;
	
	Start = -1;							//���ö��ֲ��ҷ�Χ�ϱ߽�Ϊ����0λ��
	End = CntInFileSortTableTemp;	//���ö��ֲ��ҷ�Χ�±߽�Ϊ��ǰbuf��Ԫ�ظ���

	//���ַ�����
	while(Start + 1< End)
	{
		Mid = (Start + End) / 2;
		if(compare_file_create_time(buf, FileCreateTimeNodeBuf[Mid].FileCreateTime) > 0)
		{
			End = Mid;
		}
		else
		{
			Start = Mid;
		}
	}

	//ȷ��End��Ϊ�½ڵ����λ�ã�End֮������нڵ����
	for(i = CntInFileSortTableTemp; i > End; i--)
	{
		FileCreateTimeNodeBuf[i] = FileCreateTimeNodeBuf[i - 1];
	}

	//���½ڵ���뵽Endλ��
	FileCreateTimeNodeBuf[End].FileNumInDisk = file_num_in_disk;
	memcpy(FileCreateTimeNodeBuf[End].FileCreateTime, buf, 5);
	
	if(CntInFileSortTableTemp < FS_MAX_FILE_SORT_NUM)
		CntInFileSortTableTemp++;
}


static void FileCreateTimeSortSaveResult(FS_CONTEXT* FsContext)
{
	uint16_t Cnt = (FsContext->gFsInfo.FileSum > FS_MAX_FILE_SORT_NUM) ? FS_MAX_FILE_SORT_NUM : FsContext->gFsInfo.FileSum;
	uint16_t i;
	uint16_t Temp;

	//�������б�ѹ����FileSortTable[]������
	for(i = 1; i <= Cnt; i++)
	{
		Temp = ((i - 1) / 2) * 3;

		FileSortTableTemp[i - 1]++;

		if(i % 2)	//����
		{
			FileSortTable[Temp + 0] = (uint8_t)(FileCreateTimeNodeBuf[i - 1].FileNumInDisk >> 4);
			FileSortTable[Temp + 1] = (uint8_t)(FileCreateTimeNodeBuf[i - 1].FileNumInDisk & 0x0F);
		}
		else		//ż��
		{
			FileSortTable[Temp + 1] |= (uint8_t)((FileCreateTimeNodeBuf[i - 1].FileNumInDisk >> 4) & 0xF0);
			FileSortTable[Temp + 2] = (uint8_t)FileCreateTimeNodeBuf[i - 1].FileNumInDisk;
		}
	}
}

//Ԥ��������������ļ�������
//��������ļ�����FileNameTable[]������
//�����������ʱ�����FileSortTableTemp[]������
//Ȼ���������б�ѹ��������FileSortTable[]������
//��PreSearch()��������
void FileSortSearchEnd(FS_CONTEXT* FsContext)
{
	TIMER TestTimer;	//���ڲ���ʱͳ��ִ��ʱ��

//	DBG("FileSortSearchEnd()\n");

	//��������ļ����������������
//	gFsInfo.FileSum = 10;
//	memcpy(FileNameTable[0], "1        M", 9);

	if(gFileSortType == FILE_SORT_BY_SHORT_NAME)
	{
		TimeOutSet(&TestTimer, 0);
		FileSortConvertName(FsContext);
		DBG("time1: %d ms\n", (uint16_t)PastTimeGet(&TestTimer));

		TimeOutSet(&TestTimer, 0);
		FileSortByName(FsContext);
		DBG("time2: %d ms\n", (uint16_t)PastTimeGet(&TestTimer));

		TimeOutSet(&TestTimer, 0);
		FileShortNameSortSaveResult(FsContext);
		DBG("time3: %d ms\n", (uint16_t)PastTimeGet(&TestTimer));
	}
	else if(gFileSortType == FILE_SORT_BY_CREATE_TIME)
	{
		TimeOutSet(&TestTimer, 0);
		FileCreateTimeSortSaveResult(FsContext);
		DBG("time3: %d ms\n", (uint16_t)PastTimeGet(&TestTimer));	
	}
}


//�ú�����FileOpenByNum()����֮ǰ����
//���������ĵ�FileSortNum���ļ�����Ӧ��ԭʼȫ���ļ���
//����ԭʼ�ļ��Ų���[StartFileNum + 1, EndFileNum]��Χ�ڵ��ļ�
//1.����ȫ���������ĵ�m���ļ���ԭʼȫ���ļ��ţ�
//		GetFileNumInDisk(FsContext, m, 0, 0xFFFF)���õ�ȫ��������m���ļ���Ӧ��ԭʼȫ���ļ���
//2.�����ļ����������ĵ�m���ļ���ԭʼȫ���ļ��ţ�
//		GetFileNumInDisk(FsContext, m, Folder->StartFileNum, Folder->StartFileNum + Folder->IncFileCnt)��
uint16_t GetFileNumInDisk(FS_CONTEXT* FsContext, uint16_t FileSortNum, uint16_t StartFileNum, uint16_t EndFileNum)
{
	uint16_t i;
	uint16_t FileNumInDisk;
	uint16_t Index;

	//��ǰΪ��ֹ�ļ���������
	if(!gFileSortEnable)
	{
		return (StartFileNum + FileSortNum);
	}

	//���Ŀ���ļ�������������ļ�������ֱ��ʹ��ԭʼ�ļ�����Ϊ�������ļ���
	if(FileSortNum + StartFileNum > FS_MAX_FILE_SORT_NUM)
	{
		return (StartFileNum + FileSortNum);
	}

	//�������б����������Ƿ�ΪĿ���ļ�
	for(i = 1; i <= FS_MAX_FILE_SORT_NUM; i++)
	{
		//���������ĵ�i���ļ���Ӧ��ԭʼȫ���ļ���
		Index = ((i - 1) / 2) * 3;
		if(i % 2)	//����
		{
			FileNumInDisk = ((FileSortTable[Index] << 4) + (FileSortTable[Index + 1] & 0x0F));
		}
		else		//ż��
		{
			FileNumInDisk = (((FileSortTable[Index + 1] & 0xF0) << 4) + FileSortTable[Index + 2]);		//(FileSortTable[Index + 1] & 0xF0)����4λ������8λ
		}

		//�ж��ļ����Ƿ�����Ŀ�귶Χ��
		if((FileNumInDisk > StartFileNum) && (FileNumInDisk <= EndFileNum))
		{
			FileSortNum--;
			if(FileSortNum == 0)
			{
				//�ҵ�Ŀ���ļ�
				return FileNumInDisk;
			}
		}
	}
}

#endif//FS_FILE_SORT_EN

