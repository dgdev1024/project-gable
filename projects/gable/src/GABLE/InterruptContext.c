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
    GABLE_ResetInterruptContext(l_Context);

    // Return the new interrupt context instance.
    return l_Context;
}

void GABLE_ResetInterruptContext (GABLE_InterruptContext* p_Context)
{
    // Validate the interrupt context instance.
    GABLE_expect(p_Context != NULL, "Interrupt context is NULL!");

    // Reset the interrupt context's properties.
    p_Context->m_IF  = 0xE1;
    p_Context->m_IE  = 0;
    p_Context->m_IME = false;
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
    if (GABLE_IsInterruptMasterEnabled(p_Engine))
    {
        // Check if any interrupts are requested.
        for (Uint8 i = 0; i < GABLE_INT_COUNT; i++)
        {
            // Check if the interrupt is enabled and requested.
            if (
                GABLE_bit(p_Context->m_IE, i) == true &&
                GABLE_bit(p_Context->m_IF, i) == true
            )
            {
                // Acknowledge the interrupt request by clearing the interrupt request flag and
                // the interrupt master enable flag.
                //
                // Disabling the interrupt master enable flag will prevent any further interrupts
                // from being serviced until the current interrupt handler returns.
                GABLE_clearbit(p_Context->m_IF, i);
                p_Context->m_IME = false;

                // Call the interrupt handler.
                if (p_Context->m_Handlers[i] != NULL)
                {
                    return (p_Context->m_Handlers[i](p_Engine) == true) ? 1 : -1;
                }
            }
        }
    }

    // Return no interrupt serviced.
    return 0;
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

// Public Functions - High-Level Functions /////////////////////////////////////////////////////////

Bool GABLE_IsInterruptMasterEnabled (GABLE_Engine* p_Engine)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Pointer to the GABLE Engine interrupt context instance.
    // Return its interrupt master enable flag.
    return GABLE_GetInterruptContext(p_Engine)->m_IME;
}

Bool GABLE_IsInterruptEnabled (GABLE_Engine* p_Engine, GABLE_InterruptType p_Type)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");
    
    // Read the interrupt enable register.
    Uint8 l_IE = 0x00;
    GABLE_CycleReadByte(p_Engine, GABLE_HP_IE, &l_IE);

    // Return the interrupt enable flag for the specified interrupt type.
    return GABLE_bit(l_IE, p_Type);
}

Bool GABLE_IsInterruptRequested (GABLE_Engine* p_Engine, GABLE_InterruptType p_Type)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");
    
    // Read the interrupt flag register.
    Uint8 l_IF = 0x00;
    GABLE_CycleReadByte(p_Engine, GABLE_HP_IF, &l_IF);

    // Return the interrupt request flag for the specified interrupt type.
    return GABLE_bit(l_IF, p_Type);
}

void GABLE_RequestInterrupt (GABLE_Engine* p_Engine, GABLE_InterruptType p_Type)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Pointer to the GABLE Engine interrupt context instance.
    // Set the interrupt request flag for the specified interrupt type.
    GABLE_setbit(GABLE_GetInterruptContext(p_Engine)->m_IF, p_Type);
}

Bool GABLE_ReturnFromInterrupt (GABLE_Engine* p_Engine)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Pointer to the GABLE Engine interrupt context instance.
    // Set the interrupt master enable flag.
    GABLE_GetInterruptContext(p_Engine)->m_IME = true;

    // Return success.
    return true;
}

void GABLE_CancelInterrupt (GABLE_Engine* p_Engine, GABLE_InterruptType p_Type)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Pointer to the GABLE Engine interrupt context instance.
    // Clear the interrupt request flag for the specified interrupt type.
    GABLE_clearbit(GABLE_GetInterruptContext(p_Engine)->m_IF, p_Type);
}

void GABLE_SetInterruptMasterEnable (GABLE_Engine* p_Engine, Bool p_Enable)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Pointer to the GABLE Engine interrupt context instance.
    // Set the interrupt master enable flag.
    GABLE_GetInterruptContext(p_Engine)->m_IME = p_Enable;
}

void GABLE_SetInterruptEnable (GABLE_Engine* p_Engine, GABLE_InterruptType p_Type, Bool p_Enable)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");
    
    // Read the interrupt enable register.
    Uint8 l_IE = 0x00;
    GABLE_CycleReadByte(p_Engine, GABLE_HP_IE, &l_IE);

    // Set the interrupt enable flag for the specified interrupt type.
    GABLE_changebit(l_IE, p_Type, p_Enable);

    // Write back to the interrupt enable register.
    GABLE_CycleWriteByte(p_Engine, GABLE_HP_IE, l_IE);
}

void GABLE_SetInterruptRequested (GABLE_Engine* p_Engine, GABLE_InterruptType p_Type, Bool p_Request)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Read the interrupt flag register.
    Uint8 l_IF = 0x00;
    GABLE_CycleReadByte(p_Engine, GABLE_HP_IF, &l_IF);

    // Set the interrupt request flag for the specified interrupt type.
    GABLE_changebit(l_IF, p_Type, p_Request);

    // Write back to the interrupt flag register.
    GABLE_CycleWriteByte(p_Engine, GABLE_HP_IF, l_IF);
}

void GABLE_SetInterruptHandler (GABLE_Engine* p_Engine, GABLE_InterruptType p_Type, GABLE_InterruptHandler p_Handler)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Pointer to the GABLE Engine interrupt context instance.
    // Set the interrupt handler for the specified interrupt type.
    GABLE_GetInterruptContext(p_Engine)->m_Handlers[p_Type] = p_Handler;
}
