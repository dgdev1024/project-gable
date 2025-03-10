/**
 * @file      GABLE/InterruptContext.h
 * @brief     Contains the GABLE Engine's interrupt context structure and public functions.
 * 
 * The GABLE Engine's interrupt context seeks to simulate the behavior of the Game Boy CPU's
 * interrupt request and servicing mechanism.
 * 
 * The interrupt context is responsible for the following hardware registers:
 * 
 * - `IF` (Interrupt Flag) - The interrupt flag register, which contains the status of the
 *   interrupt requests generated by the CPU. Each bit in this register corresponds to a specific
 *   interrupt request, and is set when the corresponding interrupt is requested.
 * 
 * - `IE` (Interrupt Enable) - The interrupt enable register, which contains the mask for the
 *   interrupt requests generated by the CPU. Each bit in this register corresponds to a specific
 *   interrupt request, and is set when the corresponding interrupt is enabled.
 * 
 * The interrupt context also contains an "interrupt master enable" (`IME`) flag, which controls
 * whether the CPU should service interrupts or not. When the `IME` flag is set, the CPU will service
 * interrupts as they are requested, provided that the corresponding interrupt is enabled in the
 * `IE` register. When the `IME` flag is cleared, the CPU will not service interrupts, regardless of
 * the state of the `IE` register.
 * 
 * The interrupt context contains a set of pointers to functions which are called when the CPU
 * services a specific interrupt. These functions are called "interrupt handlers", and are set by the
 * game software to handle specific events, which are described below:
 * 
 * - `0` - Vertical Blank Interrupt (`VBLANK`) - This interrupt is requested when the GABLE engine's
 *   pixel-processing unit (PPU) has finished processing the visible portion of the current frame,
 *   and is about to start processing the next frame. This interrupt is typically used to update the
 *   game's logic and systems.
 * 
 * - `1` - LCD Status Interrupt (`LCD_STAT`) - This interrupt is requested when the GABLE engine's
 *   PPU enters certain specific states during the rendering of the current frame, such as when the
 *   PPU enters the vertical blank (`VBLANK`) state, horizontal blank (`HBLANK`) state, or object
 *   scan state (`OAM_SCAN`), or when the PPU's current scanline matches its line compare register.
 *   This interrupt is typically used to synchronize the game's logic with the PPU's rendering
 *   process and can be used to implement special effects, such as screen transitions or split
 *   screens.
 * 
 * - `2` - Timer Overflow Interrupt (`TIMER`) - This interrupt is requested when the GABLE engine's
 *   timer counter register (`TIMA`) overflows. This interrupt is typically used to update the game's
 *   logic and systems based on the passage of time.
 * 
 * - `3` - Serial Transfer Complete Interrupt (`SERIAL`) - This interrupt is requested when the GABLE
 *   engine's serial transfer controller (`SC`) has completed a serial data transfer. This interrupt
 *   is typically used to handle communication between the game software and external devices. In
 *   the case of the GABLE Engine, the serial transfer controller will likely use some kind of
 *   network socket to communicate with other instances of the GABLE Engine running on other
 *   machines, likely via TCP/IP or UDP. This interrupt is typically used to handle network
 *   communication events.
 * 
 * - `4` - Joypad Input Interrupt (`JOYP`) - This interrupt is requested when the GABLE engine's
 *   joypad register (`JOYP`) detects a change in the state of the joypad buttons or direction pad,
 *   depending on which of these are enabled in the joypad register. This interrupt is typically used
 *   to handle player input events.
 * 
 * - `5` - Real-Time Clock Interrupt (`RTC`) - This interrupt is requested when one or more of the
 *   GABLE engine's real-time clock registers (`RTCS`, `RTCM`, `RTCH`, `RTCDH`, `RTCDL`) have
 *   changed. This interrupt is typically used to handle time-based events in the game software.
 * 
 * - `6` - Network Transfer Complete Interrupt (`NET`) - This interrupt is requested when the GABLE
 *   engine's network interface has completed a data transfer. This interrupt is typically used to
 *   handle network communication events or network errors.
 * 
 */

#pragma once
#include <GABLE/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward declaration of the GABLE Engine structure.
 */
typedef struct GABLE_Engine GABLE_Engine;

/**
 * @brief The GABLE Engine's interrupt context structure.
 */
typedef struct GABLE_InterruptContext GABLE_InterruptContext;

/**
 * @brief The GABLE Engine's interrupt handler function type.
 * @param p_Engine  A pointer to the GABLE Engine instance whose interrupt context is servicing the
 *                  interrupt.
 * @return `true` if the interrupt was serviced successfully; `false` otherwise.
 */
typedef Bool (*GABLE_InterruptHandler) (GABLE_Engine* p_Engine);

// Interrupt Type Enumeration //////////////////////////////////////////////////////////////////////

/**
 * @brief Enumerates the types of interrupts that can be requested by the GABLE Engine.
 */
typedef enum GABLE_InterruptType
{
    GABLE_INT_VBLANK = 0,   ///< @brief Vertical Blank Interrupt
    GABLE_INT_LCD_STAT,     ///< @brief LCD Status Interrupt
    GABLE_INT_TIMER,        ///< @brief Timer Overflow Interrupt
    GABLE_INT_SERIAL,       ///< @brief Serial Transfer Complete Interrupt
    GABLE_INT_JOYPAD,       ///< @brief Joypad Input Interrupt
    GABLE_INT_RTC,          ///< @brief Real-Time Clock Interrupt
    GABLE_INT_NET,          ///< @brief Network Transfer Complete Interrupt

    GABLE_INT_COUNT         ///< @brief The number of interrupt types
} GABLE_InterruptType;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief      Creates a new GABLE Engine interrupt context instance.
 * 
 * @return     A pointer to the new GABLE Engine interrupt context instance.
 */
GABLE_InterruptContext* GABLE_CreateInterruptContext ();

/**
 * @brief      Destroys a GABLE Engine interrupt context instance.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance to destroy.
 */
void GABLE_DestroyInterruptContext (GABLE_InterruptContext* p_Context);

/**
 * @brief      Services one interrupt which is currently requested by the GABLE Engine.
 * 
 * Interrupts are serviced in the following order of priority:
 * - Vertical Blank Interrupt (`VBLANK`)
 * - LCD Status Interrupt (`LCD_STAT`)
 * - Timer Overflow Interrupt (`TIMER`)
 * - Serial Transfer Complete Interrupt (`SERIAL`)
 * - Joypad Input Interrupt (`JOYP`)
 * - Real-Time Clock Interrupt (`RTC`)
 * - Network Transfer Complete Interrupt (`NET`)
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * @param      p_Engine   A pointer to the GABLE Engine instance whose interrupt context is
 *                        servicing the interrupt.
 * 
 * @return     `1` if an interrupt was serviced; `0` if no interrupt was serviced; `-1` if an error
 *             occurred while servicing the interrupt.
 */
Int32 GABLE_ServiceInterrupt (GABLE_InterruptContext* p_Context, GABLE_Engine* p_Engine);

/**
 * @brief      Requests an interrupt of the specified type.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * @param      p_Type     The type of interrupt to request.
 */
void GABLE_RequestInterrupt (GABLE_InterruptContext* p_Context, GABLE_InterruptType p_Type);

/**
 * @brief      Checks if the interrupt master enable flag is currently set.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * 
 * @return     `true` if the interrupt master enable flag is set; `false` otherwise.
 */
Bool GABLE_IsInterruptMasterEnabled (const GABLE_InterruptContext* p_Context);

/**
 * @brief      Checks if an interrupt of the specified type is currently enabled.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * @param      p_Type     The type of interrupt to check.
 * 
 * @return     `true` if the interrupt is currently enabled; `false` otherwise.
 */
Bool GABLE_IsInterruptEnabled (const GABLE_InterruptContext* p_Context, GABLE_InterruptType p_Type);

/**
 * @brief      Checks if an interrupt of the specified type is currently requested.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * @param      p_Type     The type of interrupt to check.
 * 
 * @return     `true` if the interrupt is currently requested; `false` otherwise.
 */
Bool GABLE_IsInterruptRequested (const GABLE_InterruptContext* p_Context, GABLE_InterruptType p_Type);

/**
 * @brief      Enables or disables an interrupt of the specified type.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * @param      p_Type     The type of interrupt to enable or disable.
 * @param      p_Enable   `true` to enable the interrupt; `false` to disable the interrupt.
 */
void GABLE_SetInterruptEnable (GABLE_InterruptContext* p_Context, GABLE_InterruptType p_Type, Bool p_Enable);

/**
 * @brief      Sets the interrupt handler for the specified interrupt type.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * @param      p_Type     The type of interrupt to set the handler for.
 * @param      p_Handler  A pointer to the interrupt handler function.
 */
void GABLE_SetInterruptHandler (GABLE_InterruptContext* p_Context, GABLE_InterruptType p_Type, GABLE_InterruptHandler p_Handler);

/**
 * @brief      Sets the interrupt master enable flag.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * @param      p_Enable   `true` to enable interrupts; `false` to disable interrupts.
 */
void GABLE_SetInterruptMasterEnable (GABLE_InterruptContext* p_Context, Bool p_Enable);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief      Gets the value of the `IF` (Interrupt Flag) register.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * 
 * @return     The value of the `IF` register.
 */
Uint8 GABLE_ReadIF (const GABLE_InterruptContext* p_Context);

/**
 * @brief      Gets the value of the `IE` (Interrupt Enable) register.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * 
 * @return     The value of the `IE` register.
 */
Uint8 GABLE_ReadIE (const GABLE_InterruptContext* p_Context);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief      Sets the value of the `IF` (Interrupt Flag) register.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * @param      p_Value    The new value of the `IF` register.
 */
void GABLE_WriteIF (GABLE_InterruptContext* p_Context, Uint8 p_Value);

/**
 * @brief      Sets the value of the `IE` (Interrupt Enable) register.
 * 
 * @param      p_Context  A pointer to the GABLE Engine interrupt context instance.
 * @param      p_Value    The new value of the `IE` register.
 */
void GABLE_WriteIE (GABLE_InterruptContext* p_Context, Uint8 p_Value);
