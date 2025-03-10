/**
 * @file GABLE/RAM.c
 */

#include <GABLE/RAM.h>

// GABLE RAM Structure /////////////////////////////////////////////////////////////////////////////

typedef struct GABLE_RAM
{
    Uint8* m_WRAM;              ///< @brief The working RAM banks.
    Uint8* m_SRAM;              ///< @brief The static RAM banks.
    Uint8* m_HRAM;              ///< @brief The high RAM buffer.
    Uint8  m_WRAMBankCount;     ///< @brief The number of working RAM banks.
    Uint8  m_SRAMBankCount;     ///< @brief The number of static RAM banks.
    Uint8  m_WRAMBankNumber;    ///< @brief The current working RAM bank number.
    Uint8  m_SRAMBankNumber;    ///< @brief The current static RAM bank number.
} GABLE_RAM;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_RAM* GABLE_CreateRAM ()
{
    // Allocate the GABLE Engine RAM instance.
    GABLE_RAM* l_RAM = GABLE_calloc(1, GABLE_RAM);
    GABLE_pexpect(l_RAM != NULL, "Failed to allocate GABLE Engine RAM");

    // Allocate the RAM buffers.
    l_RAM->m_WRAM = GABLE_calloc(GABLE_RAM_WRAM_BANK_SIZE * GABLE_RAM_DEFAULT_WRAM_BANKS, Uint8);
    GABLE_expect(l_RAM->m_WRAM != NULL, "Failed to allocate GABLE Engine working RAM banks");

    l_RAM->m_SRAM = GABLE_calloc(GABLE_RAM_SRAM_BANK_SIZE * GABLE_RAM_DEFAULT_SRAM_BANKS, Uint8);
    GABLE_expect(l_RAM->m_SRAM != NULL, "Failed to allocate GABLE Engine static RAM banks");

    l_RAM->m_HRAM = GABLE_calloc(GABLE_RAM_HRAM_SIZE, Uint8);
    GABLE_expect(l_RAM->m_HRAM != NULL, "Failed to allocate GABLE Engine high RAM buffer");

    // Initialize the RAM properties.
    l_RAM->m_WRAMBankCount  = GABLE_RAM_DEFAULT_WRAM_BANKS;
    l_RAM->m_SRAMBankCount  = GABLE_RAM_DEFAULT_SRAM_BANKS;
    l_RAM->m_WRAMBankNumber = 1;
    l_RAM->m_SRAMBankNumber = 0;

    // Return the new RAM instance.
    return l_RAM;
}

void GABLE_DestroyRAM (GABLE_RAM* p_RAM)
{
    if (p_RAM != NULL)
    {
        // Free the RAM buffers.
        GABLE_free(p_RAM->m_WRAM);
        GABLE_free(p_RAM->m_SRAM);
        GABLE_free(p_RAM->m_HRAM);

        // Free the RAM instance.
        GABLE_free(p_RAM);
    }
}

Uint8 GABLE_GetWRAMBankCount (const GABLE_RAM* p_RAM)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Return the number of working RAM banks.
    return p_RAM->m_WRAMBankCount;
}

Uint8 GABLE_GetSRAMBankCount (const GABLE_RAM* p_RAM)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Return the number of static RAM banks.
    return p_RAM->m_SRAMBankCount;
}

Uint8 GABLE_GetWRAMBankNumber (const GABLE_RAM* p_RAM)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Return the current working RAM bank number.
    return p_RAM->m_WRAMBankNumber;
}

Uint8 GABLE_GetSRAMBankNumber (const GABLE_RAM* p_RAM)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Return the current static RAM bank number.
    return p_RAM->m_SRAMBankNumber;
}

void GABLE_SetWRAMBankCount (GABLE_RAM* p_RAM, Uint8 p_BankCount)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");
    
    // Minimum of 2 working RAM banks. Correct if less.
    if (p_BankCount < 2)
    {
        p_BankCount = 2;
    }

    // If the number of banks is greater than the current number of banks, reallocate memory for the 
    // new banks.
    if (p_BankCount > p_RAM->m_WRAMBankCount)
    {
        Uint8* l_Reallocation = GABLE_realloc(p_RAM->m_WRAM, p_BankCount * GABLE_RAM_WRAM_BANK_SIZE, Uint8);
        GABLE_pexpect(l_Reallocation != NULL, "Failed to reallocate GABLE Engine working RAM banks");

        // Initialize the new banks to zero.
        memset(l_Reallocation + (p_RAM->m_WRAMBankCount * GABLE_RAM_WRAM_BANK_SIZE), 0, (p_BankCount - p_RAM->m_WRAMBankCount) * GABLE_RAM_WRAM_BANK_SIZE);

        // Update the RAM properties.
        p_RAM->m_WRAM           = l_Reallocation;
        p_RAM->m_WRAMBankCount  = p_BankCount;
    }

    // If the number of banks is less than the current number of banks, then do nothing.
}

void GABLE_SetSRAMBankCount (GABLE_RAM* p_RAM, Uint8 p_BankCount)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Minimum of 1 static RAM bank. Correct if less.
    if (p_BankCount < 1)
    {
        p_BankCount = 1;
    }

    // If the number of banks is greater than the current number of banks, reallocate memory for the 
    // new banks.
    if (p_BankCount > p_RAM->m_SRAMBankCount)
    {
        Uint8* l_Reallocation = GABLE_realloc(p_RAM->m_SRAM, p_BankCount * GABLE_RAM_SRAM_BANK_SIZE, Uint8);
        GABLE_pexpect(l_Reallocation != NULL, "Failed to reallocate GABLE Engine static RAM banks");

        // Initialize the new banks to zero.
        memset(l_Reallocation + (p_RAM->m_SRAMBankCount * GABLE_RAM_SRAM_BANK_SIZE), 0, (p_BankCount - p_RAM->m_SRAMBankCount) * GABLE_RAM_SRAM_BANK_SIZE);

        // Update the RAM properties.
        p_RAM->m_SRAM           = l_Reallocation;
        p_RAM->m_SRAMBankCount  = p_BankCount;
    }

    // If the number of banks is less than the current number of banks, retain that memory for later use.
    else if (p_BankCount < p_RAM->m_SRAMBankCount)
    {
        // Correct the current bank number if it exceeds the new bank count.
        if (p_RAM->m_SRAMBankNumber >= p_BankCount)
        {
            p_RAM->m_SRAMBankNumber = 0;
        }

        // Update the RAM properties.
        p_RAM->m_SRAMBankCount  = p_BankCount;
    }
}

void GABLE_SetWRAMBankNumber (GABLE_RAM* p_RAM, Uint8 p_BankNumber)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Correct the bank number if it exceeds the number of banks.
    if (p_BankNumber >= p_RAM->m_WRAMBankCount)
    {
        p_BankNumber = p_RAM->m_WRAMBankCount - 1;
    }

    // Update the RAM's current working RAM bank number.
    p_RAM->m_WRAMBankNumber = p_BankNumber;
}

void GABLE_SetSRAMBankNumber (GABLE_RAM* p_RAM, Uint8 p_BankNumber)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Correct the bank number if it exceeds the number of banks.
    if (p_BankNumber >= p_RAM->m_SRAMBankCount)
    {
        p_BankNumber = p_RAM->m_SRAMBankCount - 1;
    }

    // Update the RAM's current static RAM bank number.
    p_RAM->m_SRAMBankNumber = p_BankNumber;
}

Bool GABLE_ReadWRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8* p_Value)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    if (p_Address >= GABLE_GB_WRAM_SIZE)
    {
        GABLE_error("Working RAM address '%u' is out of bounds.", p_Address);
        return false;
    }

    if (p_Address < GABLE_RAM_WRAM_BANK_SIZE)
    {
        *p_Value = p_RAM->m_WRAM[p_Address];
    }
    else
    {
        *p_Value = p_RAM->m_WRAM[(p_RAM->m_WRAMBankNumber * GABLE_RAM_WRAM_BANK_SIZE) + (p_Address - GABLE_RAM_WRAM_BANK_SIZE)];
    }

    return true;
}

Bool GABLE_ReadSRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8* p_Value)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    if (p_Address >= GABLE_RAM_SRAM_BANK_SIZE)
    {
        GABLE_error("Static RAM address '%u' is out of bounds.", p_Address);
        return false;
    }

    *p_Value = p_RAM->m_SRAM[(p_RAM->m_SRAMBankNumber * GABLE_RAM_SRAM_BANK_SIZE) + p_Address];

    return true;
}

Bool GABLE_ReadHRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8* p_Value)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    if (p_Address >= GABLE_RAM_HRAM_SIZE)
    {
        GABLE_error("High RAM address '%u' is out of bounds.", p_Address);
        return false;
    }

    *p_Value = p_RAM->m_HRAM[p_Address];

    return true;
}

Bool GABLE_WriteWRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8 p_Value)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    if (p_Address >= GABLE_GB_WRAM_SIZE)
    {
        GABLE_error("Working RAM address '%u' is out of bounds.", p_Address);
        return false;
    }

    if (p_Address < GABLE_RAM_WRAM_BANK_SIZE)
    {
        p_RAM->m_WRAM[p_Address] = p_Value;
    }
    else
    {
        p_RAM->m_WRAM[(p_RAM->m_WRAMBankNumber * GABLE_RAM_WRAM_BANK_SIZE) + (p_Address - GABLE_RAM_WRAM_BANK_SIZE)] = p_Value;
    }

    return true;
}

Bool GABLE_WriteSRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8 p_Value)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    if (p_Address >= GABLE_RAM_SRAM_BANK_SIZE)
    {
        GABLE_error("Static RAM address '%u' is out of bounds.", p_Address);
        return false;
    }

    p_RAM->m_SRAM[(p_RAM->m_SRAMBankNumber * GABLE_RAM_SRAM_BANK_SIZE) + p_Address] = p_Value;

    return true;
}

Bool GABLE_WriteHRAMByte (GABLE_RAM* p_RAM, Uint16 p_Address, Uint8 p_Value)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    if (p_Address >= GABLE_RAM_HRAM_SIZE)
    {
        GABLE_error("High RAM address '%u' is out of bounds.", p_Address);
        return false;
    }

    p_RAM->m_HRAM[p_Address] = p_Value;

    return true;
}

Bool GABLE_LoadSRAMFile (GABLE_RAM* p_RAM, const Char* p_FilePath)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Validate the file path.
    GABLE_expect(p_FilePath != NULL, "File path is NULL!");
    if (p_FilePath[0] == '\0')
    {
        GABLE_error("File path is empty.");
        return false;
    }

    // Open the file for reading in binary mode.
    FILE* l_File = fopen(p_FilePath, "rb");
    GABLE_pexpect(l_File != NULL, "Failed to open save file '%s' for reading", p_FilePath);

    // Get and validate the file size.
    fseek(l_File, 0, SEEK_END);
    Int64 l_FileSize = ftell(l_File);
    if (l_FileSize < 0)
    {
        GABLE_perror("Failed to get file size of save file '%s'", p_FilePath);
        fclose(l_File);
        return false;
    }
    else if (l_FileSize % GABLE_RAM_SRAM_BANK_SIZE != 0)
    {
        GABLE_error("Save file '%s' size is not a multiple of the SRAM bank size.", p_FilePath);
        fclose(l_File);
        return false;
    }
    else if (l_FileSize > GABLE_RAM_SRAM_BANK_SIZE * GABLE_RAM_MAX_BANKS)
    {
        GABLE_error("Save file '%s' size exceeds the maximum SRAM size.", p_FilePath);
        fclose(l_File);
        return false;
    }
    rewind(l_File);

    // Re-size the SRAM buffer if necessary.
    Uint8 l_BankCount = (Uint8) (l_FileSize / GABLE_RAM_SRAM_BANK_SIZE);
    GABLE_SetSRAMBankCount(p_RAM, l_BankCount);

    // Read the file into the SRAM buffer.
    size_t l_BytesRead = fread(p_RAM->m_SRAM, 1, (Size) l_FileSize, l_File);
    if (l_BytesRead != (Size) l_FileSize || (ferror(l_File) && !feof(l_File)))
    {
        GABLE_perror("Failed to read save file '%s' into SRAM buffer", p_FilePath);
        fclose(l_File);
        return false;
    }

    // Close the file.
    fclose(l_File);
    return true;
}

Bool GABLE_SaveSRAMFile (GABLE_RAM* p_RAM, const Char* p_FilePath)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Validate the file path.
    GABLE_expect(p_FilePath != NULL, "File path is NULL!");
    if (p_FilePath[0] == '\0')
    {
        GABLE_error("File path is empty.");
        return false;
    }

    // Open the file for writing in binary mode.
    FILE* l_File = fopen(p_FilePath, "wb");
    GABLE_pexpect(l_File != NULL, "Failed to open save file '%s' for writing", p_FilePath);

    // Get the size of the SRAM buffer.
    Size l_FileSize = GABLE_RAM_SRAM_BANK_SIZE * p_RAM->m_SRAMBankCount;

    // Write the SRAM buffer to the file.
    size_t l_BytesWritten = fwrite(p_RAM->m_SRAM, 1, (Size) l_FileSize, l_File);
    if (l_BytesWritten != (Size) l_FileSize || ferror(l_File))
    {
        GABLE_perror("Failed to write SRAM buffer to save file '%s'", p_FilePath);
        fclose(l_File);
        return false;
    }

    // Close the file.
    fclose(l_File);
    return true;
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadSVBK (const GABLE_RAM* p_RAM)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Return the current working RAM bank number.
    return p_RAM->m_WRAMBankNumber;
}

Uint8 GABLE_ReadSSBK (const GABLE_RAM* p_RAM)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Return the current static RAM bank number.
    return p_RAM->m_SRAMBankNumber;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void GABLE_WriteSVBK (GABLE_RAM* p_RAM, Uint8 p_Value)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Correct the bank number if it exceeds the number of banks.
    if (p_Value >= p_RAM->m_WRAMBankCount)
    {
        p_Value = p_RAM->m_WRAMBankCount - 1;
    }

    // Update the current working RAM bank number.
    p_RAM->m_WRAMBankNumber = p_Value;
}

void GABLE_WriteSSBK (GABLE_RAM* p_RAM, Uint8 p_Value)
{
    // Validate the RAM instance.
    GABLE_expect(p_RAM != NULL, "RAM context is NULL!");

    // Correct the bank number if it exceeds the number of banks.
    if (p_Value >= p_RAM->m_SRAMBankCount)
    {
        p_Value = p_RAM->m_SRAMBankCount - 1;
    }

    // Update the current static RAM bank number.
    p_RAM->m_SRAMBankNumber = p_Value;
}
