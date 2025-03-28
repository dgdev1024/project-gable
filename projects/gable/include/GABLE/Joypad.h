/**
 * @file     GABLE/Joypad.h
 * @brief    Contains the GABLE Engine's joypad component structure and public functions.
 * 
 * The GABLE Engine's joypad component simulates the Game Boy's internal joypad hardware. The joypad
 * component is responsible for reading the state of the Game Boy's buttons and directional pad, and
 * updating the joypad register in the memory map.
 * 
 * The joypad component is responsible for the following hardware register:
 * 
 * - `JOYP` or `P1` (Joypad Register) - The joypad register, which is used to read the state of the
 *   Game Boy buttons and/or directional pad. The bits of this register control the following settings:
 *       - Bit 0: A Button / Right Directional Pad. Read-only.
 *       - Bit 1: B Button / Left Directional Pad. Read-only.
 *       - Bit 2: Select Button / Up Directional Pad. Read-only.
 *       - Bit 3: Start Button / Down Directional Pad. Read-only.
 *       - Bit 4: Select D-Pad.
 *       - Bit 5: Select Buttons.
 *       - Bits 6-7: Not used.
 *       - Note that the bits of this register are inverted, with a bit set to 0 indicating that the
 *         corresponding button or directional pad is pressed.
 * 
 * The joypad component is responsible for the following hardware interrupt:
 * 
 * - `JOYPAD` (Joypad Interrupt) - The joypad interrupt, which is requested when a button or directional
 *   pad's pressed state changes, assuming that its group of buttons is being selected by the `JOYP`
 *   register. This interrupt is typically used to update the game's engine state in response to player
 *   input.
 */

#pragma once
#include <GABLE/Common.h>

// Joypad Button Enumeration ///////////////////////////////////////////////////////////////////////

/**
 * @brief Enumerates the eight buttons found on the Game Boy's joypad.
 */
typedef enum GABLE_JoypadButton
{
    GABLE_JB_A        = 0b000,            ///< @brief The A button.
    GABLE_JB_B        = 0b001,            ///< @brief The B button.
    GABLE_JB_SELECT   = 0b010,            ///< @brief The Select button.
    GABLE_JB_START    = 0b011,            ///< @brief The Start button.
    GABLE_JB_RIGHT    = 0b100,            ///< @brief The Right directional pad button.
    GABLE_JB_LEFT     = 0b101,            ///< @brief The Left directional pad button.
    GABLE_JB_UP       = 0b110,            ///< @brief The Up directional pad button.
    GABLE_JB_DOWN     = 0b111             ///< @brief The Down directional pad button.
} GABLE_JoypadButton;

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward-declaration of the GABLE Engine structure.
 */
typedef struct GABLE_Engine GABLE_Engine;

/**
 * @brief A forward-declaration of the GABLE Engine joypad structure.
 */
typedef struct GABLE_Joypad GABLE_Joypad;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief      Creates a new GABLE Engine joypad instance.
 * 
 * @param      p_Engine  A pointer to the parent GABLE Engine instance.
 * 
 * @return     A pointer to the new GABLE Engine joypad instance.
 */
GABLE_Joypad* GABLE_CreateJoypad (GABLE_Engine* p_Engine);

/**
 * @brief      Resets a GABLE Engine joypad instance.
 * 
 * @param      p_Joypad  A pointer to the GABLE Engine joypad instance to reset.
 */
void GABLE_ResetJoypad (GABLE_Joypad* p_Joypad);

/**
 * @brief      Destroys a GABLE Engine joypad instance.
 * 
 * @param      p_Joypad  A pointer to the GABLE Engine joypad instance to destroy.
 */
void GABLE_DestroyJoypad (GABLE_Joypad* p_Joypad);

/**
 * @brief      Presses a button on the GABLE Engine joypad.
 * 
 * @param      p_Engine  A pointer to the parent GABLE Engine instance.
 * @param      p_Button  The button to press.
 */
void GABLE_PressButton (GABLE_Engine* p_Engine, GABLE_JoypadButton p_Button);

/**
 * @brief      Releases a button on the GABLE Engine joypad.
 * 
 * @param      p_Engine  A pointer to the parent GABLE Engine instance.
 * @param      p_Button  The button to release.
 */
void GABLE_ReleaseButton (GABLE_Engine* p_Engine, GABLE_JoypadButton p_Button);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief      Gets the value of the `JOYP` (Joypad Register) register.
 * 
 * @param      p_Joypad  A pointer to the GABLE Engine joypad instance.
 * 
 * @return     The value of the `JOYP` register.
 */
Uint8 GABLE_ReadJOYP (const GABLE_Joypad* p_Joypad);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief      Sets the value of the `JOYP` (Joypad Register) register.
 * 
 * @param      p_Joypad  A pointer to the GABLE Engine joypad instance.
 * @param      p_Value   The new value of the `JOYP` register.
 */
void GABLE_WriteJOYP (GABLE_Joypad* p_Joypad, Uint8 p_Value);

// Public Functions - High-Level Functions /////////////////////////////////////////////////////////
