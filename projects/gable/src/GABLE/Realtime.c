/**
 * @file GABLE/Realtime.c
 */

#include <GABLE/Engine.h>
#include <GABLE/InterruptContext.h>
#include <GABLE/Realtime.h>

// GABLE Real-Time Clock Structure /////////////////////////////////////////////////////////////////

typedef struct GABLE_Realtime
{
    Uint8 m_RTCS;   ///< @brief The value of the `RTCS` register.
    Uint8 m_RTCM;   ///< @brief The value of the `RTCM` register.
    Uint8 m_RTCH;   ///< @brief The value of the `RTCH` register.
    Uint8 m_RTCDH;  ///< @brief The value of the `RTCDH` register.
    Uint8 m_RTCDL;  ///< @brief The value of the `RTCDL` register.
} GABLE_Realtime;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_Realtime* GABLE_CreateRealtime ()
{
    // Allocate the GABLE Engine real-time clock instance.
    GABLE_Realtime* l_Realtime = GABLE_calloc(1, GABLE_Realtime);
    GABLE_pexpect(l_Realtime != NULL, "Failed to allocate GABLE Engine real-time clock");

    // Poll the system clock for the current day and time.
    time_t l_Time = time(NULL);
    struct tm* l_LocalTime = localtime(&l_Time);

    // Initialize the real-time clock's properties.
    l_Realtime->m_RTCS  = l_LocalTime->tm_sec;
    l_Realtime->m_RTCM  = l_LocalTime->tm_min;
    l_Realtime->m_RTCH  = l_LocalTime->tm_hour;
    l_Realtime->m_RTCDH = (l_LocalTime->tm_yday & 0xFF00) >> 8;
    l_Realtime->m_RTCDL = (l_LocalTime->tm_yday & 0x00FF);

    // Return the new real-time clock instance.
    return l_Realtime;
}

void GABLE_DestroyRealtime (GABLE_Realtime* p_Realtime)
{
    if (p_Realtime != NULL)
    {
        // Free the real-time clock instance.
        GABLE_free(p_Realtime);
    }
}

void GABLE_LatchRealtime (GABLE_Realtime* p_Realtime, GABLE_Engine* p_Engine)
{
    // Validate the real-time clock instance.
    GABLE_expect(p_Realtime != NULL, "Real-time clock context is NULL!");
    
    // Poll the system clock for the current day and time.
    time_t l_Time = time(NULL);
    struct tm* l_LocalTime = localtime(&l_Time);

    // Store the old values of the RTC registers.
    Uint8 l_OldRTCS  = p_Realtime->m_RTCS;
    Uint8 l_OldRTCM  = p_Realtime->m_RTCM;
    Uint8 l_OldRTCH  = p_Realtime->m_RTCH;
    Uint8 l_OldRTCDH = p_Realtime->m_RTCDH;
    Uint8 l_OldRTCDL = p_Realtime->m_RTCDL;

    // Get the day.
    Uint16 l_Day = l_LocalTime->tm_yday;

    // Update the RTC registers with the current day and time.
    p_Realtime->m_RTCS  = l_LocalTime->tm_sec;
    p_Realtime->m_RTCM  = l_LocalTime->tm_min;
    p_Realtime->m_RTCH  = l_LocalTime->tm_hour;
    p_Realtime->m_RTCDH = (l_Day & 0xFF00) >> 8;
    p_Realtime->m_RTCDL = (l_Day & 0x00FF);

    // If any of the above registers' values have changed, then we need to request an RTC interrupt.
    if (
        (l_OldRTCS  != p_Realtime->m_RTCS)  ||
        (l_OldRTCM  != p_Realtime->m_RTCM)  ||
        (l_OldRTCH  != p_Realtime->m_RTCH)  ||
        (l_OldRTCDH != p_Realtime->m_RTCDH) ||
        (l_OldRTCDL != p_Realtime->m_RTCDL)
    )
    {
        GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_RTC);
    }
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadRTCS (const GABLE_Realtime* p_Realtime)
{
    // Validate the real-time clock instance.
    GABLE_expect(p_Realtime != NULL, "Real-time clock context is NULL!");

    // Return the `RTCS` register value.
    return p_Realtime->m_RTCS;
}

Uint8 GABLE_ReadRTCM (const GABLE_Realtime* p_Realtime)
{
    // Validate the real-time clock instance.
    GABLE_expect(p_Realtime != NULL, "Real-time clock context is NULL!");

    // Return the `RTCM` register value.
    return p_Realtime->m_RTCM;
}

Uint8 GABLE_ReadRTCH (const GABLE_Realtime* p_Realtime)
{
    // Validate the real-time clock instance.
    GABLE_expect(p_Realtime != NULL, "Real-time clock context is NULL!");

    // Return the `RTCH` register value.
    return p_Realtime->m_RTCH;
}

Uint8 GABLE_ReadRTCDH (const GABLE_Realtime* p_Realtime)
{
    // Validate the real-time clock instance.
    GABLE_expect(p_Realtime != NULL, "Real-time clock context is NULL!");

    // Return the `RTCDH` register value.
    return p_Realtime->m_RTCDH;
}

Uint8 GABLE_ReadRTCDL (const GABLE_Realtime* p_Realtime)
{
    // Validate the real-time clock instance.
    GABLE_expect(p_Realtime != NULL, "Real-time clock context is NULL!");

    // Return the `RTCDL` register value.
    return p_Realtime->m_RTCDL;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void GABLE_WriteRTCL (GABLE_Realtime* p_Realtime, GABLE_Engine* p_Engine, Uint8 p_Value)
{
    // Validate the real-time clock and engine instances.
    GABLE_expect(p_Realtime != NULL, "Real-time clock context is NULL!");
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Latch the current day and time into the real-time clock's day counter registers.
    GABLE_LatchRealtime(p_Realtime, p_Engine);
}