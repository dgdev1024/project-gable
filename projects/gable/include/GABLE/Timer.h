/**
 * @file     GABLE/Timer.h
 * @brief    Contains the GABLE Engine's timer structure and public functions.
 * 
 * The GABLE Engine's timer component seeks to simulate the Game Boy's internal timer hardware.
 * The timer component is responsible for the following hardware registers:
 * 
 * - `DIV` (Divider Register) - The timer's divider register, which is incremented at a fixed rate
 *   and is used to increment the timer's counter register. The full divider is a 16-bit register
 *   which increments at the same rate as the CPU's clock speed. The upper 8 bits of this divider
 *   is mapped to the `DIV` register, and the lower 8 bits are not directly accessible.
 * 
 * - `TIMA` (Timer Counter) - The timer's counter register, which is incremented at a variable rate
 *   depending on the clock speed setting in the `TAC` register, and generates an interrupt when it
 *   overflows.
 * 
 * - `TMA` (Timer Modulo) - The timer's modulo register, which is used to reset the timer's counter
 *   register to a specific value when the counter overflows.
 * 
 * - `TAC` (Timer Control) - The timer's control register, which is used to set the clock speed of
 *   the timer and enable or disable the timer's operation. Its bits are defined as follows:
 * 
 *      - Bit 0 - 1: Clock Speed Select - This 2-bit field sets the clock speed of the timer. The
 *        possible values are:
 *          - `00` - 4096 Hz, the timer increments every 1024 cycles, the slowest speed.
 *          - `01` - 262144 Hz, the timer increments every 16 cycles, the fastest speed.
 *          - `10` - 65536 Hz, the timer increments every 64 cycles.
 *          - `11` - 16384 Hz, the timer increments every 256 cycles.
 *      - Bit 2 - Timer Enable - This bit enables or disables the timer. When the timer is disabled,
 *        the timer's counter register is stopped and does not increment. The timer's divider register
 *        continues to increment regardless of the state of this bit.
 * 
 * The timer component is responsible for reqesting the following interrupt(s):
 * 
 * - Timer Overflow Interrupt (`TIMER`) - This interrupt is requested when the timer's counter register
 *   overflows. This interrupt is typically used to update the game's logic and systems based on the
 *   passage of time.
 */

#pragma once
#include <GABLE/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward declaration of the GABLE Engine structure.
 */
typedef struct GABLE_Engine GABLE_Engine;

/**
 * @brief The GABLE Engine's timer structure.
 */
typedef struct GABLE_Timer GABLE_Timer;

// Timer Clock Speed Enumeration ///////////////////////////////////////////////////////////////////

/**
 * @brief Enumerates the possible clock speeds of the GABLE Engine's timer.
 */
typedef enum GABLE_TimerClockSpeed
{
    GABLE_TCS_4096_HZ = 0,  ///< @brief 4096 Hz, the timer increments every 1024 cycles.
    GABLE_TCS_262144_HZ,    ///< @brief 262144 Hz, the timer increments every 16 cycles.
    GABLE_TCS_65536_HZ,     ///< @brief 65536 Hz, the timer increments every 64 cycles.
    GABLE_TCS_16384_HZ,     ///< @brief 16384 Hz, the timer increments every 256 cycles.

    GABLE_TCS_SLOWEST = GABLE_TCS_4096_HZ,      ///< @brief The slowest clock speed.
    GABLE_TCS_FASTEST = GABLE_TCS_262144_HZ,    ///< @brief The fastest clock speed.
    GABLE_TCS_FAST = GABLE_TCS_65536_HZ,        ///< @brief A fast clock speed.
    GABLE_TCS_SLOW = GABLE_TCS_16384_HZ         ///< @brief A slow clock speed.
} GABLE_TimerClockSpeed;

// Timer Control Union /////////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the GABLE Engine's timer control (`TAC`) register.
 */
typedef union GABLE_TimerControl
{
    struct
    {
        Uint8 m_ClockSpeed : 2;  ///< @brief The clock speed select bits.
        Uint8 m_Enable : 1;      ///< @brief The timer enable bit.
        Uint8 : 5;               ///< @brief Unused bits.
    };
    Uint8 m_Register;           ///< @brief The raw register value.
} GABLE_TimerControl;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief      Creates a new GABLE Engine timer instance.
 * 
 * @return     A pointer to the new GABLE Engine timer instance.
 */
GABLE_Timer* GABLE_CreateTimer ();

/**
 * @brief      Resets a GABLE Engine timer instance.
 * 
 * @param      p_Timer  A pointer to the GABLE Engine timer instance to reset.
 */
void GABLE_ResetTimer (GABLE_Timer* p_Timer);

/**
 * @brief      Destroys a GABLE Engine timer instance.
 * 
 * @param      p_Timer  A pointer to the GABLE Engine timer instance to destroy.
 */
void GABLE_DestroyTimer (GABLE_Timer* p_Timer);

/**
 * @brief      Ticks the GABLE Engine's timer component.
 * 
 * @param      p_Timer  A pointer to the GABLE Engine timer instance.
 * @param      p_Engine A pointer to the GABLE Engine instance.
 */
void GABLE_TickTimer (GABLE_Timer* p_Timer, GABLE_Engine* p_Engine);

/**
 * @brief      Checks to see if the given bit of the timer's divider register has gone from high to
 *             low (from 1 to 0).
 * 
 * @param      p_Timer  A pointer to the GABLE Engine timer instance.
 * @param      p_Bit    The bit of the divider register to check.
 * 
 * @return     `true` if the bit has transitioned from high to low; `false` otherwise.
 */
Bool GABLE_CheckTimerDividerBit (GABLE_Timer* p_Timer, Uint8 p_Bit);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadDIV (const GABLE_Timer* p_Timer);
Uint8 GABLE_ReadTIMA (const GABLE_Timer* p_Timer);
Uint8 GABLE_ReadTMA (const GABLE_Timer* p_Timer);
Uint8 GABLE_ReadTAC (const GABLE_Timer* p_Timer);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void GABLE_WriteDIV (GABLE_Timer* p_Timer, Uint8 p_Value);
void GABLE_WriteTIMA (GABLE_Timer* p_Timer, Uint8 p_Value);
void GABLE_WriteTMA (GABLE_Timer* p_Timer, Uint8 p_Value);
void GABLE_WriteTAC (GABLE_Timer* p_Timer, Uint8 p_Value);
