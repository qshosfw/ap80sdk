#include "stdio.h"
#include "mv_hid.h"

void main(int argc, char *argv[])
{
	printf("test\n");
	MV_HID MyHID;
	unsigned int i;
	MyHID.HID_Init();
	
	int Vid = atoi(argv[1]);
	int Pid = atoi(argv[2]);


	printf("vid:0x%04X pid:0x%04X\n",Vid,Pid);

	if(MyHID.HID_FindDevices(Vid,Pid))
	{
		printf("find and open ok\n");
	}
	unsigned char WritetBuffer[64];
	unsigned char ReadBuffer[64];
	printf("\nwrite data:\n");
	for(i=0;i<64;i++)
	{
		WritetBuffer[i] = i;
		printf("%02X ",WritetBuffer[i]);
		if((i%16) == 15)
		{
			printf("\n");
		}
	}
	if(MyHID.HID_Write(WritetBuffer,64))
	{
		printf("write cmd ok ! !\n\n");
	}

	Sleep(100);
	memset(ReadBuffer,0,64);
	if(MyHID.HID_Read(ReadBuffer,64))
	{
		printf("read cmd ok ! !\n");
		printf("read data:");
		for(i=0;i<64;i++)
		{
			if((i%16) == 0)
			{
				printf("\n");
			}
			printf("%02X ",ReadBuffer[i]);
		}
	}
	MyHID.HID_Close();
	MyHID.HID_UnInit();
	getchar();
}

