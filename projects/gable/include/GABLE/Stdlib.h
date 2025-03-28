/**
 * @file  GABLE/Stdlib.h
 * @brief Provides a standard library interfacing with the GABLE Engine's "CPU" instruction set.
 */

#pragma once
#include <GABLE/Common.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Waits for the next vertical blank period (`VBLANK`) to occur.
 */
void G_WaitVBlank ();

/**
 * @brief If the `VBLANK` period is currently active, this function will wait until the period ends.
 */
void G_WaitAfterVBlank ();

/**
 * @brief Copies a block of memory from one location to another.
 * 
 * The destination address is stored in register `DE`, the source address is stored in register
 * `HL`, and the number of bytes to copy is stored in register `BC`. The function will copy the
 * specified number of bytes from the source address to the destination address, and then cycle the
 * engine for the number of cycles it took to copy the bytes.
 */
void G_CopyBytes ();

/**
 * @brief Clears the OAM (Object Attribute Memory) region of the VRAM.
 * 
 * This function sets all bytes in the OAM region to zero, effectively clearing any sprite
 * attributes that may have been set previously. This is typically done at the start of a new
 * frame to ensure that no old sprite data is left over.
 */
void G_ClearOAM ();

/**
 * @brief Updates the joypad state.
 * 
 * This function updates the current state of the joypad by reading the current and new key states
 * and updating the engine's internal state accordingly. The function takes two parameters:
 * 
 * @param p_CurKeys The location of the current keys byte in WRAM.
 * @param p_NewKeys The location of the new keys byte in WRAM.
 */
void G_UpdateKeys (const Uint16 p_CurKeys, const Uint16 p_NewKeys);
