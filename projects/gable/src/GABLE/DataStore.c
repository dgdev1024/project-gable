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

void GABLE_SetDataStoreBankCount (GABLE_DataStore* p_DataStore, Uint16 p_BankCount)
{
    // Validate the data store instance.
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");

    // Validate the new bank count. Correct if necessary.
    if (p_BankCount < 2)
    {
        p_BankCount = 2;
    }
    
    // If the new bank count is greater than the current bank count, reallocate the data store's memory.
    if (p_BankCount > p_DataStore->m_BankCount)
    {
        // Reallocate the data store's bank size array.
        Uint16* l_NewDataSizes = GABLE_calloc(p_BankCount, Uint16);
        GABLE_pexpect(l_NewDataSizes != NULL, "Failed to reallocate GABLE Engine data store bank sizes");

        // Reallocate the data store's memory.
        Uint8* l_NewData = GABLE_calloc(p_BankCount * GABLE_DS_BANK_SIZE, Uint8);
        GABLE_pexpect(l_NewData != NULL, "Failed to reallocate GABLE Engine data store banks");

        // Copy the existing bank sizes to the new memory.
        memcpy(l_NewDataSizes, p_DataStore->m_DataSizes, p_DataStore->m_BankCount * sizeof(Uint16));

        // Copy the existing data to the new memory.
        memcpy(l_NewData, p_DataStore->m_Data, p_DataStore->m_BankCount * GABLE_DS_BANK_SIZE);

        // Free the old data store memory and sizes.
        GABLE_free(p_DataStore->m_DataSizes);
        GABLE_free(p_DataStore->m_Data);

        // Update the data store's properties.
        p_DataStore->m_Data      = l_NewData;
        p_DataStore->m_DataSizes = l_NewDataSizes;
    }

    // Update the data store's bank count.
    p_DataStore->m_BankCount = p_BankCount;
}

void GABLE_SetDataStoreBankNumber (GABLE_DataStore* p_DataStore, Uint16 p_BankNumber)
{
    // Validate the data store instance.
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");

    // Validate the new bank number. Correct if necessary.
    if (p_BankNumber < 1)
    {
        p_BankNumber = 1;
    }
    else if (p_BankNumber > p_DataStore->m_BankCount)
    {
        p_BankNumber = p_DataStore->m_BankCount;
    }

    // Update the data store's current bank number.
    p_DataStore->m_CurrentBank = p_BankNumber;
}

const GABLE_DataHandle* GABLE_GetDataHandle (const GABLE_DataStore* p_DataStore, const Char* p_Name)
{
    // Validate the data store instance and the data handle name.
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");
    GABLE_expect(p_Name != NULL, "Data handle name is NULL!");

    // Ensure the name is not empty and is not too long.
    if (p_Name[0] == '\0')
    {
        GABLE_error("Data handle name is empty.");
        return NULL;
    }
    else if (strlen(p_Name) >= GABLE_DS_NAME_STRLEN)
    {
        GABLE_error("Data handle name is too long.");
        return NULL;
    }

    // Search for the data handle with the specified name.
    for (Count i = 0; i < p_DataStore->m_HandleCount; ++i)
    {
        if (strncmp(p_DataStore->m_Handles[i].m_Name, p_Name, GABLE_DS_NAME_STRLEN) == 0)
        {
            return &p_DataStore->m_Handles[i];
        }
    }

    // No data handle with the specified name was found.
    return NULL;
}

const GABLE_DataHandle* GABLE_LoadData (GABLE_DataStore* p_DataStore, Bool p_Bank0, const Char* p_Name, const Uint8* p_Data, Uint16 p_Length)
{
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");
    GABLE_expect(p_Name != NULL, "Data handle name is NULL!");
    GABLE_expect(p_Data != NULL, "Data pointer is NULL!");

    // Ensure the name is not empty and is not too long.
    Size l_NameLength = strlen(p_Name);
    if (l_NameLength == 0)
    {
        GABLE_error("Data handle name is empty.");
        return NULL;
    }
    else if (l_NameLength >= GABLE_DS_NAME_STRLEN)
    {
        GABLE_error("Data handle name is too long.");
        return NULL;
    }

    // Check to see if a data handle with the specified name already exists.
    const GABLE_DataHandle* l_ExistingHandle = GABLE_GetDataHandle(p_DataStore, p_Name);
    if (l_ExistingHandle != NULL)
    {
        GABLE_error("Data handle with name '%s' already exists.", p_Name);
        return NULL;
    }

    // Ensure the data length is not zero.
    if (p_Length == 0)
    {
        GABLE_error("Data length is zero.");
        return NULL;
    }

    // Ensure that there is enough space in the current 16KB bank to store the data.
    Uint16 l_BankNumber = (p_Bank0 == true) ? 0 : p_DataStore->m_CurrentBank;
    Uint16 l_BankSize = p_DataStore->m_DataSizes[l_BankNumber];
    if (l_BankSize + p_Length > GABLE_DS_BANK_SIZE)
    {
        GABLE_error("Data store bank %u does not have enough space to load %u bytes.", l_BankNumber, p_Length);
        return NULL;
    }

    // Resize the data handle array if necessary.
    GABLE_ResizeDataHandles(p_DataStore);

    // Record the new data handle.
    GABLE_DataHandle* l_NewHandle = &p_DataStore->m_Handles[p_DataStore->m_HandleCount++];
    strncpy(l_NewHandle->m_Name, p_Name, GABLE_DS_NAME_STRLEN);
    l_NewHandle->m_BankHigh = (l_BankNumber >> 8) & 0xFF;
    l_NewHandle->m_BankLow  = l_BankNumber & 0xFF;
    l_NewHandle->m_Address  = l_BankSize;
    l_NewHandle->m_Length   = p_Length;

    // Copy the data into the data store.
    memcpy(
        &p_DataStore->m_Data[l_BankNumber * GABLE_DS_BANK_SIZE + l_BankSize],
        p_Data,
        p_Length
    );

    // Update the bank size.
    p_DataStore->m_DataSizes[l_BankNumber] += p_Length;

    // Return the new data handle.
    return l_NewHandle;
}

const GABLE_DataHandle* GABLE_LoadDataFile (GABLE_DataStore* p_DataStore, Bool p_Bank0, const Char* p_Name, const Char* p_FilePath)
{
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");
    GABLE_expect(p_Name != NULL, "Data handle name is NULL!");
    GABLE_expect(p_FilePath != NULL, "File path is NULL!");

    // Ensure the file path is not empty.
    if (p_FilePath[0] == '\0')
    {
        GABLE_error("File path is empty.");
        return NULL;
    }

    // Ensure the name is not empty and is not too long.
    Size l_NameLength = strlen(p_Name);
    if (l_NameLength == 0)
    {
        GABLE_error("Data handle name is empty.");
        return NULL;
    }
    else if (l_NameLength >= GABLE_DS_NAME_STRLEN)
    {
        GABLE_error("Data handle name is too long.");
        return NULL;
    }

    // Check to see if a data handle with the specified name already exists.
    const GABLE_DataHandle* l_ExistingHandle = GABLE_GetDataHandle(p_DataStore, p_Name);
    if (l_ExistingHandle != NULL)
    {
        GABLE_error("Data handle with name '%s' already exists.", p_Name);
        return NULL;
    }

    // Load the file from disk.
    FILE* l_File = fopen(p_FilePath, "rb");
    if (l_File == NULL)
    {
        GABLE_perror("Failed to open file '%s'", p_FilePath);
        return NULL;
    }

    // Get and validate the file size.
    // - Ensure the file size is not negative, indicating an error getting the file size.
    // - Ensure the file size is not zero, indicating an empty file.
    fseek(l_File, 0, SEEK_END);
    Int64 l_FileSize = ftell(l_File);
    if (l_FileSize < 0)
    {
        GABLE_perror("Failed to get file size for '%s'", p_FilePath);
        fclose(l_File);
        return NULL;
    }
    else if (l_FileSize == 0)
    {
        GABLE_error("File '%s' is empty.", p_FilePath);
        fclose(l_File);
        return NULL;
    }
    rewind(l_File);

    // Ensure that the file can fit in the current 16KB bank.
    Uint16 l_BankNumber = (p_Bank0 == true) ? 0 : p_DataStore->m_CurrentBank;
    Uint16 l_BankSize = p_DataStore->m_DataSizes[l_BankNumber];
    if (l_BankSize + (Size) l_FileSize > GABLE_DS_BANK_SIZE)
    {
        GABLE_error("Data store bank %u does not have enough space to load file '%s' (%ld bytes).", 
            l_BankNumber, p_FilePath, l_FileSize);
        fclose(l_File);
        return NULL;
    }

    // Read the file into the data store at the current bank.
    Size l_ReadSize = fread(&p_DataStore->m_Data[l_BankNumber * GABLE_DS_BANK_SIZE + l_BankSize], 1, l_FileSize, l_File);
    if (l_ReadSize != l_FileSize || (ferror(l_File) && !feof(l_File)))
    {
        GABLE_perror("Failed to read file '%s' into data store", p_FilePath);
        fclose(l_File);
        return NULL;
    }

    // Close the file.
    fclose(l_File);

    // Resize the data handle array if necessary.
    GABLE_ResizeDataHandles(p_DataStore);

    // Record the new data handle.
    GABLE_DataHandle* l_NewHandle = &p_DataStore->m_Handles[p_DataStore->m_HandleCount++];
    strncpy(l_NewHandle->m_Name, p_Name, GABLE_DS_NAME_STRLEN);
    l_NewHandle->m_BankHigh = (l_BankNumber >> 8) & 0xFF;
    l_NewHandle->m_BankLow  = l_BankNumber & 0xFF;
    l_NewHandle->m_Address  = l_BankSize;
    l_NewHandle->m_Length   = l_FileSize;

    // Update the bank size.
    p_DataStore->m_DataSizes[l_BankNumber] += l_FileSize;

    // Return the new data handle.
    return l_NewHandle;

}

Bool GABLE_GotoDataHandle (GABLE_DataStore* p_DataStore, const GABLE_DataHandle* p_Handle)
{
    // Validate the data store instance and the data handle.
    GABLE_expect(p_DataStore != NULL, "Data store context is NULL!");
    GABLE_expect(p_Handle != NULL, "Data handle is NULL!");

    // Data store bank 0 is always accessible. If the handle is in bank 0, therefore, then do
    // nothing.
    if (p_Handle->m_BankHigh == 0 && p_Handle->m_BankLow == 0)
    {
        return true;
    }

    // Ensure the bank number is within the data store's current bank count.
    Uint16 m_TargetBank = (p_Handle->m_BankHigh << 8) | p_Handle->m_BankLow;
    if (m_TargetBank >= p_DataStore->m_BankCount)
    {
        GABLE_error("Data handle bank number %u is out of bounds.", m_TargetBank);
        return false;
    }

    // Update the data store's current bank number.
    p_DataStore->m_CurrentBank = m_TargetBank;

    return true;
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
