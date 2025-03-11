/**
 * @file GABLE/RAM.c
 */

#include <GABLE/Engine.h>
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

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

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

// Public Functions - High-Level Functions /////////////////////////////////////////////////////////

void GABLE_SetSRAMBankCount (GABLE_Engine* p_Engine, Uint8 p_BankCount)
{
    // Validate the RAM instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Point to the RAM context.
    GABLE_RAM* l_RAM = GABLE_GetRAM(p_Engine);

    // If the new bank count is greater than the current bank count, then re-size the SRAM buffer.
    if (p_BankCount > l_RAM->m_SRAMBankCount)
    {
        Uint8* l_NewSRAM = GABLE_realloc(l_RAM->m_SRAM, p_BankCount * GABLE_RAM_SRAM_BANK_SIZE, Uint8);
        GABLE_pexpect(l_NewSRAM != NULL, "Failed to re-size GABLE Engine static RAM banks");

        l_RAM->m_SRAM = l_NewSRAM;
    }

    // Update the SRAM bank count.
    l_RAM->m_SRAMBankCount = p_BankCount;
}

Bool GABLE_LoadSRAMFile (GABLE_Engine* p_Engine, const Char* p_FilePath)
{
    // Validate the RAM instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Point to the RAM context.
    GABLE_RAM* l_RAM = GABLE_GetRAM(p_Engine);

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
    GABLE_SetSRAMBankCount(p_Engine, l_BankCount);

    // Read the file into the SRAM buffer.
    size_t l_BytesRead = fread(l_RAM->m_SRAM, 1, (Size) l_FileSize, l_File);
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

Bool GABLE_SaveSRAMFile (GABLE_Engine* p_Engine, const Char* p_FilePath)
{
    // Validate the RAM instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Point to the RAM context.
    GABLE_RAM* l_RAM = GABLE_GetRAM(p_Engine);

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
    Size l_FileSize = GABLE_RAM_SRAM_BANK_SIZE * l_RAM->m_SRAMBankCount;

    // Write the SRAM buffer to the file.
    size_t l_BytesWritten = fwrite(l_RAM->m_SRAM, 1, (Size) l_FileSize, l_File);
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
