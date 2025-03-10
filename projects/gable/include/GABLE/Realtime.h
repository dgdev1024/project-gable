/**
 * @file     GABLE/Realtime.h
 * @brief    Contains the GABLE Engine's real-time clock interface.
 * 
 * The GABLE Engine's real-time clock (RTC) is a simple clock which keeps track of the current time
 * in days, hours, minutes and seconds. It seeks to simulate the real-time clock found in Game Boy
 * games using the MBC3 memory bank controller, which includes a battery-backed real-time clock
 * oscillator - namely the Generation II Pokemon games (Gold, Silver, and Crystal).
 * 
 * The real-time clock interface is responsible for the following hardware registers:
 * 
 * - `RTCS` (Real-Time Clock Seconds) - The real-time clock's seconds register, which contains the
 *   current number of seconds in the current minute (0-60, with 60 being used for leap seconds).
 * 
 * - `RTCM` (Real-Time Clock Minutes) - The real-time clock's minutes register, which contains the
 *   current number of minutes in the current hour (0-59).
 * 
 * - `RTCH` (Real-Time Clock Hours) - The real-time clock's hours register, which contains the current
 *   number of hours in the current day (0-23).
 * 
 * - `RTCDH` and `RTCDL` (Real-Time Clock Day Counter) - The real-time clock's day counter registers,
 *   which contain the current number of days since the start of the real-time clock's epoch. The
 *   day counter is a 16-bit register which can store up to 65535 days, but is only intended to be
 *   used for a single year's worth of days (0-364, or 365 for leap years).
 * 
 * - `RTCL` (Real-Time Clock Latch) - The real-time clock's latch register, which is used to update
 *   the real-time clock's day counter registers. Writing any value to this register will cause the
 *   current time to be latched into the day counter registers.
 * 
 * The real-time clock interface is responsible for requesting the following interrupt(s):
 * 
 * - Real-Time Clock Interrupt (`RTC`) - This interrupt is requested when one or more of the GABLE
 *   engine's real-time clock registers have changed. This interrupt is typically used to handle
 *   time-based events in the game software.
 */

#pragma once
#include <GABLE/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward declaration of the GABLE Engine structure.
 */
typedef struct GABLE_Engine GABLE_Engine;

/**
 * @brief The GABLE Engine's real-time clock structure.
 */
typedef struct GABLE_Realtime GABLE_Realtime;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief      Creates a new GABLE Engine real-time clock instance.
 * 
 * @return     A pointer to the new GABLE Engine real-time clock instance.
 */
GABLE_Realtime* GABLE_CreateRealtime ();

/**
 * @brief      Destroys a GABLE Engine real-time clock instance.
 * 
 * @param      p_Realtime  A pointer to the GABLE Engine real-time clock instance to destroy.
 */
void GABLE_DestroyRealtime (GABLE_Realtime* p_Realtime);

/**
 * @brief      Latches the current day and time into the real-time clock's day counter registers.
 * 
 * @param      p_Realtime  A pointer to the GABLE Engine real-time clock instance.
 * @param      p_Engine    A pointer to the GABLE Engine instance.
 */
void GABLE_LatchRealtime (GABLE_Realtime* p_Realtime, GABLE_Engine* p_Engine);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief      Gets the value of the `RTCS` (Real-Time Clock Seconds) register.
 * 
 * @param      p_Realtime  A pointer to the GABLE Engine real-time clock instance.
 * 
 * @return     The current value of the real-time clock's seconds register.
 */
Uint8 GABLE_ReadRTCS (const GABLE_Realtime* p_Realtime);

/**
 * @brief      Gets the value of the `RTCM` (Real-Time Clock Minutes) register.
 * 
 * @param      p_Realtime  A pointer to the GABLE Engine real-time clock instance.
 * 
 * @return     The current value of the real-time clock's minutes register.
 */
Uint8 GABLE_ReadRTCM (const GABLE_Realtime* p_Realtime);

/**
 * @brief      Gets the value of the `RTCH` (Real-Time Clock Hours) register.
 * 
 * @param      p_Realtime  A pointer to the GABLE Engine real-time clock instance.
 * 
 * @return     The current value of the real-time clock's hours register.
 */
Uint8 GABLE_ReadRTCH (const GABLE_Realtime* p_Realtime);

/**
 * @brief      Gets the value of the `RTCDH` (Real-Time Clock Day Counter High) register.
 * 
 * @param      p_Realtime  A pointer to the GABLE Engine real-time clock instance.
 * 
 * @return     The current value of the real-time clock's day counter high register.
 */
Uint8 GABLE_ReadRTCDH (const GABLE_Realtime* p_Realtime);

/**
 * @brief      Gets the value of the `RTCDL` (Real-Time Clock Day Counter Low) register.
 * 
 * @param      p_Realtime  A pointer to the GABLE Engine real-time clock instance.
 * 
 * @return     The current value of the real-time clock's day counter low register.
 */
Uint8 GABLE_ReadRTCDL (const GABLE_Realtime* p_Realtime);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief      Writes to the `RTCL` (Real-Time Clock Latch) register.
 * 
 * Writing any value to this register will cause the current time to be latched into the RTC's
 * registers.
 * 
 * @param      p_Realtime  A pointer to the GABLE Engine real-time clock instance.
 * @param      p_Engine    A pointer to the GABLE Engine instance.
 * @param      p_Value     The value to write to the `RTCL` register.
 * 
 * @note The `p_Engine` parameter is required to request the `RTC` interrupt.
 */
void GABLE_WriteRTCL (GABLE_Realtime* p_Realtime, GABLE_Engine* p_Engine, Uint8 p_Value);
