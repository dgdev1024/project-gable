/**
 * @file GABLE/InterruptContext.c
 */

#include <GABLE/Engine.h>
#include <GABLE/InterruptContext.h>

// GABLE Interrupt Context Structure ///////////////////////////////////////////////////////////////

typedef struct GABLE_InterruptContext
{
    GABLE_InterruptHandler m_Handlers[GABLE_INT_COUNT]; ///< @brief The interrupt handlers.
    Uint8                  m_IF;                        ///< @brief The interrupt flag register.
    Uint8                  m_IE;                        ///< @brief The interrupt enable register.
    Bool                   m_IME;                       ///< @brief The interrupt master enable flag.
} GABLE_InterruptContext;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_InterruptContext* GABLE_CreateInterruptContext ()
{
    // Allocate the GABLE Engine interrupt context instance.
    GABLE_InterruptContext* l_Context = GABLE_calloc(1, GABLE_InterruptContext);
    GABLE_pexpect(l_Context != NULL, "Failed to allocate GABLE Engine interrupt context");

    // Initialize the interrupt context's properties.
    l_Context->m_IF  = 0;
    l_Context->m_IE  = 0;
    l_Context->m_IME = false;

    // Return the new interrupt context instance.
    return l_Context;
}

void GABLE_DestroyInterruptContext (GABLE_InterruptContext* p_Context)
{
    if (p_Context != NULL)
    {
        // Free the interrupt context instance.
        GABLE_free(p_Context);
    }
}

Int32 GABLE_ServiceInterrupt (GABLE_InterruptContext* p_Context, GABLE_Engine* p_Engine)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Check if the interrupt master enable flag is set.
    if (GABLE_IsInterruptMasterEnabled(p_Context))
    {
        // Check if any interrupts are requested.
        for (Uint8 i = 0; i < GABLE_INT_COUNT; i++)
        {
            // Check if the interrupt is enabled and requested.
            if (GABLE_IsInterruptEnabled(p_Context, i) && GABLE_IsInterruptRequested(p_Context, i))
            {
                // Call the interrupt handler.
                if (p_Context->m_Handlers[i] != NULL)
                {
                    if (p_Context->m_Handlers[i](p_Engine))
                    {
                        // Clear the interrupt request.
                        GABLE_clearbit(p_Context->m_IF, i);

                        // Return success.
                        return 1;
                    }
                    else
                    {
                        // Return failure.
                        return -1;
                    }
                }
            }
        }
    }

    // Return no interrupt serviced.
    return 0;
}

void GABLE_RequestInterrupt (GABLE_InterruptContext* p_Context, GABLE_InterruptType p_Type)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Set the interrupt request.
    GABLE_setbit(p_Context->m_IF, p_Type);
}

Bool GABLE_IsInterruptMasterEnabled (const GABLE_InterruptContext* p_Context)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Return the interrupt master enable flag.
    return p_Context->m_IME;
}

Bool GABLE_IsInterruptEnabled (const GABLE_InterruptContext* p_Context, GABLE_InterruptType p_Type)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Return the interrupt enable flag.
    return GABLE_bit(p_Context->m_IE, p_Type);
}

Bool GABLE_IsInterruptRequested (const GABLE_InterruptContext* p_Context, GABLE_InterruptType p_Type)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Return the interrupt request flag.
    return GABLE_bit(p_Context->m_IF, p_Type);
}

void GABLE_SetInterruptEnable (GABLE_InterruptContext* p_Context, GABLE_InterruptType p_Type, Bool p_Enable)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Set the interrupt enable flag.
    GABLE_changebit(p_Context->m_IE, p_Type, p_Enable);
}

void GABLE_SetInterruptHandler (GABLE_InterruptContext* p_Context, GABLE_InterruptType p_Type, GABLE_InterruptHandler p_Handler)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Set the interrupt handler.
    p_Context->m_Handlers[p_Type] = p_Handler;
}

void GABLE_SetInterruptMasterEnable (GABLE_InterruptContext* p_Context, Bool p_Enable)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Set the interrupt master enable flag.
    p_Context->m_IME = p_Enable;
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadIF (const GABLE_InterruptContext* p_Context)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Return the interrupt flag register.
    return p_Context->m_IF;
}

Uint8 GABLE_ReadIE (const GABLE_InterruptContext* p_Context)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Return the interrupt enable register.
    return p_Context->m_IE;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void GABLE_WriteIF (GABLE_InterruptContext* p_Context, Uint8 p_Value)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Set the interrupt flag register.
    p_Context->m_IF = p_Value;
}

void GABLE_WriteIE (GABLE_InterruptContext* p_Context, Uint8 p_Value)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Set the interrupt enable register.
    p_Context->m_IE = p_Value;
}
