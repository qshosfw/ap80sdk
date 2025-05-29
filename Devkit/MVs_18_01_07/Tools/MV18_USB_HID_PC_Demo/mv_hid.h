#ifndef MV_HID_H
#define MV_HID_H



extern "C" {
#include "windows.h"
#include "setupapi.h"
#include "dbt.h"
}


class  MV_HID {
public:
    MV_HID();

    PSP_DEVICE_INTERFACE_DETAIL_DATA    DevDetailData;
    int                                DevDetailDataSz;
    int                                DevInputReportSz;
    int                                DevOutputReportSz;
    int                                DevFeatureReportSz;
    HANDLE                             DevHandle;
    OVERLAPPED                         DevOverlapped;
    BOOL                               DevReadPending;

    void HID_Init();
    void HID_UnInit();
    bool  HID_FindDevices(unsigned int VID,unsigned int PID);
    BOOL HID_Open();
    void HID_Close();
    BOOL HID_Read(BYTE *buf, DWORD length);
    BOOL HID_Write(BYTE *buf,DWORD length);

};

#endif // MV_HID_H
