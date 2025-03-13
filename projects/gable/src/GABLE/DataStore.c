/**
 * @file GABLE/DataStore.c
 */

#include <GABLE/Engine.h>
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

    l_DataStore->m_HandleCapacity = GABLE_DS_DEFAULT_CAPACITY;
    l_DataStore->m_HandleCount = 0;

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
        *p_Value = p_DataStore->m_Data[(l_BankNumber * GABLE_DS_BANK_SIZE) + (p_Address - GABLE_DS_BANK_SIZE)];
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

// Public Functions - High-Level Functions /////////////////////////////////////////////////////////

const GABLE_DataHandle* GABLE_LoadDataFromBuffer (GABLE_Engine* p_Engine, const Char* p_Name, const Uint8* p_Buffer, Uint16 p_Size, Uint16 p_BankNumber)
{
    // Validate the pointers.
    GABLE_expect(p_Engine != NULL, "GABLE Engine context is NULL!");
    GABLE_expect(p_Name != NULL, "Data handle name is NULL!");
    GABLE_expect(p_Buffer != NULL, "Data buffer is NULL!");

    // Ensure the data name is not empty.
    if (p_Name[0] == '\0')
    {
        GABLE_error("Data handle name is empty.");
        return NULL;
    }

    // Validate the data size.
    if (p_Size == 0)
    {
        GABLE_error("Data buffer size is 0 bytes.");
        return NULL;
    }

    // Ensure that a data handle with the given name doesn't already exist.
    if (GABLE_GetDataHandle(p_Engine, p_Name) != NULL)
    {
        GABLE_error("Data handle with name '%s' already exists.", p_Name);
        return NULL;
    }

    // Point to the data store instance.
    GABLE_DataStore* l_DataStore = GABLE_GetDataStore(p_Engine);

    // Check to see if the bank number is out of bounds.
    if (p_BankNumber >= l_DataStore->m_BankCount)
    {
        GABLE_error("Data store bank number %u is out of bounds.", p_BankNumber);
        return NULL;
    }

    // Get the size of the data in the current bank.
    Uint16 l_BankSize = l_DataStore->m_DataSizes[p_BankNumber];

    // Check to see if the new data will fit in the current bank.
    if (l_BankSize + p_Size > GABLE_DS_BANK_SIZE)
    {
        GABLE_error("Data store bank %u does not have enough space for %u bytes.", p_BankNumber, p_Size);
        GABLE_error(" - Bank %u's size: %u bytes", p_BankNumber, l_BankSize);
        return NULL;
    }
    
    // Create a new data handle instance. Resize the data handle array if necessary.
    GABLE_ResizeDataHandles(l_DataStore);
    GABLE_DataHandle* l_Handle = &l_DataStore->m_Handles[l_DataStore->m_HandleCount++];
    strncpy(l_Handle->m_Name, p_Name, GABLE_DS_NAME_STRLEN);
    l_Handle->m_Length = p_Size;
    l_Handle->m_Address = l_BankSize;
    l_Handle->m_BankHigh = (p_BankNumber >> 8) & 0xFF;
    l_Handle->m_BankLow = p_BankNumber & 0xFF;

    // Copy the data to the data store, then update the data store's bank size.
    Size l_Offset = p_BankNumber * GABLE_DS_BANK_SIZE + l_Handle->m_Address;
    memcpy(&l_DataStore->m_Data[l_Offset], p_Buffer, p_Size);
    l_DataStore->m_DataSizes[p_BankNumber] += p_Size;

    // Return the new data handle instance.
    return l_Handle;
}

const GABLE_DataHandle* GABLE_LoadDataFromFile (GABLE_Engine* p_Engine, const Char* p_Name, const Char* p_FilePath, Uint16 p_BankNumber)
{
    // Validate the pointers.
    GABLE_expect(p_Engine != NULL, "GABLE Engine context is NULL!");
    GABLE_expect(p_Name != NULL, "Data handle name is NULL!");
    GABLE_expect(p_FilePath != NULL, "File path is NULL!");

    // Ensure the data name and file path are not empty.
    if (p_Name[0] == '\0')
    {
        GABLE_error("Data handle name is empty.");
        return NULL;
    }
    else if (p_FilePath[0] == '\0')
    {
        GABLE_error("File path is empty.");
        return NULL;
    }

    // Ensure that a data handle with the given name doesn't already exist.
    if (GABLE_GetDataHandle(p_Engine, p_Name) != NULL)
    {
        GABLE_error("Data handle with name '%s' already exists.", p_Name);
        return NULL;
    }

    // Point to the data store instance.
    GABLE_DataStore* l_DataStore = GABLE_GetDataStore(p_Engine);

    // Check to see if the bank number is out of bounds.
    if (p_BankNumber >= l_DataStore->m_BankCount)
    {
        GABLE_error("Data store bank number %u is out of bounds.", p_BankNumber);
        return NULL;
    }

    // Get the size of the data in the current bank.
    Uint16 l_BankSize = l_DataStore->m_DataSizes[p_BankNumber];

    // Attempt to open the file for reading.
    FILE* l_File = fopen(p_FilePath, "rb");
    if (l_File == NULL)
    {
        GABLE_perror("Failed to open file '%s' for reading", p_FilePath);
        return NULL;
    }

    // Attempt to read the file's size.
    fseek(l_File, 0, SEEK_END);
    Int64 l_SignedSize = ftell(l_File);
    if (l_SignedSize < 0)
    {
        GABLE_perror("Failed to get size of file '%s'", p_FilePath);
        fclose(l_File);
        return NULL;
    }
    rewind(l_File);

    // Check to see if the file is too large to fit in the current bank.
    Size l_FileSize = (Size) l_SignedSize;
    if (l_FileSize + l_BankSize > GABLE_DS_BANK_SIZE)
    {
        GABLE_error("Data store bank %u does not have enough space for %zu bytes.", p_BankNumber, l_FileSize);
        GABLE_error(" - Loading file '%s'.", p_FilePath);
        GABLE_error(" - Bank %u's size: %u bytes", p_BankNumber, l_BankSize);
        fclose(l_File);
        return NULL;
    }

    // Create a new data handle instance. Resize the data handle array if necessary.
    GABLE_ResizeDataHandles(l_DataStore);
    GABLE_DataHandle* l_Handle = &l_DataStore->m_Handles[l_DataStore->m_HandleCount++];
    strncpy(l_Handle->m_Name, p_Name, GABLE_DS_NAME_STRLEN);
    l_Handle->m_Length = l_FileSize;
    l_Handle->m_Address = l_BankSize;
    l_Handle->m_BankHigh = (p_BankNumber >> 8) & 0xFF;
    l_Handle->m_BankLow = p_BankNumber & 0xFF;

    // Read the file's contents into the data store, then update the data store's bank size.
    Size l_Offset = p_BankNumber * GABLE_DS_BANK_SIZE + l_Handle->m_Address;
    Size l_ReadSize = fread(&l_DataStore->m_Data[l_Offset], 1, l_FileSize, l_File);
    if (l_ReadSize != l_FileSize || (ferror(l_File) && !feof(l_File)))
    {
        GABLE_perror("Failed to read file '%s' into data store", p_FilePath);
        fclose(l_File);
        l_DataStore->m_HandleCount--;
        return NULL;
    }

    // Update the data store's bank size, then close the file.
    l_DataStore->m_DataSizes[p_BankNumber] += l_FileSize;
    fclose(l_File);

    // Return the new data handle instance.
    return l_Handle;
}

const GABLE_DataHandle* GABLE_GetDataHandle (GABLE_Engine* p_Engine, const Char* p_Name)
{
    // Validate the pointers.
    GABLE_expect(p_Engine != NULL, "GABLE Engine context is NULL!");
    GABLE_expect(p_Name != NULL, "Data handle name is NULL!");

    // Ensure the data name is not empty.
    if (p_Name[0] == '\0')
    {
        GABLE_error("Data handle name is empty.");
        return NULL;
    }

    // Point to the data store instance.
    GABLE_DataStore* l_DataStore = GABLE_GetDataStore(p_Engine);

    // Search for the data handle with the specified name.
    for (Index i = 0; i < l_DataStore->m_HandleCount; i++)
    {
        if (strncmp(l_DataStore->m_Handles[i].m_Name, p_Name, GABLE_DS_NAME_STRLEN) == 0)
        {
            return &l_DataStore->m_Handles[i];
        }
    }

    // Return NULL if the data handle was not found.
    return NULL;
}

Bool GABLE_GotoBankWithDataHandle (GABLE_Engine* p_Engine, const GABLE_DataHandle* p_Handle)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "GABLE Engine context is NULL!");

    // Validate the data handle instance.
    GABLE_expect(p_Handle != NULL, "Data handle context is NULL!");
    
    // Combine the high and low bytes of the bank number.
    Uint16 l_BankNumber = (p_Handle->m_BankHigh << 8) | p_Handle->m_BankLow;

    // If the bank number is zero, then that bank is always accessible. Do nothing.
    if (l_BankNumber == 0)
    {
        return true;
    }

    // Check to see if the bank number is out of bounds.
    if (l_BankNumber >= GABLE_GetDataStoreBankCount(p_Engine))
    {
        GABLE_error("Data store bank number %u is out of bounds.", l_BankNumber);
        return false;
    }

    // Write the high and low bytes of the bank number to the `DSBKH` and `DSBKL` registers.
    GABLE_SetDataStoreBankNumber(p_Engine, l_BankNumber);
    return true;
}

Uint16 GABLE_GetDataStoreBankCount (GABLE_Engine* p_Engine)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "GABLE Engine context is NULL!");

    // Point to the data store instance. Get the number of banks in the data store.
    GABLE_DataStore* l_DataStore = GABLE_GetDataStore(p_Engine);
    return l_DataStore->m_BankCount;
}

Uint16 GABLE_GetDataStoreBankNumber (GABLE_Engine* p_Engine)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "GABLE Engine context is NULL!");

    // Read the `DSBKH` and `DSBKL` registers. Combine the values to get the bank number.
    Uint8 l_BankHigh = 0x00, l_BankLow = 0x00;
    GABLE_CycleReadByte(p_Engine, GABLE_HP_DSBKH, &l_BankHigh);
    GABLE_CycleReadByte(p_Engine, GABLE_HP_DSBKL, &l_BankLow);

    return (l_BankHigh << 8) | l_BankLow;
}

void GABLE_SetDataStoreBankCount (GABLE_Engine* p_Engine, Uint16 p_BankCount)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "GABLE Engine context is NULL!");

    // Point to the data store instance.
    GABLE_DataStore* l_DataStore = GABLE_GetDataStore(p_Engine);

    // Minimum 2 banks are required for the data store. Correct if needed.
    if (p_BankCount < 2)
    {
        p_BankCount = 2;
    }

    // Check to see if the new bank count is different from the current bank count.
    if (p_BankCount != l_DataStore->m_BankCount)
    {
        // If the current bank number exceeds the new bank count, then correct it.
        if (l_DataStore->m_CurrentBank >= p_BankCount)
        {
            l_DataStore->m_CurrentBank = p_BankCount - 1;
        }

        // Allocate a new data store memory block with the new bank count.
        Uint8* l_NewData = GABLE_calloc(p_BankCount * GABLE_DS_BANK_SIZE, Uint8);
        GABLE_pexpect(l_NewData != NULL, "Failed to allocate GABLE Engine data store banks");

        // Allocate a new data store bank size array with the new bank count.
        Uint16* l_NewDataSizes = GABLE_calloc(p_BankCount, Uint16);
        GABLE_pexpect(l_NewDataSizes != NULL, "Failed to allocate GABLE Engine data store bank sizes");
        
        // Copy the data from the old data store memory block to the new data store memory block.
        if (p_BankCount > l_DataStore->m_BankCount)
        {
            memcpy(l_NewData, l_DataStore->m_Data, l_DataStore->m_BankCount * GABLE_DS_BANK_SIZE);
            memcpy(l_NewDataSizes, l_DataStore->m_DataSizes, l_DataStore->m_BankCount * sizeof(Uint16));
        }
        else
        {
            memcpy(l_NewData, l_DataStore->m_Data, p_BankCount * GABLE_DS_BANK_SIZE);
            memcpy(l_NewDataSizes, l_DataStore->m_DataSizes, p_BankCount * sizeof(Uint16));
        }

        // Free the old data store memory block and bank size array.
        GABLE_free(l_DataStore->m_Data);
        GABLE_free(l_DataStore->m_DataSizes);

        // Point the old pointers to the new data store memory block and bank size array.
        l_DataStore->m_Data = l_NewData;
        l_DataStore->m_DataSizes = l_NewDataSizes;

        // Update the data store's bank count.
        l_DataStore->m_BankCount = p_BankCount;
    }
}

void GABLE_SetDataStoreBankNumber (GABLE_Engine* p_Engine, Uint16 p_BankNumber)
{
    // Validate the GABLE Engine instance.
    GABLE_expect(p_Engine != NULL, "GABLE Engine context is NULL!");

    // Write the high and low bytes of the bank number to the `DSBKH` and `DSBKL` registers.
    GABLE_CycleWriteByte(p_Engine, GABLE_HP_DSBKH, (p_BankNumber >> 8) & 0xFF);
    GABLE_CycleWriteByte(p_Engine, GABLE_HP_DSBKL, p_BankNumber & 0xFF);
}
