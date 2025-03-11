/**
 * @file GABLE/Timer.c
 */

#include <GABLE/Engine.h>
#include <GABLE/InterruptContext.h>
#include <GABLE/Timer.h>

// GABLE Timer Structure ///////////////////////////////////////////////////////////////////////////

typedef struct GABLE_Timer
{
    Uint16              m_OldDIV;       ///< @brief The old value of the DIV register.
    Uint16              m_DIV;          ///< @brief The value of the DIV register.
    Uint8               m_TIMA;         ///< @brief The value of the TIMA register.
    Uint8               m_TMA;          ///< @brief The value of the TMA register.
    GABLE_TimerControl  m_TAC;          ///< @brief The value of the TAC register.
} GABLE_Timer;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_Timer* GABLE_CreateTimer ()
{
    // Allocate the GABLE Engine timer instance.
    GABLE_Timer* l_Timer = GABLE_calloc(1, GABLE_Timer);
    GABLE_pexpect(l_Timer != NULL, "Failed to allocate GABLE Engine timer");

    // Initialize the timer's properties.
    l_Timer->m_OldDIV           = 0;
    l_Timer->m_DIV              = 0;
    l_Timer->m_TIMA             = 0;
    l_Timer->m_TMA              = 0;
    l_Timer->m_TAC.m_Register   = 0;

    // Return the new timer instance.
    return l_Timer;
}

void GABLE_DestroyTimer (GABLE_Timer* p_Timer)
{
    if (p_Timer != NULL)
    {
        // Free the timer instance.
        GABLE_free(p_Timer);
    }
}

void GABLE_TickTimer (GABLE_Timer* p_Timer, GABLE_Engine* p_Engine)
{
    // Validate the timer and engine instances.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Tick the divider. Store the old value.
    p_Timer->m_OldDIV = p_Timer->m_DIV++;

    // Check if the timer is enabled.
    if (p_Timer->m_TAC.m_Enable == false)
    {
        return;
    }

    // The bits of the 16-bit divider change from high to low at the following rates:
    // - Bit 0: Changes at 4194304 Hz (every cycle); from high to low at 2097152 Hz (every two cycles).
    // - Bit 1: Changes at 2097152 Hz (every two cycles); from high to low at 1048576 Hz (every four cycles).
    // - Bit 2: Changes at 1048576 Hz (every four cycles); from high to low at 524288 Hz (every eight cycles).
    // - Bit 3: Changes at 524288 Hz (every eight cycles); from high to low at 262144 Hz (every 16 cycles).
    // - Bit 4: Changes at 262144 Hz (every 16 cycles); from high to low at 131072 Hz (every 32 cycles).
    // - Bit 5: Changes at 131072 Hz (every 32 cycles); from high to low at 65536 Hz (every 64 cycles).
    // - Bit 6: Changes at 65536 Hz (every 64 cycles); from high to low at 32768 Hz (every 128 cycles).
    // - Bit 7: Changes at 32768 Hz (every 128 cycles); from high to low at 16384 Hz (every 256 cycles).
    // - Bit 8: Changes at 16384 Hz (every 256 cycles); from high to low at 8192 Hz (every 512 cycles).
    // - Bit 9: Changes at 8192 Hz (every 512 cycles); from high to low at 4096 Hz (every 1,024 cycles).
    // - Bit 10: Changes at 4096 Hz (every 1,024 cycles); from high to low at 2048 Hz (every 2,048 cycles).
    // - Bit 11: Changes at 2048 Hz (every 2,048 cycles); from high to low at 1024 Hz (every 4,096 cycles).
    // - Bit 12: Changes at 1024 Hz (every 4,096 cycles); from high to low at 512 Hz (every 8,192 cycles).
    // - Bit 13: Changes at 512 Hz (every 8,192 cycles); from high to low at 256 Hz (every 16,384 cycles).
    // - Bit 14: Changes at 256 Hz (every 16,384 cycles); from high to low at 128 Hz (every 32,768 cycles).
    // - Bit 15: Changes at 128 Hz (every 32,768 cycles); from high to low at 64 Hz (every 65,536 cycles).
    
    // Depending on the timer's clock speed, determine which divider bit needs to be checked.
    Uint8 l_Bit = 0;
    switch (p_Timer->m_TAC.m_ClockSpeed)
    {
        case GABLE_TCS_4096_HZ:     l_Bit = 9; break;
        case GABLE_TCS_262144_HZ:   l_Bit = 3; break;
        case GABLE_TCS_65536_HZ:    l_Bit = 5; break;
        case GABLE_TCS_16384_HZ:    l_Bit = 7; break;
    }

    // Check if the divider bit has transitioned from high to low.
    Bool l_TimerNeedsTick = GABLE_CheckTimerDividerBit(p_Timer, l_Bit);
    if (l_TimerNeedsTick == true && ++p_Timer->m_TIMA == 0)
    {
        // Reset the TIMA register to the TMA register value.
        p_Timer->m_TIMA = p_Timer->m_TMA;

        // Request a timer interrupt.
        GABLE_RequestInterrupt(p_Engine, GABLE_INT_TIMER);
    }

}

Bool GABLE_CheckTimerDividerBit (GABLE_Timer* p_Timer, Uint8 p_Bit)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Check if the specified bit has transitioned from high to low.
    Bool l_OldBit = GABLE_bit(p_Timer->m_OldDIV, p_Bit);
    Bool l_NewBit = GABLE_bit(p_Timer->m_DIV, p_Bit);
    return (l_OldBit == true && l_NewBit == false);
}

GABLE_TimerClockSpeed GABLE_GetTimerClockSpeed (const GABLE_Timer* p_Timer)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Return the timer's clock speed setting.
    return (GABLE_TimerClockSpeed) p_Timer->m_TAC.m_ClockSpeed;
}

Bool GABLE_IsTimerEnabled (const GABLE_Timer* p_Timer)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Return the timer's enable flag.
    return p_Timer->m_TAC.m_Enable;
}

Uint8 GABLE_GetTimerModulo (const GABLE_Timer* p_Timer)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Return the timer's modulo register value.
    return p_Timer->m_TMA;
}

Uint8 GABLE_GetTimerCounter (const GABLE_Timer* p_Timer)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Return the timer's counter register value.
    return p_Timer->m_TIMA;
}

void GABLE_SetTimerClockSpeed (GABLE_Timer* p_Timer, GABLE_TimerClockSpeed p_Speed)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Set the timer's clock speed setting.
    p_Timer->m_TAC.m_ClockSpeed = (Uint8) p_Speed;
}

void GABLE_SetTimerEnable (GABLE_Timer* p_Timer, Bool p_Enable)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Set the timer's enable flag.
    p_Timer->m_TAC.m_Enable = p_Enable;
}

void GABLE_SetTimerModulo (GABLE_Timer* p_Timer, Uint8 p_Value)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Set the timer's modulo register value.
    p_Timer->m_TMA = p_Value;
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadDIV (const GABLE_Timer* p_Timer)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Return the DIV register value.
    return (p_Timer->m_DIV >> 8) & 0xFF;
}

Uint8 GABLE_ReadTIMA (const GABLE_Timer* p_Timer)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Return the TIMA register value.
    return p_Timer->m_TIMA;
}

Uint8 GABLE_ReadTMA (const GABLE_Timer* p_Timer)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Return the TMA register value.
    return p_Timer->m_TMA;
}

Uint8 GABLE_ReadTAC (const GABLE_Timer* p_Timer)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Return the TAC register value.
    return p_Timer->m_TAC.m_Register;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void GABLE_WriteDIV (GABLE_Timer* p_Timer, Uint8 p_Value)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    (void) p_Value; // Unused.

    // Any write to the DIV register resets it to zero.
    p_Timer->m_DIV = 0;
}

void GABLE_WriteTIMA (GABLE_Timer* p_Timer, Uint8 p_Value)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Set the TIMA register value.
    p_Timer->m_TIMA = p_Value;
}

void GABLE_WriteTMA (GABLE_Timer* p_Timer, Uint8 p_Value)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Set the TMA register value.
    p_Timer->m_TMA = p_Value;
}

void GABLE_WriteTAC (GABLE_Timer* p_Timer, Uint8 p_Value)
{
    // Validate the timer instance.
    GABLE_expect(p_Timer != NULL, "Timer context is NULL!");

    // Set the TAC register value.
    p_Timer->m_TAC.m_Register = p_Value;
}
