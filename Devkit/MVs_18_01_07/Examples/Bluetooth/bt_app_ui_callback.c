///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: bt_app_ui_callback.c
//
//BT application UI callback functions. This function is called by the BT stack.
//One can modify/implement this function but MUST NOT change the interface.
//
//用户必需参考下面的实现方式按需实现本函数，蓝牙协议栈内会调用此函数。
//
#include <stdio.h>
#include "type.h"
//#include "debug.h"
#include "pcm_sync.h"
#include "bt_app_ui_callback.h"
#include "bt_control_api.h"
#include "app_config.h"

static bool PowerOnBtConnectFlag = TRUE;//是否开启开机自动回连
extern bool FlshGetAliveAddr(uint8_t* Addr);

void BTStackStatusCallBackFunc(uint8_t CBParameter)
{
    switch(CBParameter)
    {
        case BT_UI_CB_STATUS_IDLE:
			if(PowerOnBtConnectFlag)	
			{
				uint8_t BdAddr[6];
				if(FlshGetAliveAddr(BdAddr))
				{
					DBG("Last Dev:%x:%x:%x:%x:%x:%x\n", BdAddr[0], BdAddr[1], BdAddr[2], BdAddr[3], BdAddr[4], BdAddr[5]);
					BTConnect(BdAddr);
				}
				else
				{
					DBG("Last Dev: Not found!\n");
				}

                PowerOnBtConnectFlag = FALSE; //仅尝试一次
			}          
            DBG("&&&&&&&&& WaitForPairing/Connection &&&&&&&&&&&\r\n");
            break;
            
        case BT_UI_CB_STATUS_PAIRED:
            DBG("&&&&&&&&& BT device paired &&&&&&&&&&&\r\n");
            break;
        
        case BT_UI_CB_STATUS_A2DP_CONNECTED:
            DBG("-----------------------A2DP Connected ###########\r\n");
            break;
            
        case BT_UI_CB_STATUS_PLAY_CONTROL_CONNECTED:
            DBG("BT_UI_CB_STATUS_PLAY_CONTROL_CONNECTED\r\n");
            break;

        case BT_UI_CB_STATUS_DISCONNECTED:
            DBG("------------------------A2DP DisConnected $$$$$$$$$$$\r\n");

            break;
        
        case BT_UI_CB_STATUS_ERROR:
            DBG("BT_UI_CB_STATUS_ERROR\n");
            break;
        
////HF////////////////////////////////////////////////////////////////
        case BT_UI_CB_STATUS_HF_DISCONNECTED:
            DBG("------------------------HF DisConnected $$$$$$$$$$$\r\n");
            PcmSyncUnInit();
            break;

        case BT_UI_CB_STATUS_HF_CONNECTED:
            DBG("--------------------------------HF connected $$$$\n");
            break;

        case BT_UI_CB_STATUS_HF_INCOME_RING:
            break;

        case BT_UI_CB_STATUS_HF_CALL_SETUP_NONE:
            DBG("&&&&&&&&& BT_UI_CB_STATUS_HF_CALL_SETUP_END &&&&&&&&&\r\n");
            break;

        case BT_UI_CB_STATUS_HF_CALL_SETUP_INCOMING:
            DBG("BT_UI_CB_STATUS_HF_CALL_SETUP_INCOMING. \n");
            break; 
            
        case BT_UI_CB_STATUS_HF_CALL_SETUP_OUTCALL_ALERT:
            DBG("BT_UI_CB_STATUS_HF_CALL_SETUP_OUTCALL_ALERT \n");
            break;

        case BT_UI_CB_STATUS_HF_INCOME_CALL_NUM:
            DBG("BT_UI_CB_STATUS_HF_INCOME_CALL_NUM\r\n");
            break;

        case BT_UI_CB_STATUS_HF_CALL_CONNECTED: 
            DBG("BT_UI_CB_STATUS_HF_CALL_CONNECTED\r\n");                
            if((PcmSyncAecGetStatus() == PCMSYNC_AEC_NONE) || (PcmSyncAecGetStatus() == PCMSYNC_AEC_MV))
            {
                BTCallDisableAEC();
            }

            DBG(":: Call connected... \n");          
            break;

        case BT_UI_CB_STATUS_HF_CALL_DISCONNECTED:

            DBG(":: Call disconnected... \n");
            break;
            
        case BT_UI_CB_STATUS_GET_REMOTE_BT_DEV_NAME:
            DBG(":: connected : %s \r\n", BTGetRemoteDevName());
            break;

////////SPP//////////////////////////////////////////////////////////////
        case BT_UI_CB_STATUS_SPP_CONNECTED:
            DBG("------------------------SPP connected  ::::########\r\n");
            break;

        case BT_UI_CB_STATUS_SPP_DISCONNECTED: 
            DBG("------------------------SPP DisConnected $$$$$$$$$$$\r\n");
            break;
            
        case BT_UI_CB_STATUS_INQUIRY_RESULT:
            //for i/o connection test function ONLY

            DBG("########:::: BT_UI_CB_STATUS_INQUIRY_RESULT  ::::########\r\n");
            break;
        
        case BT_UI_CB_STATUS_CONNECTION_LOST: 
            DBG("BB lost\n");            
            break;

//////////////////////////////////////////////////////////////////////
        default:
            //DBG("unsupported code = 0x%02x\r\n",CBParameter));
            break;
        
    }
}




