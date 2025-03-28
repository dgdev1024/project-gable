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
 * clock (RTC), a data store, and a network interface, which are not present in the original Game
 * Boy hardware.
 * 
 * - The Real-Time Clock (RTC) is a simple clock which keeps track of the current time in hours and
 *   minutes. It can be read from and written to by the game software, and can be used to implement
 *   time-based events in games.
 * 
 * - The Data Store is a large, variable-sized, banked memory area which is used to store game
 *   assets, such as graphics, sound effects, music, and level data.
 * 
 * - The Network Interface replaces the Game Boy's serial port with a simple network interface which
 *   allows for the transfer of data between the game software and another source, such as a server
 *   or another game instance.
 * 
 * It's important to note that the GABLE Engine is NOT an emulator. It is not intended to be used
 * for the emulation of Gameboy or Gameboy Color ROMS. Instead, it is intended to be used for the
 * creation of games which look, feel and sound like Gameboy games, but which are written in C (or
 * C++, or any other language which can interface with C) and can run on modern hardware.
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

/**
 * @brief      A function pointer used for simulating the "CPU"'s `RST` instruction.
 */
typedef Bool (*GABLE_RestartVector) (GABLE_Engine*);

// Registers Structure /////////////////////////////////////////////////////////////////////////////

/**
 * @brief      This context structure simulates the Game Boy CPU's registers.
 */
typedef struct GABLE_Registers
{
    Uint8  m_A;     ///< @brief The CPU's 8-bit accumulator register.
    Uint8  m_F;     ///< @brief The CPU's 8-bit flags register.
    Uint8  m_B;     ///< @brief One of the CPU's 8-bit general-purpose registers.
    Uint8  m_C;     ///< @brief One of the CPU's 8-bit general-purpose registers.
    Uint8  m_D;     ///< @brief One of the CPU's 8-bit general-purpose registers.
    Uint8  m_E;     ///< @brief One of the CPU's 8-bit general-purpose registers.
    Uint8  m_H;     ///< @brief One of the CPU's 8-bit general-purpose registers.
    Uint8  m_L;     ///< @brief One of the CPU's 8-bit general-purpose registers.
    Uint16 m_SP;    ///< @brief The CPU's 16-bit stack pointer register.
    Uint8  m_RST;   ///< @brief An internal register used to store the current restart vector.
} GABLE_Registers;

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
 * @brief Makes the specified GABLE Engine the current engine.
 * 
 * This function sets the specified GABLE Engine as the current engine, which means that all
 * subsequent calls to instruction simulation functions, and other shortform functions (`G_*`), will
 * use the specified engine.
 * 
 * @param p_Engine A pointer to the GABLE Engine structure.
 */
void GABLE_MakeEngineCurrent (GABLE_Engine* p_Engine);

/**
 * @brief Gets the current GABLE Engine instance.
 * 
 * @return A pointer to the current GABLE Engine instance, or `NULL` if no engine is set.
 */
GABLE_Engine* GABLE_GetCurrentEngine ();

/**
 * @brief Checks if a GABLE Engine is currently set as the current engine used for shortform functions.
 * 
 * @return `true` if a GABLE Engine instance is currently set, `false` otherwise.
 */
Bool GABLE_IsCurrentEngineSet ();

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
 * @brief      Pushes a value at the current stack pointer in the GABLE Engine's memory map.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Value          The value to push.
 * 
 * @return     `true` if the value was pushed successfully; `false` otherwise.
 */
Bool GABLE_PushWord (GABLE_Engine* p_Engine, Uint16 p_Value);

/**
 * @brief      Pops a value from the current stack pointer in the GABLE Engine's memory map.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Value          A pointer to the variable to store the popped value.
 * 
 * @return     `true` if the value was popped successfully; `false` otherwise.
 */
Bool GABLE_PopWord (GABLE_Engine* p_Engine, Uint16* p_Value);

/**
 * @brief      Reads a byte from the specified 8-bit register in the GABLE Engine's CPU.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Register       The register to read from.
 * @param      p_Value          A pointer to the variable to store the read byte.
 * 
 * @return     `true` if the register was read from successfully; `false` otherwise, or if the
 *             register is not an 8-bit register.
 */
Bool GABLE_ReadByteRegister (GABLE_Engine* p_Engine, GABLE_RegisterType p_Register, Uint8* p_Value);

/**
 * @brief      Reads a word from the specified 16-bit register pair in the GABLE Engine's CPU.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Register       The register to read from.
 * @param      p_Value          A pointer to the variable to store the read word.
 * 
 * @return     `true` if the register was read from successfully; `false` otherwise, or if the
 *             register is not a 16-bit register pair or the stack pointer.
 */
Bool GABLE_ReadWordRegister (GABLE_Engine* p_Engine, GABLE_RegisterType p_Register, Uint16* p_Value);

/**
 * @brief      Writes a byte to the specified 8-bit register in the GABLE Engine's "CPU".
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Register       The register to write to.
 * @param      p_Value          The byte to write.
 * 
 * @return     `true` if the register was written to successfully; `false` otherwise, or if the
 *             register is not an 8-bit register.
 */
Bool GABLE_WriteByteRegister (GABLE_Engine* p_Engine, GABLE_RegisterType p_Register, Uint8 p_Value);

/**
 * @brief      Writes a word to the specified 16-bit register pair in the GABLE Engine's "CPU".
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Register       The register to write to.
 * @param      p_Value          The word to write.
 * 
 * @return     `true` if the register was written to successfully; `false` otherwise, or if the
 *             register is not a 16-bit register pair or the stack pointer.
 */
Bool GABLE_WriteWordRegister (GABLE_Engine* p_Engine, GABLE_RegisterType p_Register, Uint16 p_Value);

/**
 * @brief      Retrieves the value of the specified flag in the GABLE Engine's "CPU" flags register.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Flag           The flag to retrieve the value of.
 * 
 * @return     The value of the specified flag in the "CPU" flags register.
 */
Bool GABLE_GetFlag (GABLE_Engine* p_Engine, GABLE_FlagType p_Flag);

/**
 * @brief      Sets the value of the specified flag in the GABLE Engine's "CPU" flags register.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Flag           The flag to set the value of.
 * @param      p_Value          The value to set the flag to.
 */
void GABLE_SetFlag (GABLE_Engine* p_Engine, GABLE_FlagType p_Flag, Bool p_Value);

/**
 * @brief      Sets the flags of the GABLE Engine's "CPU" flags register.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Z              The value to set the Zero flag to.
 * @param      p_N              The value to set the Subtract flag to.
 * @param      p_H              The value to set the Half-Carry flag to.
 * @param      p_C              The value to set the Carry flag to.
 */
void GABLE_SetFlags (GABLE_Engine* p_Engine, Bool p_Z, Bool p_N, Bool p_H, Bool p_C);

/**
 * @brief      Gets the number of cycles elapsed on the GABLE Engine.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     The number of cycles elapsed on the GABLE Engine.
 */
Uint64 GABLE_GetCycleCount (const GABLE_Engine* p_Engine);

/**
 * @brief      Sets a handler function to call when the `RST` instruction is simulated.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * @param      p_RST     The restart vector to set the handler for (0x00 - 0x07).
 * @param      p_Handler A pointer to the handler function to call for the restart vector.
 */
void GABLE_SetRestartVectorHandler (GABLE_Engine* p_Engine, Uint8 p_RST, GABLE_RestartVector p_Handler);

/**
 * @brief      When the `RST` instruction is simulated, sets the next restart vector to call.
 *             The handler function is not called immediately; it is called at the end of the next
 *             cycle.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * @param      p_RST     The restart vector to call (0x00 - 0x07).
 * 
 * @return     `true` if the restart vector was set successfully; `false` otherwise.
 */
Bool GABLE_CallRestartVector (GABLE_Engine* p_Engine, Uint8 p_RST);

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

// Public Functions - User Data ////////////////////////////////////////////////////////////////////

/**
 * @brief      Gets the user data associated with the GABLE Engine.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the user data associated with the GABLE Engine.
 */
void* GABLE_GetUserdata (GABLE_Engine* p_Engine);

/**
 * @brief      Sets the user data associated with the GABLE Engine.
 * 
 * @param      p_Engine    A pointer to the GABLE Engine instance.
 * @param      p_Userdata  A pointer to the user data to associate with the GABLE Engine.
 */
void GABLE_SetUserdata (GABLE_Engine* p_Engine, void* p_Userdata);

// Helper Macros ///////////////////////////////////////////////////////////////////////////////////

/**
 * @def        GABLE_CycleRead*(p_Engine, p_Address, p_Value)
 * 
 * @brief      Reads from the specified address in the GABLE Engine's memory map, then cycles
 *             the engine a number of times equal to the number of bytes read.
 * 
 * Every time GABLE's memory map is accessed (read from or written to), the engine must be cycled
 * by a certain number of cycles to simulate the time it takes for the CPU to access the memory,
 * which is 1 cycle for every byte read or written.
 * 
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Address        The address to read from.
 * @param      p_Value          A pointer to the variable to store the read value.
 * 
 * @return     `true` if both the memory map was read from successfully and the engine cycled
 *             successfully; `false` otherwise.
 */
#define GABLE_CycleReadByte(p_Engine, p_Address, p_Value) \
    (GABLE_ReadByte(p_Engine, p_Address, p_Value) && GABLE_CycleEngine(p_Engine, 1))
#define GABLE_CycleReadWord(p_Engine, p_Address, p_Value) \
    (GABLE_ReadWord(p_Engine, p_Address, p_Value) && GABLE_CycleEngine(p_Engine, 2))

/**
 * @def        GABLE_CycleWrite*(p_Engine, p_Address, p_Value)
 *
 * @brief      Writes to the specified address in the GABLE Engine's memory map, then cycles
 *             the engine a number of times equal to the number of bytes written.
 *
 * Every time GABLE's memory map is accessed (read from or written to), the engine must be cycled
 * by a certain number of cycles to simulate the time it takes for the CPU to access the memory,
 * which is 1 cycle for every byte read or written.
 *
 * @param      p_Engine         A pointer to the GABLE Engine instance.
 * @param      p_Address        The address to write to.
 * @param      p_Value          The value to write.
 *
 * @return     `true` if both the memory map was written to successfully and the engine cycled
 *             successfully; `false` otherwise.
 */
#define GABLE_CycleWriteByte(p_Engine, p_Address, p_Value) \
    (GABLE_WriteByte(p_Engine, p_Address, p_Value) && GABLE_CycleEngine(p_Engine, 1))
#define GABLE_CycleWriteWord(p_Engine, p_Address, p_Value) \
    (GABLE_WriteWord(p_Engine, p_Address, p_Value) && GABLE_CycleEngine(p_Engine, 2))
