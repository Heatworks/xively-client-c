/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_time.h>
//#include <time.h>
#include "tcpip/tcpip.h"
#include "Model3.h"

void xi_bsp_time_init()
{
}

//xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
//{
//    return ( xi_time_t )TickConvertToMilliseconds( TickGet() );
//}


xi_time_t xi_bsp_time_getcurrenttime_seconds(void)
{
    unsigned int utcSeconds;
    unsigned int sntpMs;
    TCPIP_SNTP_RESULT result;
    result = TCPIP_SNTP_TimeGet(&utcSeconds, &sntpMs);
    
    if(result == SNTP_RES_OK){
        return (xi_time_t)utcSeconds;
    }
    else{
        return -1;
    }
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds(void)
{
    unsigned int utcSeconds;
    unsigned int sntpMs;
    TCPIP_SNTP_RESULT result;
    result = TCPIP_SNTP_TimeGet(&utcSeconds, &sntpMs);
    SYS_CONSOLE_PRINT("Time result:%ld\r\n", sntpMs);
    if(result == SNTP_RES_OK){
        return (xi_time_t)sntpMs;
    }
    else{
        return -1;
    }
}
