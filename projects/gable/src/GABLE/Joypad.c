/**
 * @file GABLE/Joypad.c
 */

#include <GABLE/Engine.h>
#include <GABLE/InterruptContext.h>
#include <GABLE/Joypad.h>

// Joypad Component Structure //////////////////////////////////////////////////////////////////////

typedef struct GABLE_Joypad
{
    GABLE_Engine*           m_Engine;                       ///< @brief A pointer to the parent GABLE Engine instance.
    Bool                    m_SelectedButtons;              ///< @brief `true` if the joypad buttons are mapped to the low nibble of the `JOYP` register; `false` if not.
    Bool                    m_SelectedDirectionalPad;       ///< @brief `true` if the joypad directional pad is mapped to the low nibble of the `JOYP` register; `false` if not.
    Bool                    m_States[8];                    ///< @brief The state of the joypad buttons and directional pad.
} GABLE_Joypad;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_Joypad* GABLE_CreateJoypad (GABLE_Engine* p_Engine)
{

    GABLE_Joypad* l_Joypad = GABLE_calloc(1, GABLE_Joypad);
    GABLE_pexpect(l_Joypad, "Failed to allocate memory for the joypad component");

    l_Joypad->m_Engine = p_Engine;

    return l_Joypad;

}

void GABLE_ResetJoypad (GABLE_Joypad* p_Joypad)
{
    GABLE_expect(p_Joypad != NULL, "Joypad component is NULL!");

    // The `JOYP` register is reset to 0xCF (0b11001111).
    p_Joypad->m_SelectedButtons = true;
    p_Joypad->m_SelectedDirectionalPad = true;
    memset(p_Joypad->m_States, 0x00, sizeof(p_Joypad->m_States));
}

void GABLE_DestroyJoypad (GABLE_Joypad* p_Joypad)
{
    if (p_Joypad != NULL)
    {
        p_Joypad->m_Engine = NULL;
        GABLE_free(p_Joypad);
    }
}

void GABLE_PressButton (GABLE_Engine* p_Engine, GABLE_JoypadButton p_Button)
{
    GABLE_pexpect(p_Engine != NULL, "Engine context is NULL");

    // Get the joypad component from the engine.
    GABLE_Joypad* l_Joypad = GABLE_GetJoypad(p_Engine);

    // Check to see if the button being pressed is a directional pad button.
    Bool l_IsDirectionalPadButton = (GABLE_bit(p_Button, 2) != 0);

    // Get the old state of the button.
    Bool l_Old = l_Joypad->m_States[p_Button & 0b111];

    // Change the state of the button to pressed.
    l_Joypad->m_States[p_Button & 0b111] = true;

    // Clear the bit in the appropriate button state.
    if (l_IsDirectionalPadButton == true)
    {
        // If the button was not pressed before, and the DPAD group is selected, then trigger the joypad interrupt.
        if (l_Joypad->m_SelectedDirectionalPad == true && l_Old == false)
        {
            GABLE_RequestInterrupt(p_Engine, GABLE_INT_JOYPAD);
        }
    }
    else
    {
        // If the button was not pressed before, and the BUTTON group is selected, then trigger the joypad interrupt.
        if (l_Joypad->m_SelectedButtons == true && l_Old == false)
        {
            GABLE_RequestInterrupt(p_Engine, GABLE_INT_JOYPAD);
        }
    }
}

void GABLE_ReleaseButton (GABLE_Engine* p_Engine, GABLE_JoypadButton p_Button)
{
    GABLE_pexpect(p_Engine != NULL, "Engine context is NULL");

    // Get the joypad component from the engine.
    GABLE_Joypad* l_Joypad = GABLE_GetJoypad(p_Engine);

    // Set the button state to released.
    // Releasing a button does not trigger an interrupt.
    l_Joypad->m_States[p_Button & 0b111] = false;
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadJOYP (const GABLE_Joypad* p_Joypad)
{
    GABLE_pexpect(p_Joypad != NULL, "Joypad component is NULL");

    // Set up the value to be returned.
    Uint8 l_JOYP = 0xCF;

    // Get the watch state of the face buttons and directional pad.
    // - If the face buttons are selected, then clear bit 5; otherwise, set it.
    // - If the directional pad is selected, then clear bit 4; otherwise, set it.
    GABLE_changebit(l_JOYP, 5, (p_Joypad->m_SelectedButtons == false));
    GABLE_changebit(l_JOYP, 4, (p_Joypad->m_SelectedDirectionalPad == false));

    // If the face buttons are selected, then copy the face button state to bits 3-0.
    // Otherwise, copy the directional pad state to bits 3-0.
    if (p_Joypad->m_SelectedButtons == true)
    {
        if (p_Joypad->m_States[GABLE_JB_START] == true)
            l_JOYP &= ~(1 << 3);
        if (p_Joypad->m_States[GABLE_JB_SELECT] == true)
            l_JOYP &= ~(1 << 2);
        if (p_Joypad->m_States[GABLE_JB_B] == true)
            l_JOYP &= ~(1 << 1);
        if (p_Joypad->m_States[GABLE_JB_A] == true)
            l_JOYP &= ~(1 << 0);
    }
    else
    {
        if (p_Joypad->m_States[GABLE_JB_DOWN] == true)
            l_JOYP &= ~(1 << 3);
        if (p_Joypad->m_States[GABLE_JB_UP] == true)
            l_JOYP &= ~(1 << 2);
        if (p_Joypad->m_States[GABLE_JB_LEFT] == true)
            l_JOYP &= ~(1 << 1);
        if (p_Joypad->m_States[GABLE_JB_RIGHT] == true)
            l_JOYP &= ~(1 << 0);
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
