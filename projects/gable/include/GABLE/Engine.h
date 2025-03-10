/**
 * @file       GABLE/Engine.h
 * @brief      Contains the GABLE Engine's core context structure and function prototypes.
 * 
 * The "GAmeBoy-Like Engine" (or "GABLE Engine" for short) is a game engine which allows for the
 * developement of games which mimic, as accurately as possible, the look, feel, sound and behaviour
 * of games developed for the Nintendo Game Boy and Game Boy Color. This is achieved by emulating
 * just enough of the Game Boy's hardware - the joypad, timer, pixel-processing unit (PPU),
 * audio-processing unit (APU), memory map and CPU interrupts - to allow for the development of
 * "Game Boy-like" games on PC and other platforms.
 * 
 * The GABLE Engine also includes some of its own emulated hardware components, such as a real-time
 * clock (RTC) and a data store, which are not present in the original Game Boy hardware:
 * 
 * - The Real-Time Clock (RTC) is a simple clock which keeps track of the current time in hours and
 *   minutes. It can be read from and written to by the game software, and can be used to implement
 *   time-based events in games.
 * 
 * - The Data Store is a large, variable-sized, banked memory area which is used to store game
 *   assets, such as graphics, sound effects, music, and level data.
 */

#pragma once
#include <GABLE/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief      Forward declaration of the GABLE Interrupt Context structure.
 */
typedef struct GABLE_InterruptContext GABLE_InterruptContext;

/**
 * @brief      Forward declaration of the GABLE Timer structure.
 */
typedef struct GABLE_Timer GABLE_Timer;

/**
 * @brief      Forward declaration of the GABLE Real-Time Clock structure.
 */
typedef struct GABLE_Realtime GABLE_Realtime;

/**
 * @brief      Forward declaration of the GABLE Data Store structure.
 */
typedef struct GABLE_DataStore GABLE_DataStore;

/**
 * @brief      Forward declaration of the GABLE RAM structure.
 */
typedef struct GABLE_RAM GABLE_RAM;

/**
 * @brief      Forward declaration of the GABLE Engine APU structure.
 */
typedef struct GABLE_APU GABLE_APU;

/**
 * @brief      Forward declaration of the GABLE Engine PPU structure.
 */
typedef struct GABLE_PPU GABLE_PPU;

/**
 * @brief      Forward declaration of the GABLE Engine Joypad structure.
 */
typedef struct GABLE_Joypad GABLE_Joypad;

/**
 * @brief      Forward declaration of the GABLE Engine Network structure.
 */
typedef struct GABLE_NetworkContext GABLE_NetworkContext;

/**
 * @brief      The GABLE Engine's core context structure.
 */
typedef struct GABLE_Engine GABLE_Engine;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief      Creates a new GABLE Engine instance.
 *
 * @return     A pointer to the new GABLE Engine instance.
 */
GABLE_Engine* GABLE_CreateEngine ();

/**
 * @brief      Destroys a GABLE Engine instance.
 *
 * @param      p_Engine  A pointer to the GABLE Engine instance to destroy.
 */
void GABLE_DestroyEngine (GABLE_Engine* p_Engine);

/**
 * @brief      Elapses the given number of cycles on the GABLE Engine.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Cycles         The number of cycles to elapse.
 *
 * @return     `true` if the engine's components ticked with no errors; `false` otherwise. 
 */
Bool GABLE_CycleEngine (GABLE_Engine* p_Engine, Count p_Cycles);

/**
 * @brief      Reads a byte from the specified address in the GABLE Engine's memory map.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Address        The address to read from.
 * @param      p_Value          A pointer to the variable to store the read byte.
 * 
 * @return     `true` if the memory map was read from successfully; `false` otherwise.
 */
Bool GABLE_ReadByte (GABLE_Engine* p_Engine, Uint16 p_Address, Uint8* p_Value);

/**
 * @brief      Reads a word from the specified address in the GABLE Engine's memory map.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Address        The address to read from.
 * @param      p_Value          A pointer to the variable to store the read word.
 * 
 * @return     `true` if the memory map was read from successfully; `false` otherwise.
 */
Bool GABLE_ReadWord (GABLE_Engine* p_Engine, Uint16 p_Address, Uint16* p_Value);

/**
 * @brief      Writes a byte to the specified address in the GABLE Engine's memory map.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Address        The address to write to.
 * @param      p_Value          The byte to write.
 * 
 * @return     `true` if the memory map was written to successfully; `false` otherwise.
 */
Bool GABLE_WriteByte (GABLE_Engine* p_Engine, Uint16 p_Address, Uint8 p_Value);

/**
 * @brief      Writes a word to the specified address in the GABLE Engine's memory map.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Address        The address to write to.
 * @param      p_Value          The word to write.
 * 
 * @return     `true` if the memory map was written to successfully; `false` otherwise.
 */
Bool GABLE_WriteWord (GABLE_Engine* p_Engine, Uint16 p_Address, Uint16 p_Value);

/**
 * @brief      Gets the number of cycles elapsed on the GABLE Engine.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     The number of cycles elapsed on the GABLE Engine.
 */
Uint64 GABLE_GetCycleCount (const GABLE_Engine* p_Engine);

// Public Functions - Component Getters ////////////////////////////////////////////////////////////

/**
 * @brief      Gets the GABLE Engine's interrupt context instance.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the GABLE Engine's interrupt context instance.
 */
GABLE_InterruptContext* GABLE_GetInterruptContext (GABLE_Engine* p_Engine);

/**
 * @brief      Gets the GABLE Engine's timer instance.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the GABLE Engine's timer instance.
 */
GABLE_Timer* GABLE_GetTimer (GABLE_Engine* p_Engine);

/**
 * @brief      Gets the GABLE Engine's real-time clock instance.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the GABLE Engine's real-time clock instance.
 */
GABLE_Realtime* GABLE_GetRealtime (GABLE_Engine* p_Engine);

/**
 * @brief      Gets the GABLE Engine's data store instance.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the GABLE Engine's data store instance.
 */
GABLE_DataStore* GABLE_GetDataStore (GABLE_Engine* p_Engine);

/**
 * @brief      Gets the GABLE Engine's RAM instance.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the GABLE Engine's RAM instance.
 */
GABLE_RAM* GABLE_GetRAM (GABLE_Engine* p_Engine);

/**
 * @brief      Gets the GABLE Engine's APU instance.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the GABLE Engine's APU instance.
 */
GABLE_APU* GABLE_GetAPU (GABLE_Engine* p_Engine);

/**
 * @brief      Gets the GABLE Engine's PPU instance.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the GABLE Engine's PPU instance.
 */
GABLE_PPU* GABLE_GetPPU (GABLE_Engine* p_Engine);

/**
 * @brief      Gets the GABLE Engine's joypad instance.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the GABLE Engine's joypad instance.
 */
GABLE_Joypad* GABLE_GetJoypad (GABLE_Engine* p_Engine);

/**
 * @brief      Gets the GABLE Engine's network interface instance.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the GABLE Engine's network interface instance.
 */
GABLE_NetworkContext* GABLE_GetNetwork (GABLE_Engine* p_Engine);
