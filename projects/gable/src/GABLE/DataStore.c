/**
 * @file GABLE/DataStore.c
 */

#include <GABLE/DataStore.h>

// Private Constants ///////////////////////////////////////////////////////////////////////////////

#define GABLE_DS_DEFAULT_CAPACITY  16

// GABLE Data Store Structure //////////////////////////////////////////////////////////////////////

typedef struct GABLE_DataStore
{
    GABLE_DataHandle* m_Handles;            ///< @brief An array of data handles.
    Count             m_HandleCount;        ///< @brief The number of data handles.
    Count             m_HandleCapacity;     ///< @brief The capacity of the data handle array.

    Uint8*  m_Data;             ///< @brief The data store's allocated memory.
    Uint16* m_DataSizes;        ///< @brief An array of the occupied sizes of each bank.
    Uint16  m_BankCount;        ///< @brief The number of banks in the data store.
    Uint16  m_CurrentBank;      ///< @brief The current bank number.
} GABLE_DataStore;

// Static Functions ////////////////////////////////////////////////////////////////////////////////

static void GABLE_ResizeDataHandles (GABLE_DataStore* p_Store)
{
    if (p_Store->m_HandleCount + 1 >= p_Store->m_HandleCapacity)
    {
        Count l_NewCapacity = p_Store->m_HandleCapacity * 2;
        GABLE_DataHandle* l_NewHandles = GABLE_realloc(p_Store->m_Handles, l_NewCapacity, GABLE_DataHandle);
        GABLE_pexpect(l_NewHandles != NULL, "Failed to reallocate GABLE Engine data store handles");

        p_Store->m_Handles = l_NewHandles;
        p_Store->m_HandleCapacity = l_NewCapacity;
    }
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_DataStore* GABLE_CreateDataStore ()
{
    // Allocate the GABLE Engine data store instance.
    GABLE_DataStore* l_DataStore = GABLE_calloc(1, GABLE_DataStore);
    GABLE_pexpect(l_DataStore != NULL, "Failed to allocate GABLE Engine data store");

    // Initialize the data store's properties.
    l_DataStore->m_BankCount   = GABLE_DS_DEFAULT_BANK_COUNT;
    l_DataStore->m_CurrentBank = 1;

    l_DataStore->m_Data        = GABLE_calloc(l_DataStore->m_BankCount * GABLE_DS_BANK_SIZE, Uint8);
    GABLE_pexpect(l_DataStore->m_Data != NULL, "Failed to allocate GABLE Engine data store banks");

    l_DataStore->m_DataSizes   = GABLE_calloc(l_DataStore->m_BankCount, Uint16);
    GABLE_pexpect(l_DataStore->m_DataSizes != NULL, "Failed to allocate GABLE Engine data store bank sizes");

    l_DataStore->m_Handles     = GABLE_calloc(GABLE_DS_DEFAULT_CAPACITY, GABLE_DataHandle);
    GABLE_pexpect(l_DataStore->m_Handles != NULL, "Failed to allocate GABLE Engine data store handles");

    // Return the new data store instance.
    return l_DataStore;
}

void GABLE_DestroyDataStore (GABLE_DataStore* p_DataStore)
{
    if (p_DataStore != NULL)
    {
        GABLE_free(p_DataStore->m_Handles);
        GABLE_free(p_DataStore->m_DataSizes);
        GABLE_free(p_DataStore->m_Data);
        GABLE_free(p_DataStore);
    }
}

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

Bool GABLE_ReadDataStoreByte (const GABLE_DataStore* p_DataStore, Uint16 p_Address, Uint8* p_Value)
{
    // Validate the data store instance.
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");
    
    // Validate the address.
    if (p_Address >= GABLE_GB_ROM_SIZE)
    {
        GABLE_error("Data store address %u is out of bounds.", p_Address);
        return false;
    }

    // Read the byte from the data store.
    // - Read from bank 0 if the address is less than `$4000`.
    // - Read from the current bank otherwise.
    if (p_Address < GABLE_DS_BANK_SIZE)
    {
        *p_Value = p_DataStore->m_Data[p_Address];
    }
    else
    {
        Uint16 l_BankNumber = p_DataStore->m_CurrentBank;
        *p_Value = p_DataStore->m_Data[l_BankNumber * GABLE_DS_BANK_SIZE + p_Address];
    }

    return true;
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadDSBKH (const GABLE_DataStore* p_DataStore)
{
    // Validate the data store instance.
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");

    // Return the high byte of the current bank number.
    return (p_DataStore->m_CurrentBank >> 8) & 0xFF;
}

Uint8 GABLE_ReadDSBKL (const GABLE_DataStore* p_DataStore)
{
    // Validate the data store instance.
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");

    // Return the low byte of the current bank number.
    return p_DataStore->m_CurrentBank & 0xFF;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void GABLE_WriteDSBKH (GABLE_DataStore* p_DataStore, Uint8 p_Value)
{
    // Validate the data store instance.
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");

    // Update the data store's bank number.
    p_DataStore->m_CurrentBank = (p_DataStore->m_CurrentBank & 0x00FF) | (p_Value << 8);
    if (p_DataStore->m_CurrentBank > p_DataStore->m_BankCount)
    {
        p_DataStore->m_CurrentBank = p_DataStore->m_BankCount;
    }
    else if (p_DataStore->m_CurrentBank < 1)
    {
        p_DataStore->m_CurrentBank = 1;
    }
}

void GABLE_WriteDSBKL (GABLE_DataStore* p_DataStore, Uint8 p_Value)
{
    // Validate the data store instance.
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");

    // Update the data store's bank number.
    p_DataStore->m_CurrentBank = (p_DataStore->m_CurrentBank & 0xFF00) | p_Value;
    if (p_DataStore->m_CurrentBank > p_DataStore->m_BankCount)
    {
        p_DataStore->m_CurrentBank = p_DataStore->m_BankCount;
    }
    else if (p_DataStore->m_CurrentBank < 1)
    {
        p_DataStore->m_CurrentBank = 1;
    }
}
