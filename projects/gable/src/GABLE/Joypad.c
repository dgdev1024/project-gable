/**
 * @file GABLE/Joypad.c
 */

#include <GABLE/Engine.h>
#include <GABLE/InterruptContext.h>
#include <GABLE/Joypad.h>

// Joypad Component Structure //////////////////////////////////////////////////////////////////////

typedef struct GABLE_Joypad
{
    GABLE_Engine*           m_Engine;                   ///< @brief A pointer to the parent GABLE Engine instance.
    Bool                    m_SelectedButtons;          ///< @brief `true` if the joypad buttons are mapped to the low nibble of the `JOYP` register; `false` if not.
    Bool                    m_SelectedDirectionalPad;   ///< @brief `true` if the joypad directional pad is mapped to the low nibble of the `JOYP` register; `false` if not.
    Uint8                   m_Buttons;                  ///< @brief The current state of the joypad buttons.
    Uint8                   m_DirectionalPad;           ///< @brief The current state of the joypad directional pad.
} GABLE_Joypad;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_Joypad* GABLE_CreateJoypad (GABLE_Engine* p_Engine)
{

    GABLE_Joypad* l_Joypad = GABLE_calloc(1, GABLE_Joypad);
    GABLE_pexpect(l_Joypad, "Failed to allocate memory for the joypad component");

    l_Joypad->m_Engine = p_Engine;

    return l_Joypad;

}

void GABLE_DestroyJoypad (GABLE_Joypad* p_Joypad)
{
    if (p_Joypad != NULL)
    {
        p_Joypad->m_Engine = NULL;
        GABLE_free(p_Joypad);
    }
}

void GABLE_SelectButtons (GABLE_Joypad* p_Joypad, Bool p_Selected)
{
    GABLE_pexpect(p_Joypad != NULL, "Joypad component is NULL");

    p_Joypad->m_SelectedButtons = p_Selected;
    if (p_Selected == true) { p_Joypad->m_SelectedDirectionalPad = false; }
}

void GABLE_SelectDirectionalPad (GABLE_Joypad* p_Joypad, Bool p_Selected)
{
    GABLE_pexpect(p_Joypad != NULL, "Joypad component is NULL");

    p_Joypad->m_SelectedDirectionalPad = p_Selected;
    if (p_Selected == true) { p_Joypad->m_SelectedButtons = false; }
}

void GABLE_PressButton (GABLE_Joypad* p_Joypad, GABLE_JoypadButton p_Button)
{
    GABLE_pexpect(p_Joypad != NULL, "Joypad component is NULL");

    // Is the button pressed a face button or the directional pad?
    Bool l_IsDirectionalPad = (GABLE_bit(p_Button, 2) != 0);

    // Keep track of whether or not the joypad interrupt should be requested.
    Bool l_RequestInterrupt = false;

    if (l_IsDirectionalPad == true)
    {
        // Get the old state of the directional pad.
        Uint8 l_OldDirectionalPad = p_Joypad->m_DirectionalPad;

        // Update the correct bit in the directional pad state. Remember that a button's bit is
        // cleared when that button is pressed.
        GABLE_clearbit(p_Joypad->m_DirectionalPad, (p_Button & 0b11));

        // Determine if the interrupt should be requested.
        l_RequestInterrupt = (
            p_Joypad->m_SelectedDirectionalPad == true &&
            GABLE_bit(l_OldDirectionalPad, (p_Button & 0b11)) != 0 &&
            GABLE_bit(p_Joypad->m_DirectionalPad, (p_Button & 0b11)) == 0
        );
    }
    else
    {
        // Repeat the same process as above, but for the face buttons.
        Uint8 l_OldButtons = p_Joypad->m_Buttons;
        GABLE_clearbit(p_Joypad->m_Buttons, (p_Button & 0b11));
        l_RequestInterrupt = (
            p_Joypad->m_SelectedButtons == true &&
            GABLE_bit(l_OldButtons, (p_Button & 0b11)) != 0 &&
            GABLE_bit(p_Joypad->m_Buttons, (p_Button & 0b11)) == 0
        );
    }

    // If the interrupt should be requested, do so.
    if (l_RequestInterrupt == true)
    {
        GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Joypad->m_Engine), GABLE_INT_JOYPAD);
    }
}

void GABLE_ReleaseButton (GABLE_Joypad* p_Joypad, GABLE_JoypadButton p_Button)
{
    GABLE_pexpect(p_Joypad != NULL, "Joypad component is NULL");

    // Is the button released a face button or the directional pad?
    Bool l_IsDirectionalPad = (GABLE_bit(p_Button, 2) != 0);

    // Releasing a button never triggers an interrupt.

    if (l_IsDirectionalPad == true)
    {
        // Update the correct bit in the directional pad state. Remember that a button's bit is
        // set when that button is released.
        GABLE_setbit(p_Joypad->m_DirectionalPad, (p_Button & 0b11));
    }
    else
    {
        // Repeat the same process as above, but for the face buttons.
        GABLE_setbit(p_Joypad->m_Buttons, (p_Button & 0b11));
    }
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadJOYP (const GABLE_Joypad* p_Joypad)
{
    GABLE_pexpect(p_Joypad != NULL, "Joypad component is NULL");

    // Set up the value to be returned.
    Uint8 l_JOYP = 0b00000000;

    // Get the watch state of the face buttons and directional pad.
    // - If the face buttons are selected, then clear bit 5; otherwise, set it.
    // - If the directional pad is selected, then clear bit 4; otherwise, set it.
    GABLE_changebit(l_JOYP, 5, (p_Joypad->m_SelectedButtons == false));
    GABLE_changebit(l_JOYP, 4, (p_Joypad->m_SelectedDirectionalPad == false));

    // If the face buttons are selected, then copy the face button state to bits 3-0.
    // Otherwise, copy the directional pad state to bits 3-0.
    if (p_Joypad->m_SelectedButtons == true)
    {
        l_JOYP |= (p_Joypad->m_Buttons & 0b00001111);
    }
    else
    {
        l_JOYP |= (p_Joypad->m_DirectionalPad & 0b00001111);
    }

    return l_JOYP;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void GABLE_WriteJOYP (GABLE_Joypad* p_Joypad, Uint8 p_Value)
{
    GABLE_pexpect(p_Joypad != NULL, "Joypad component is NULL");
    
    // The low-nibble of the JOYP register is read-only.

    // The high-nibble of the JOYP register is used to select the buttons or the directional pad.
    // - If bit 5 is clear, then the face buttons are selected.
    // - If bit 4 is clear, then the directional pad is selected.
    p_Joypad->m_SelectedButtons = (GABLE_bit(p_Value, 5) == 0);
    p_Joypad->m_SelectedDirectionalPad = (GABLE_bit(p_Value, 4) == 0);
}