//spp proc interface implementation example file, one may implement it by following the defined protocol.
//spp_app_proc.c
//zhouyi,2012.10.30

#include "type.h"
#include "spp_app_proc.h"
#include "bt_control_api.h"

uint32_t UserProcSppData(void);

//
//Deal with the recived cmd
//

//User APIs
//if fetched data from received buffer, then MUST return non-zero.
uint32_t UserProcSppData(void)
{
    uint16_t Size = SppGetRecBufDataSize();  //get received buff data size
    
    if(0 == Size)
    {
        return 0;
    }

    return 1;
}
