/*
bt_play_internal_api.c
*/
#include <stdio.h>
#include "type.h"
//#include "debug.h"
#include "pcm_sync.h"
#include "bt_play_internal_api.h"
#include "bt_device_type.h"
#include "bt_stack_api.h"
#include "bt_control_api.h" 
#include "app_config.h"

void BTStackDataStatusCallBackFunc(uint8_t CBParameter, uint8_t* HistoryStatus/*This value will be AUTOMATICALLY reset to zero when new BT device connected*/)
{
    switch(CBParameter)
    {
        case BT_DS_A2DP_STREAM_START:
            DBG("BT_DS_A2DP_STREAM_START\r\n");
            //tbd...
            break;

        case BT_DS_A2DP_STREAM_SUSPEND:
            DBG("BT_DS_A2DP_STREAM_SUSPEND\r\n");
            break;
		
        case BT_DS_SCO_STREAM_START:
            //tbd...
            PcmSyncInit(MV_MCU_AS_PCMSYNC_CLK_SLAVE);
            DBG("BT_DS_SCO_STREAM_START\r\n");
            break;

        case BT_DS_CALL_SCO_STREAM_PAUSE:
            //tbd...
            DBG("BT_DS_CALL_SCO_STREAM_PAUSE\r\n");
            PcmSyncUnInit();
            break;

        case BT_DS_CALL_START:
            //tbd...
            DBG("BT_DS_CALL_START\r\n");
            break;

        case BT_DS_CALL_END:
            //tbd...
            //pls check "IsISOHistoryFlag"
            DBG("BT_DS_CALL_END\r\n");
            break;

        case BT_DS_CALL_AEC_SUPPORTED:
            DBG("BT_DS_AEC_SUPPORTED\r\n");
            break;
		
        case BT_DS_SNIFF_MODE:
            DBG("BT_DS_SNIFF_MODE\r\n");
            break;

        case BT_DS_ACTIVE_MODE:
            DBG("BT_DS_ACTIVE_MODE\r\n");
            break;

        default:
            break;
    }
}

