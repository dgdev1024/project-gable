/**
 * @file     GABLE/RAM.h
 * @brief    Contains the GABLE Engine's RAM structure and public functions.
 * 
 * The GABLE Engine's RAM component seeks to provide a centralized location for storing and
 * retrieving game data, such as variables, flags, and other runtime information. It is a fixed-size
 * memory area which is used to store game data in a structured and organized manner. The RAM
 * component consists of the following buffers:
 * 
 * - `WRAM` (Working RAM) - The working RAM buffer, which is used to store general-purpose data
 *   during the execution of the game software. The working RAM buffer consists of up to 256 banks
 *   of 4 KB each, for a total of 1 MB of working RAM. The current WRAM bank can be selected using
 *   the `SVBK` hardware register. WRAM Bank 0 is always accessible to the software, mapped to the
 *  `0xC000` to `0xCFFF` memory region. The current WRAM bank is mapped to the `0xD000` to `0xDFFF`
 *  memory region.
 * 
 * - `SRAM` (Static RAM, or Save RAM) - The static RAM buffer, which is used to store persistent game
 *   data, such as save files, high scores, and other information that needs to be retained across
 *   power cycles - or, in case of the GABLE Engine, that can be saved to and loaded from a file.
 *   The static RAM buffer consists of up to 256 banks of 8 KB each, for a total of 2 MB of static
 *   RAM. The current SRAM bank can be selected using the `SSBK` hardware register. The current SRAM
 *   bank is always accessible to the software, mapped to the `0xA000` to `0xBFFF` memory region.
 * 
 * - `HRAM` (High RAM) - The high RAM buffer, which is used to store temporary data and flags during
 *   the execution of the game software. The high RAM buffer is a fixed-size 127-byte buffer that is
 *   always accessible to the software, mapped to the `0xFF80` to `0xFFFE` memory region.
 * 
 * The RAM component is responsible for the following hardware registers:
 * 
 * - `SVBK` (Switch VRAM Bank) - The switch VRAM bank register, which is used to select the current
 *   working RAM bank.
 * - `SSBK` (Switch SRAM Bank) - The switch SRAM bank register, which is used to select the current
 *   static RAM bank.
 */

#pragma once
#include <GABLE/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief The default number of working RAM banks in the RAM component.
 */
#define GABLE_RAM_DEFAULT_WRAM_BANKS 2

/**
 * @brief The default number of static RAM banks in the RAM component.
 */
#define GABLE_RAM_DEFAULT_SRAM_BANKS 1

/**
 * @brief The maximum number of WRAM and SRAM banks that can be allocated by the RAM component.
 */
#define GABLE_RAM_MAX_BANKS 256

/**
 * @brief The size of each working RAM bank in the RAM component, in bytes.
 */
#define GABLE_RAM_WRAM_BANK_SIZE 4096

/**
 * @brief The size of each static RAM bank in the RAM component, in bytes.
 */
#define GABLE_RAM_SRAM_BANK_SIZE 8192

/**
 * @brief The size of the high RAM buffer in the RAM component, in bytes.
 */
#define GABLE_RAM_HRAM_SIZE 127

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward declaration of the GABLE Engine structure.
 */
typedef struct GABLE_Engine GABLE_Engine;

/**
 * @brief The GABLE Engine's RAM structure.
 */
typedef struct GABLE_RAM GABLE_RAM;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief      Creates a new GABLE Engine RAM instance.
 * 
 * @return     A pointer to the new GABLE Engine RAM instance.
 */
GABLE_RAM* GABLE_CreateRAM ();

/**
 * @brief      Resets the contents of a GABLE Engine RAM instance.
 * 
 * @param      p_RAM  A pointer to the GABLE Engine RAM instance to reset.
 */
void GABLE_ResetRAM (GABLE_RAM* p_RAM);

/**
 * @brief      Destroys a GABLE Engine RAM instance.
 * 
 * @param      p_RAM  A pointer to the GABLE Engine RAM instance to destroy.
 */
void GABLE_DestroyRAM (GABLE_RAM* p_RAM);

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

/**
 * @brief      Reads a byte from the specified address in the current WRAM bank.
 * 
 * @param      p_RAM     A pointer to the GABLE Engine RAM instance.
 * @param      p_Address The address to read from.
 * @param      p_Value   A pointer to the variable to store the read byte.
 * 
 * @return     `true` if the WRAM was read from successfully; `false` otherwise.
 */
Bool GABLE_ReadWRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8* p_Value);

/**
 * @brief      Reads a byte from the specified address in the current SRAM bank.
 * 
 * @param      p_RAM     A pointer to the GABLE Engine RAM instance.
 * @param      p_Address The address to read from.
 * @param      p_Value   A pointer to the variable to store the read byte.
 * 
 * @return     `true` if the SRAM was read from successfully; `false` otherwise.
 */
Bool GABLE_ReadSRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8* p_Value);

/**
 * @brief      Reads a byte from the specified address in the HRAM buffer.
 * 
 * @param      p_RAM     A pointer to the GABLE Engine RAM instance.
 * @param      p_Address The address to read from.
 * @param      p_Value   A pointer to the variable to store the read byte.
 * 
 * @return     `true` if the HRAM was read from successfully; `false` otherwise.
 */
Bool GABLE_ReadHRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8* p_Value);

/**
 * @brief      Writes a byte to the specified address in the current WRAM bank.
 * 
 * @param      p_RAM     A pointer to the GABLE Engine RAM instance.
 * @param      p_Address The address to write to.
 * @param      p_Value   The byte value to write.
 * 
 * @return     `true` if the WRAM was written to successfully; `false` otherwise.
 */
Bool GABLE_WriteWRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8 p_Value);

/**
 * @brief      Writes a byte to the specified address in the current SRAM bank.
 * 
 * @param      p_RAM     A pointer to the GABLE Engine RAM instance.
 * @param      p_Address The address to write to.
 * @param      p_Value   The byte value to write.
 * 
 * @return     `true` if the SRAM was written to successfully; `false` otherwise.
 */
Bool GABLE_WriteSRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8 p_Value);

/**
 * @brief      Writes a byte to the specified address in the HRAM buffer.
 * 
 * @param      p_RAM     A pointer to the GABLE Engine RAM instance.
 * @param      p_Address The address to write to.
 * @param      p_Value   The byte value to write.
 * 
 * @return     `true` if the HRAM was written to successfully; `false` otherwise.
 */
Bool GABLE_WriteHRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8 p_Value);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief      Gets the value of the `SVBK` (Switch VRAM Bank) register.
 * 
 * @param      p_RAM  A pointer to the GABLE Engine RAM instance.
 * 
 * @return     The value of the `SVBK` register.
 */
Uint8 GABLE_ReadSVBK (const GABLE_RAM* p_RAM);

/**
 * @brief      Gets the value of the `SSBK` (Switch SRAM Bank) register.
 * 
 * @param      p_RAM  A pointer to the GABLE Engine RAM instance.
 * 
 * @return     The value of the `SSBK` register.
 */
Uint8 GABLE_ReadSSBK (const GABLE_RAM* p_RAM);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief      Sets the value of the `SVBK` (Switch VRAM Bank) register.
 * 
 * @param      p_RAM    A pointer to the GABLE Engine RAM instance.
 * @param      p_Value  The new value of the `SVBK` register.
 */
void GABLE_WriteSVBK (GABLE_RAM* p_RAM, Uint8 p_Value);

/**
 * @brief      Sets the value of the `SSBK` (Switch SRAM Bank) register.
 * 
 * @param      p_RAM    A pointer to the GABLE Engine RAM instance.
 * @param      p_Value  The new value of the `SSBK` register.
 */
void GABLE_WriteSSBK (GABLE_RAM* p_RAM, Uint8 p_Value);

// Public Functions - High-Level Functions /////////////////////////////////////////////////////////

/**
 * @brief      Loads a save file into save RAM.
 * 
 * The file's size must be a multiple of the size of a single SRAM bank, and cannot exceed the
 * maximum size of the save RAM buffer (256 banks * 8 KB = 2 MB). The SRAM buffer will be resized
 * to accommodate the size of the save file, if necessary.
 * 
 * @param      p_Engine    A pointer to the GABLE Engine instance.
 * @param      p_FilePath  The path to the save file to load.
 * 
 * @return     `true` if the save file was loaded successfully; `false` otherwise.
 */
Bool GABLE_LoadSRAMFile (GABLE_Engine* p_Engine, const Char* p_FilePath);

/**
 * @brief      Saves the current save RAM to a file.
 * 
 * The save file will be created or overwritten with the contents of the current save RAM buffer in
 * save RAM. The save file will be the size of the current save RAM buffer, and will be written in
 * binary format.
 * 
 * @param      p_Engine    A pointer to the GABLE Engine instance.
 * @param      p_FilePath  The path to the save file to save.
 * 
 * @return     `true` if the save file was saved successfully; `false` otherwise.
 */
Bool GABLE_SaveSRAMFile (GABLE_Engine* p_Engine, const Char* p_FilePath);

Uint8 GABLE_GetWRAMBankCount (GABLE_Engine* p_Engine);
Uint8 GABLE_GetSRAMBankCount (GABLE_Engine* p_Engine);
void GABLE_SetWRAMBankCount (GABLE_Engine* p_Engine, Uint8 p_BankCount);
void GABLE_SetSRAMBankCount (GABLE_Engine* p_Engine, Uint8 p_BankCount);
