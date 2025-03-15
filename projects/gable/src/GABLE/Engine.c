/**
 * @file GABLE/Engine.c
 */

#include <GABLE/InterruptContext.h>
#include <GABLE/Timer.h>
#include <GABLE/Realtime.h>
#include <GABLE/DataStore.h>
#include <GABLE/RAM.h>
#include <GABLE/APU.h>
#include <GABLE/PPU.h>
#include <GABLE/Joypad.h>
#include <GABLE/Network.h>
#include <GABLE/Engine.h>

// GABLE Engine Structure //////////////////////////////////////////////////////////////////////////

typedef struct GABLE_Engine
{
    Uint64                  m_Cycles;       ///< @brief The number of cycles elapsed on the engine.
    GABLE_InterruptContext* m_Interrupts;   ///< @brief The engine's interrupt context.
    GABLE_Timer*            m_Timer;        ///< @brief The engine's timer.
    GABLE_Realtime*         m_Realtime;     ///< @brief The engine's real-time clock.
    GABLE_DataStore*        m_DataStore;    ///< @brief The engine's data store.
    GABLE_RAM*              m_RAM;          ///< @brief The engine's RAM.
    GABLE_APU*              m_APU;          ///< @brief The engine's APU.
    GABLE_PPU*              m_PPU;          ///< @brief The engine's PPU.
    GABLE_Joypad*           m_Joypad;       ///< @brief The engine's joypad.
    GABLE_NetworkContext*   m_Network;      ///< @brief The engine's network interface.
    void*                   m_Userdata;     ///< @brief User data associated with the engine.
} GABLE_Engine;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_Engine* GABLE_CreateEngine ()
{
    // Allocate the GABLE Engine instance.
    GABLE_Engine* l_Engine = GABLE_calloc(1, GABLE_Engine);
    GABLE_pexpect(l_Engine != NULL, "Failed to allocate GABLE Engine");

    // Create the engine's components.
    l_Engine->m_Interrupts = GABLE_CreateInterruptContext();
    l_Engine->m_Timer = GABLE_CreateTimer();
    l_Engine->m_Realtime = GABLE_CreateRealtime();
    l_Engine->m_DataStore = GABLE_CreateDataStore();
    l_Engine->m_RAM = GABLE_CreateRAM();
    l_Engine->m_APU = GABLE_CreateAPU();
    l_Engine->m_PPU = GABLE_CreatePPU();
    l_Engine->m_Joypad = GABLE_CreateJoypad(l_Engine);
    l_Engine->m_Network = GABLE_CreateNetworkContext();

    // Initialize the engine's properties.
    l_Engine->m_Cycles = 0;

    // Return the new engine instance.
    return l_Engine;
}

void GABLE_DestroyEngine (GABLE_Engine* p_Engine)
{
    if (p_Engine != NULL)
    {
        // Un-set the user data pointer.
        p_Engine->m_Userdata = NULL;

        // Destroy the engine's components.
        GABLE_DestroyNetworkContext(p_Engine->m_Network);
        GABLE_DestroyInterruptContext(p_Engine->m_Interrupts);
        GABLE_DestroyTimer(p_Engine->m_Timer);
        GABLE_DestroyRealtime(p_Engine->m_Realtime);
        GABLE_DestroyDataStore(p_Engine->m_DataStore);
        GABLE_DestroyRAM(p_Engine->m_RAM);
        GABLE_DestroyAPU(p_Engine->m_APU);
        GABLE_DestroyPPU(p_Engine->m_PPU);
        GABLE_DestroyJoypad(p_Engine->m_Joypad);

        // Free the engine instance.
        GABLE_free(p_Engine);
    }
}

Bool GABLE_CycleEngine (GABLE_Engine* p_Engine, Count p_Cycles)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    for (Count i = 0; i < p_Cycles; i++)
    {
        for (Count j = 0; j < 4; j++)
        {
            // Elapse a cycle on the engine.
            p_Engine->m_Cycles++;

            // Tick the engine's components.
            GABLE_TickTimer(p_Engine->m_Timer, p_Engine);
            GABLE_TickAPU(p_Engine->m_APU, p_Engine);
            GABLE_TickPPU(p_Engine->m_PPU, p_Engine);
            GABLE_TickNetworkContext(p_Engine->m_Network, p_Engine);

            // Service an interrupt if requested.
            if (GABLE_ServiceInterrupt(p_Engine->m_Interrupts, p_Engine) == -1)
            {
                // Return failure.
                return false;
            }
        }

        // Tick the PPU's OAM DMA transfer.
        GABLE_TickODMA(p_Engine->m_PPU, p_Engine);
    }

    // Return success.
    return true;
}

Bool GABLE_ReadByte (GABLE_Engine* p_Engine, Uint16 p_Address, Uint8* p_Value)
{
    // Validate the engine instance and value pointer.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");
    GABLE_expect(p_Value != NULL, "Value pointer is NULL!");

    // `0x0000` - `0x7FFF`: Read from the data store.
    if (p_Address <= GABLE_GB_ROM_END)
    {
        return GABLE_ReadDataStoreByte(p_Engine->m_DataStore, p_Address, p_Value);
    }

    // `0x8000` - `0x9FFF`: Read from the video RAM.
    if (p_Address >= GABLE_GB_VRAM_START && p_Address <= GABLE_GB_VRAM_END)
    {
        return GABLE_ReadVRAMByte(p_Engine->m_PPU, p_Address - GABLE_GB_VRAM_START, p_Value);
    }

    // `0xA000` - `0xBFFF`: Read from the static RAM.
    if (p_Address >= GABLE_GB_SRAM_START && p_Address <= GABLE_GB_SRAM_END)
    {
        return GABLE_ReadSRAMByte(p_Engine->m_RAM, p_Address - GABLE_GB_SRAM_START, p_Value);
    }

    // `0xC000` - `0xDFFF`: Read from the working RAM.
    if (p_Address >= GABLE_GB_WRAM_START && p_Address <= GABLE_GB_WRAM_END)
    {
        return GABLE_ReadWRAMByte(p_Engine->m_RAM, p_Address - GABLE_GB_WRAM_START, p_Value);
    }

    // `0xE000` - `0xE100`: Read from the network RAM.
    if (p_Address >= GABLE_NETRAM_START && p_Address <= GABLE_NETRAM_END)
    {
        return GABLE_ReadNetworkRAMByte(p_Engine->m_Network, p_Address - GABLE_NETRAM_START, p_Value);
    }

    // `0xE100` - `0xFDFF`: Read from the working RAM (echo).
    if (p_Address >= GABLE_GB_ECHO_START && p_Address <= GABLE_GB_ECHO_END)
    {
        return GABLE_ReadWRAMByte(p_Engine->m_RAM, p_Address - GABLE_GB_ECHO_START, p_Value);
    }

    // `0xFE00` - `0xFE9F`: Read from the object attribute memory.
    if (p_Address >= GABLE_GB_OAM_START && p_Address <= GABLE_GB_OAM_END)
    {
        return GABLE_ReadOAMByte(p_Engine->m_PPU, p_Address - GABLE_GB_OAM_START, p_Value);
    }

    // `0xFF30` - `0xFF3F`: Read from the wave pattern RAM.
    if (p_Address >= GABLE_GB_WAVE_START && p_Address <= GABLE_GB_WAVE_END)
    {
        return GABLE_ReadWaveByte(p_Engine->m_APU, p_Address - GABLE_GB_WAVE_START, p_Value);
    }

    // `0xFF80` - `0xFFFE`: Read from the high RAM buffer.
    if (p_Address >= GABLE_GB_HRAM_START && p_Address <= GABLE_GB_HRAM_END)
    {
        return GABLE_ReadHRAMByte(p_Engine->m_RAM, p_Address - GABLE_GB_HRAM_START, p_Value);
    }

    // If we reach this point, then we must be reading from a hardware port.
    switch (p_Address)
    {
        case GABLE_HP_JOYP:     *p_Value = GABLE_ReadJOYP(p_Engine->m_Joypad); break;
        case GABLE_HP_NTS:      *p_Value = GABLE_ReadNTS(p_Engine->m_Network); break;
        case GABLE_HP_NTC:      *p_Value = GABLE_ReadNTC(p_Engine->m_Network); break;
        case GABLE_HP_DIV:      *p_Value = GABLE_ReadDIV(p_Engine->m_Timer); break;
        case GABLE_HP_TIMA:     *p_Value = GABLE_ReadTIMA(p_Engine->m_Timer); break;
        case GABLE_HP_TMA:      *p_Value = GABLE_ReadTMA(p_Engine->m_Timer); break;
        case GABLE_HP_TAC:      *p_Value = GABLE_ReadTAC(p_Engine->m_Timer); break;
        case GABLE_HP_RTCS:     *p_Value = GABLE_ReadRTCS(p_Engine->m_Realtime); break;
        case GABLE_HP_RTCM:     *p_Value = GABLE_ReadRTCM(p_Engine->m_Realtime); break;
        case GABLE_HP_RTCH:     *p_Value = GABLE_ReadRTCH(p_Engine->m_Realtime); break;
        case GABLE_HP_RTCDL:    *p_Value = GABLE_ReadRTCDL(p_Engine->m_Realtime); break;
        case GABLE_HP_RTCDH:    *p_Value = GABLE_ReadRTCDH(p_Engine->m_Realtime); break;
        case GABLE_HP_RTCL:     *p_Value = 0xFF; break; // Write-only register.
        case GABLE_HP_IF:       *p_Value = GABLE_ReadIF(p_Engine->m_Interrupts); break;
        case GABLE_HP_NR10:     *p_Value = GABLE_ReadNR10(p_Engine->m_APU); break;
        case GABLE_HP_NR11:     *p_Value = GABLE_ReadNR11(p_Engine->m_APU); break;
        case GABLE_HP_NR12:     *p_Value = GABLE_ReadNR12(p_Engine->m_APU); break;
        case GABLE_HP_NR13:     *p_Value = 0xFF; break; // Write-only register.
        case GABLE_HP_NR14:     *p_Value = GABLE_ReadNR14(p_Engine->m_APU); break;
        case GABLE_HP_NR21:     *p_Value = GABLE_ReadNR21(p_Engine->m_APU); break;
        case GABLE_HP_NR22:     *p_Value = GABLE_ReadNR22(p_Engine->m_APU); break;
        case GABLE_HP_NR23:     *p_Value = 0xFF; break; // Write-only register.
        case GABLE_HP_NR24:     *p_Value = GABLE_ReadNR24(p_Engine->m_APU); break;
        case GABLE_HP_NR30:     *p_Value = GABLE_ReadNR30(p_Engine->m_APU); break;
        case GABLE_HP_NR31:     *p_Value = 0xFF; break; // Write-only register.
        case GABLE_HP_NR32:     *p_Value = GABLE_ReadNR32(p_Engine->m_APU); break;
        case GABLE_HP_NR33:     *p_Value = 0xFF; break; // Write-only register.
        case GABLE_HP_NR34:     *p_Value = GABLE_ReadNR34(p_Engine->m_APU); break;
        case GABLE_HP_NR41:     *p_Value = 0xFF; break; // Write-only register.
        case GABLE_HP_NR42:     *p_Value = GABLE_ReadNR42(p_Engine->m_APU); break;
        case GABLE_HP_NR43:     *p_Value = GABLE_ReadNR43(p_Engine->m_APU); break;
        case GABLE_HP_NR44:     *p_Value = GABLE_ReadNR44(p_Engine->m_APU); break;
        case GABLE_HP_NR50:     *p_Value = GABLE_ReadNR50(p_Engine->m_APU); break;
        case GABLE_HP_NR51:     *p_Value = GABLE_ReadNR51(p_Engine->m_APU); break;
        case GABLE_HP_NR52:     *p_Value = GABLE_ReadNR52(p_Engine->m_APU); break;
        case GABLE_HP_LCDC:     *p_Value = GABLE_ReadLCDC(p_Engine->m_PPU); break;
        case GABLE_HP_STAT:     *p_Value = GABLE_ReadSTAT(p_Engine->m_PPU); break;
        case GABLE_HP_SCY:      *p_Value = GABLE_ReadSCY(p_Engine->m_PPU); break;
        case GABLE_HP_SCX:      *p_Value = GABLE_ReadSCX(p_Engine->m_PPU); break;
        case GABLE_HP_LY:       *p_Value = GABLE_ReadLY(p_Engine->m_PPU); break;
        case GABLE_HP_LYC:      *p_Value = GABLE_ReadLYC(p_Engine->m_PPU); break;
        case GABLE_HP_DMA:      *p_Value = GABLE_ReadDMA(p_Engine->m_PPU); break;
        case GABLE_HP_BGP:      *p_Value = GABLE_ReadBGP(p_Engine->m_PPU); break;
        case GABLE_HP_OBP0:     *p_Value = GABLE_ReadOBP0(p_Engine->m_PPU); break;
        case GABLE_HP_OBP1:     *p_Value = GABLE_ReadOBP1(p_Engine->m_PPU); break;
        case GABLE_HP_WY:       *p_Value = GABLE_ReadWY(p_Engine->m_PPU); break;
        case GABLE_HP_VBK:      *p_Value = GABLE_ReadVBK(p_Engine->m_PPU); break;
        case GABLE_HP_HDMA1:    *p_Value = 0xFF; break; // Write-only register.
        case GABLE_HP_HDMA2:    *p_Value = 0xFF; break; // Write-only register.
        case GABLE_HP_HDMA3:    *p_Value = 0xFF; break; // Write-only register.
        case GABLE_HP_HDMA4:    *p_Value = 0xFF; break; // Write-only register.
        case GABLE_HP_HDMA5:    *p_Value = GABLE_ReadHDMA5(p_Engine->m_PPU); break;
        case GABLE_HP_BGPI:     *p_Value = GABLE_ReadBGPI(p_Engine->m_PPU); break;
        case GABLE_HP_OBPI:     *p_Value = GABLE_ReadOBPI(p_Engine->m_PPU); break;
        case GABLE_HP_BGPD:     *p_Value = GABLE_ReadBGPD(p_Engine->m_PPU); break;
        case GABLE_HP_OBPD:     *p_Value = GABLE_ReadOBPD(p_Engine->m_PPU); break;
        case GABLE_HP_OPRI:     *p_Value = GABLE_ReadOPRI(p_Engine->m_PPU); break;
        case GABLE_HP_GRPM:     *p_Value = GABLE_ReadGRPM(p_Engine->m_PPU); break;
        case GABLE_HP_SVBK:     *p_Value = GABLE_ReadSVBK(p_Engine->m_RAM); break;
        case GABLE_HP_SSBK:     *p_Value = GABLE_ReadSSBK(p_Engine->m_RAM); break;
        case GABLE_HP_DSBKH:    *p_Value = GABLE_ReadDSBKH(p_Engine->m_DataStore); break;
        case GABLE_HP_DSBKL:    *p_Value = GABLE_ReadDSBKL(p_Engine->m_DataStore); break;
        case GABLE_HP_IE:       *p_Value = GABLE_ReadIE(p_Engine->m_Interrupts); break;
        default: 
            GABLE_error("Attempted read from invalid address: $%04X", p_Address);
            return false;
    }

    return true;
}

Bool GABLE_ReadWord (GABLE_Engine* p_Engine, Uint16 p_Address, Uint16* p_Value)
{
    // Validate the engine instance and value pointer.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");
    GABLE_expect(p_Value != NULL, "Value pointer is NULL!");

    // Read two bytes from the memory map, in big-endian order.
    Uint8 l_Byte0 = 0;
    Uint8 l_Byte1 = 0;

    if (!GABLE_ReadByte(p_Engine, p_Address, &l_Byte0)) { return false; }
    if (!GABLE_ReadByte(p_Engine, p_Address + 1, &l_Byte1)) { return false; }

    // Combine the bytes into a word.
    *p_Value = (l_Byte1 << 8) | l_Byte0;
    return true;
}

Bool GABLE_WriteByte (GABLE_Engine* p_Engine, Uint16 p_Address, Uint8 p_Value)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // `0x8000` - `0x9FFF`: Write to the video RAM.
    if (p_Address >= GABLE_GB_VRAM_START && p_Address <= GABLE_GB_VRAM_END)
    {
        return GABLE_WriteVRAMByte(p_Engine->m_PPU, p_Address - GABLE_GB_VRAM_START, p_Value);
    }

    // `0xA000` - `0xBFFF`: Write to the static RAM.
    if (p_Address >= GABLE_GB_SRAM_START && p_Address <= GABLE_GB_SRAM_END)
    {
        return GABLE_WriteSRAMByte(p_Engine->m_RAM, p_Address - GABLE_GB_SRAM_START, p_Value);
    }

    // `0xC000` - `0xDFFF`: Write to the working RAM.
    if (p_Address >= GABLE_GB_WRAM_START && p_Address <= GABLE_GB_WRAM_END)
    {
        return GABLE_WriteWRAMByte(p_Engine->m_RAM, p_Address - GABLE_GB_WRAM_START, p_Value);
    }

    // `0xE000` - `0xE100`: Write to the network RAM.
    if (p_Address >= GABLE_NETRAM_START && p_Address <= GABLE_NETRAM_END)
    {
        return GABLE_WriteNetworkRAMByte(p_Engine->m_Network, p_Address - GABLE_NETRAM_START, p_Value);
    }

    // `0xE100` - `0xFDFF`: Write to the working RAM (echo).
    if (p_Address >= GABLE_GB_ECHO_START && p_Address <= GABLE_GB_ECHO_END)
    {
        return GABLE_WriteWRAMByte(p_Engine->m_RAM, p_Address - GABLE_GB_ECHO_START, p_Value);
    }

    // `0xFE00` - `0xFE9F`: Write to the object attribute memory.
    if (p_Address >= GABLE_GB_OAM_START && p_Address <= GABLE_GB_OAM_END)
    {
        return GABLE_WriteOAMByte(p_Engine->m_PPU, p_Address - GABLE_GB_OAM_START, p_Value);
    }

    // `0xFF30` - `0xFF3F`: Write to the wave pattern RAM.
    if (p_Address >= GABLE_GB_WAVE_START && p_Address <= GABLE_GB_WAVE_END)
    {
        return GABLE_WriteWaveByte(p_Engine->m_APU, p_Address - GABLE_GB_WAVE_START, p_Value);
    }

    // `0xFF80` - `0xFFFE`: Write to the high RAM buffer.
    if (p_Address >= GABLE_GB_HRAM_START && p_Address <= GABLE_GB_HRAM_END)
    {
        return GABLE_WriteHRAMByte(p_Engine->m_RAM, p_Address - GABLE_GB_HRAM_START, p_Value);
    }

    // If we reach this point, then we must be writing to a hardware port.
    switch (p_Address)
    {
        case GABLE_HP_JOYP:     GABLE_WriteJOYP(p_Engine->m_Joypad, p_Value); break;
        case GABLE_HP_NTS:      GABLE_WriteNTS(p_Engine->m_Network, p_Value); break;
        case GABLE_HP_NTC:      GABLE_WriteNTC(p_Engine->m_Network, p_Value); break;
        case GABLE_HP_DIV:      GABLE_WriteDIV(p_Engine->m_Timer, p_Value); break;
        case GABLE_HP_TIMA:     GABLE_WriteTIMA(p_Engine->m_Timer, p_Value); break;
        case GABLE_HP_TMA:      GABLE_WriteTMA(p_Engine->m_Timer, p_Value); break;
        case GABLE_HP_TAC:      GABLE_WriteTAC(p_Engine->m_Timer, p_Value); break;
        case GABLE_HP_RTCS:     break; // Read-only register.
        case GABLE_HP_RTCM:     break; // Read-only register.
        case GABLE_HP_RTCH:     break; // Read-only register.
        case GABLE_HP_RTCDL:    break; // Read-only register.
        case GABLE_HP_RTCDH:    break; // Read-only register.
        case GABLE_HP_RTCL:     GABLE_WriteRTCL(p_Engine->m_Realtime, p_Engine, p_Value); break;
        case GABLE_HP_IF:       GABLE_WriteIF(p_Engine->m_Interrupts, p_Value); break;
        case GABLE_HP_NR10:     GABLE_WriteNR10(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR11:     GABLE_WriteNR11(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR12:     GABLE_WriteNR12(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR13:     GABLE_WriteNR13(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR14:     GABLE_WriteNR14(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR21:     GABLE_WriteNR21(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR22:     GABLE_WriteNR22(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR23:     GABLE_WriteNR23(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR24:     GABLE_WriteNR24(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR30:     GABLE_WriteNR30(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR31:     GABLE_WriteNR31(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR32:     GABLE_WriteNR32(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR33:     GABLE_WriteNR33(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR34:     GABLE_WriteNR34(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR41:     GABLE_WriteNR41(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR42:     GABLE_WriteNR42(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR43:     GABLE_WriteNR43(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR44:     GABLE_WriteNR44(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR50:     GABLE_WriteNR50(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR51:     GABLE_WriteNR51(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_NR52:     GABLE_WriteNR52(p_Engine->m_APU, p_Value); break;
        case GABLE_HP_LCDC:     GABLE_WriteLCDC(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_STAT:     GABLE_WriteSTAT(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_SCY:      GABLE_WriteSCY(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_SCX:      GABLE_WriteSCX(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_LY:       break; // Read-only register.
        case GABLE_HP_LYC:      GABLE_WriteLYC(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_DMA:      GABLE_WriteDMA(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_BGP:      GABLE_WriteBGP(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_OBP0:     GABLE_WriteOBP0(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_OBP1:     GABLE_WriteOBP1(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_WY:       GABLE_WriteWY(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_WX:       GABLE_WriteWX(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_VBK:      GABLE_WriteVBK(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_HDMA1:    GABLE_WriteHDMA1(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_HDMA2:    GABLE_WriteHDMA2(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_HDMA3:    GABLE_WriteHDMA3(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_HDMA4:    GABLE_WriteHDMA4(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_HDMA5:    GABLE_WriteHDMA5(p_Engine->m_PPU, p_Engine, p_Value); break;
        case GABLE_HP_BGPI:     GABLE_WriteBGPI(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_OBPI:     GABLE_WriteOBPI(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_BGPD:     GABLE_WriteBGPD(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_OBPD:     GABLE_WriteOBPD(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_OPRI:     GABLE_WriteOPRI(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_GRPM:     GABLE_WriteGRPM(p_Engine->m_PPU, p_Value); break;
        case GABLE_HP_SVBK:     GABLE_WriteSVBK(p_Engine->m_RAM, p_Value); break;
        case GABLE_HP_SSBK:     GABLE_WriteSSBK(p_Engine->m_RAM, p_Value); break;
        case GABLE_HP_DSBKH:    GABLE_WriteDSBKH(p_Engine->m_DataStore, p_Value); break;
        case GABLE_HP_DSBKL:    GABLE_WriteDSBKL(p_Engine->m_DataStore, p_Value); break;
        case GABLE_HP_IE:       GABLE_WriteIE(p_Engine->m_Interrupts, p_Value); break;
        default: 
            GABLE_error("Attempted write to invalid address: $%04X", p_Address);
            return false;
    }

    return true;
}

Bool GABLE_WriteWord (GABLE_Engine* p_Engine, Uint16 p_Address, Uint16 p_Value)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Write two bytes to the memory map, in big-endian order.
    Uint8 l_Byte0 = (p_Value & 0x00FF);
    Uint8 l_Byte1 = (p_Value & 0xFF00) >> 8;

    if (!GABLE_WriteByte(p_Engine, p_Address, l_Byte0)) { return false; }
    if (!GABLE_WriteByte(p_Engine, p_Address + 1, l_Byte1)) { return false; }

    return true;
}

Uint64 GABLE_GetCycleCount (const GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the engine's cycle count.
    return p_Engine->m_Cycles;
}

// Public Functions - Component Getters ////////////////////////////////////////////////////////////

GABLE_InterruptContext* GABLE_GetInterruptContext (GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the engine's interrupt context.
    return p_Engine->m_Interrupts;
}

GABLE_Timer* GABLE_GetTimer (GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the engine's timer.
    return p_Engine->m_Timer;
}

GABLE_Realtime* GABLE_GetRealtime (GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the engine's real-time clock.
    return p_Engine->m_Realtime;
}

GABLE_DataStore* GABLE_GetDataStore (GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the engine's data store.
    return p_Engine->m_DataStore;
}

GABLE_RAM* GABLE_GetRAM (GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the engine's RAM.
    return p_Engine->m_RAM;
}

GABLE_APU* GABLE_GetAPU (GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the engine's APU.
    return p_Engine->m_APU;
}

GABLE_PPU* GABLE_GetPPU (GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the engine's PPU.
    return p_Engine->m_PPU;
}

GABLE_Joypad* GABLE_GetJoypad (GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the engine's joypad.
    return p_Engine->m_Joypad;
}

GABLE_NetworkContext* GABLE_GetNetwork (GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the engine's network interface.
    return p_Engine->m_Network;
}

// Public Functions - User Data ////////////////////////////////////////////////////////////////////

void* GABLE_GetUserdata (GABLE_Engine* p_Engine)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Return the user data pointer.
    return p_Engine->m_Userdata;
}

void GABLE_SetUserdata (GABLE_Engine* p_Engine, void* p_Userdata)
{
    // Validate the engine instance.
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Set the user data pointer.
    p_Engine->m_Userdata = p_Userdata;
}
