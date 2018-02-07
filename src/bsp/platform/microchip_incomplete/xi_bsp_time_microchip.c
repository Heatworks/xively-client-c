/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_time.h>
//#include <time.h>
//#include "tcpip/tcpip.h"
#include "wifi.h"
#include <xc.h>
#include "system_config.h"
#include "system_definitions.h"


void xi_bsp_time_init()
{
}

//xi_time_t xi_bsp_time_getcurrenttime_milliseconds()
//{
//    return ( xi_time_t )TickConvertToMilliseconds( TickGet() );
//}

//Adopted from Microchip's TCPIP sntp.c
static uint32_t TCPIP_SNTP_CurrTime(uint32_t* pMs)
{

    TCPIP_SNTP_TIME_STAMP deltaStamp, fractStamp;
    
    uint32_t ticksPerSec = SYS_TMR_TickCounterFrequencyGet();
    uint64_t deltaTick = SYS_TMR_TickCountGetLong() - wifiData.ntpTimeStampTicks;
    
    // calculate seconds = deltaTick / ticksPerSec;
    deltaStamp.tStampSeconds = (uint32_t)(deltaTick / ticksPerSec);

    // calculate fract part: (deltaTick % ticksPerSec) / ticksPerSec) * 2^32 ; 
    fractStamp.tStampSeconds = deltaTick - deltaStamp.tStampSeconds * ticksPerSec;
    fractStamp.tStampFraction = 0;

    deltaStamp.tStampFraction = (uint32_t)(fractStamp.llStamp / ticksPerSec);

    // 64 bit addition gets us the new time stamp
    deltaStamp.llStamp += wifiData.latestSecsSince1900.llStamp;


    // calculate milliseconds: (fract / 2 ^ 32) * 1000;
    if(pMs)
    {
        fractStamp.llStamp = (uint64_t)deltaStamp.tStampFraction * 1000;
        *pMs = fractStamp.tStampSeconds;
    }

    return deltaStamp.tStampSeconds - TCPIP_NTP_EPOCH;
}


xi_time_t xi_bsp_time_getcurrenttime_seconds(void)
{
    
    return (xi_time_t)TCPIP_SNTP_CurrTime(0);
}

xi_time_t xi_bsp_time_getcurrenttime_milliseconds(void)
{
    uint32_t msTime;
    TCPIP_SNTP_CurrTime(&msTime);
    return (xi_time_t)msTime;
}
