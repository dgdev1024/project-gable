/**
 * @file     GABLE/DataStore.h
 * @brief    Contains the GABLE Engine's data store structure and public functions.
 * 
 * Normally, the ROM section of the Game Boy's memory map would be used to store instructions for
 * the CPU to execute, in addition to game assets. However, because games developed on the GABLE
 * Engine are run on modern PC (or game console) hardware, there are no GB instructions to emulate.
 * As such, the ROM section of the memory map is repurposed to map the GABLE Engine's data store.
 * 
 * The GABLE Engine's data store component seeks to provide a centralized location for storing and
 * retrieving game assets, such as graphics, sound effects, music, and level data. It is a large,
 * variable-sized, banked memory area which is used to store game assets in a structured and
 * organized manner. Each bank in the data store is 16 KB in size, and can be accessed by setting
 * the current bank number using the `DSBKH` and `DSBKL` registers.
 * 
 * The maximum number of banks that can be allocated by the data store is 65,536, which is the
 * maximum number of 16-bit values that can be stored in a 16-bit register. The data store, as such,
 * can store up to 1 GB of data (65,536 banks * 16 KB per bank = 1,048,576 KB = 1 GB).
 * 
 * The data store is responsible for the following hardware registers:
 * 
 * - `DSBKH` (Data Store Bank High) - The data store's bank high register, which contains the high
 *   byte of the current bank number.
 * - `DSBKL` (Data Store Bank Low) - The data store's bank low register, which contains the low byte
 *   of the current bank number.
 * 
 * The data store component maps to the following memory regions:
 * 
 * - `0x0000` to `0x3FFF` - Data Store Bank 0
 * - `0x4000` to `0x7FFF` - Data Store Bank XX, where XX is the current bank number set by the
 *   `DSBKH` and `DSBKL` registers.
 * 
 * The data store component should include functions for reading and writing data to and from the
 * data store, as well as functions for setting and getting the current bank number. Functions for
 * loading game assets from external files and other sources should also be included.
 */

#pragma once
#include <GABLE/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief The default number of banks in the data store.
 */
#define GABLE_DS_DEFAULT_BANK_COUNT  2

/**
 * @brief The maximum number of banks that can be allocated by the data store.
 */
#define GABLE_DS_MAX_BANK_COUNT      65536

/**
 * @brief The size of each bank in the data store, in bytes.
 */
#define GABLE_DS_BANK_SIZE           16384

/**
 * @brief The length of the name of a handle to a chunk of data in the data store.
 */
#define GABLE_DS_NAME_STRLEN         32

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief The GABLE Engine's data store structure.
 */
typedef struct GABLE_DataStore GABLE_DataStore;

// Data Handle Structure ///////////////////////////////////////////////////////////////////////////

/**
 * @brief A structure representing a handle to a chunk of data in the data store.
 */
typedef struct GABLE_DataHandle
{
    Char            m_Name[GABLE_DS_NAME_STRLEN];       ///< @brief A string identifying the data chunk.
    Uint16          m_Length;                           ///< @brief The length of the data chunk, in bytes.
    Uint16          m_Address;                          ///< @brief The address of the data chunk in the referenced bank.
    Uint8           m_BankHigh;                         ///< @brief The high byte of the number of the bank housing the data chunk.
    Uint8           m_BankLow;                          ///< @brief The low byte of the number of the bank housing the data chunk.
} GABLE_DataHandle;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief      Creates a new GABLE Engine data store instance.
 * 
 * @return     A pointer to the new GABLE Engine data store instance.
 */
GABLE_DataStore* GABLE_CreateDataStore ();

/**
 * @brief      Destroys a GABLE Engine data store instance.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance to destroy.
 */
void GABLE_DestroyDataStore (GABLE_DataStore* p_DataStore);

/**
 * @brief      Sets the number of banks in the data store, reallocating memory if necessary.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance.
 * @param      p_BankCount  The new number of banks to allocate.
 */
void GABLE_SetDataStoreBankCount (GABLE_DataStore* p_DataStore, Uint16 p_BankCount);

/**
 * @brief      Sets the current bank number of the data store.
 * 
 * @param      p_DataStore   A pointer to the GABLE Engine data store instance.
 * @param      p_BankNumber  The new current bank number.
 */
void GABLE_SetDataStoreBankNumber (GABLE_DataStore* p_DataStore, Uint16 p_BankNumber);

/**
 * @brief      Gets a data handle from the data store by name.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance.
 * @param      p_Name       The name of the data handle to retrieve.
 * 
 * @return     A pointer to the data handle with the specified name, or `NULL` if no such handle exists.
 */
const GABLE_DataHandle* GABLE_GetDataHandle (const GABLE_DataStore* p_DataStore, const Char* p_Name);

/**
 * @brief      Loads data into the data store from an external source into the current bank.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance.
 * @param      p_Bank0      `true` if the data should be loaded into bank 0, `false` if it should be loaded into the current bank.
 * @param      p_Name       The name to assign to the data handle.
 * @param      p_Data       A pointer to the data to load into the data store.
 * @param      p_Length     The length of the data to load, in bytes.
 * 
 * @return     A pointer to the data handle for the loaded data, or `NULL` if the load operation failed.
 */
const GABLE_DataHandle* GABLE_LoadData (GABLE_DataStore* p_DataStore, Bool p_Bank0, const Char* p_Name, const Uint8* p_Data, Uint16 p_Length);

/**
 * @brief      Loads data into the data store from a file on disk into the current bank.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance.
 * @param      p_Bank0      `true` if the data should be loaded into bank 0, `false` if it should be loaded into the current bank.
 * @param      p_Name       The name to assign to the data handle.
 * @param      p_FilePath   The path to the file to load data from.
 * 
 * @return     A pointer to the data handle for the loaded data, or `NULL` if the load operation failed.
 */
const GABLE_DataHandle* GABLE_LoadDataFile (GABLE_DataStore* p_DataStore, Bool p_Bank0, const Char* p_Name, const Char* p_FilePath);

/**
 * @brief      Changes the current data store bank to the bank housing the data handle.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance.
 * @param      p_Handle     A pointer to the data handle to change the current bank to.
 * 
 * @return     `true` if the operation was successful, `false` if the handle's bank number is out of bounds.
 */
Bool GABLE_GotoDataHandle (GABLE_DataStore* p_DataStore, const GABLE_DataHandle* p_Handle);

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

/**
 * @brief      Reads a byte from the data store at the specified address.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance.
 * @param      p_Address     The address in the data store to read from.
 * @param      p_Value       A pointer to the variable to store the read byte in.
 * 
 * @return     `true` if the read operation was successful, `false` otherwise.
 */
Bool GABLE_ReadDataStoreByte (const GABLE_DataStore* p_DataStore, Uint16 p_Address, Uint8* p_Value);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief      Gets the value of the `DSBKH` (Data Store Bank High) register.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance.
 * 
 * @return     The value of the `DSBKH` register.
 */
Uint8 GABLE_ReadDSBKH (const GABLE_DataStore* p_DataStore);

/**
 * @brief      Gets the value of the `DSBKL` (Data Store Bank Low) register.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance.
 * 
 * @return     The value of the `DSBKL` register.
 */
Uint8 GABLE_ReadDSBKL (const GABLE_DataStore* p_DataStore);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief      Sets the value of the `DSBKH` (Data Store Bank High) register.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance.
 * @param      p_Value      The new value of the `DSBKH` register.
 */
void GABLE_WriteDSBKH (GABLE_DataStore* p_DataStore, Uint8 p_Value);

/**
 * @brief      Sets the value of the `DSBKL` (Data Store Bank Low) register.
 * 
 * @param      p_DataStore  A pointer to the GABLE Engine data store instance.
 * @param      p_Value      The new value of the `DSBKL` register.
 */
void GABLE_WriteDSBKL (GABLE_DataStore* p_DataStore, Uint8 p_Value);
