/// @file       GABLE/Common.h
/// @brief      Contains commonly-used includes, constants, macros and typedefs.

#pragma once

// Include Files ///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <time.h>

// Helper Macros - Logging /////////////////////////////////////////////////////////////////////////

#define GABLE_log(p_Stream, p_Level, ...) \
    fprintf(p_Stream, "[%s] %s: ", p_Level, __func__); \
    fprintf(p_Stream, __VA_ARGS__); \
    fprintf(p_Stream, "\n");
#define GABLE_info(...) GABLE_log(stdout, "INFO", __VA_ARGS__)
#define GABLE_warn(...) GABLE_log(stderr, "WARN", __VA_ARGS__)
#define GABLE_error(...) GABLE_log(stderr, "ERROR", __VA_ARGS__)
#define GABLE_fatal(...) GABLE_log(stderr, "FATAL", __VA_ARGS__)

#define GABLE_perror(...) \
{ \
    int l_Errno = errno; \
    fprintf(stderr, "[%s] %s: ", "ERROR", __func__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, " - %s\n", strerror(l_Errno)); \
}
#define GABLE_pfatal(...) \
{ \
    int l_Errno = errno; \
    fprintf(stderr, "[%s] %s: ", "FATAL", __func__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, " - %s\n", strerror(l_Errno)); \
}

#if defined(GABLE_DEBUG)
    #define GABLE_debug(...) GABLE_log(stdout, "DEBUG", __VA_ARGS__)
    #define GABLE_trace() \
        GABLE_log(stdout, "TRACE", " - In Function '%s'", __func__); \
        GABLE_log(stdout, "TRACE", " - In File '%s:%d'", __FILE__, __LINE__);
#else
    #define GABLE_debug(...)
    #define GABLE_trace()
#endif

// Helper Macros - Error Handling //////////////////////////////////////////////////////////////////

#define GABLE_assert(p_Clause) \
    if (!(p_Clause)) \
    { \
        GABLE_fatal("Assertion Failure: '%s'!", #p_Clause); \
        GABLE_trace(); \
        abort(); \
    }
#define GABLE_expect(p_Clause, ...) \
    if (!(p_Clause)) \
    { \
        GABLE_fatal(__VA_ARGS__); \
        GABLE_trace(); \
        exit(EXIT_FAILURE); \
    }
#define GABLE_pexpect(p_Clause, ...) \
    if (!(p_Clause)) \
    { \
        GABLE_pfatal(__VA_ARGS__); \
        GABLE_trace(); \
        exit(EXIT_FAILURE); \
    }
#define GABLE_check(p_Clause, ...) \
    if (!(p_Clause)) \
    { \
        GABLE_error(__VA_ARGS__); \
        GABLE_trace(); \
        return; \
    }
#define GABLE_pcheck(p_Clause, ...) \
    if (!(p_Clause)) \
    { \
        GABLE_perror(__VA_ARGS__); \
        GABLE_trace(); \
        return; \
    }
#define GABLE_vcheck(p_Clause, p_Value, ...) \
    if (!(p_Clause)) \
    { \
        GABLE_error(__VA_ARGS__); \
        GABLE_trace(); \
        return p_Value; \
    }
#define GABLE_pvcheck(p_Clause, p_Value, ...) \
    if (!(p_Clause)) \
    { \
        GABLE_perror(__VA_ARGS__); \
        GABLE_trace(); \
        return p_Value; \
    }

// Helper Macros - Memory Management ///////////////////////////////////////////////////////////////

#define GABLE_malloc(p_Count, p_Type) \
    ((p_Type*) malloc((p_Count) * sizeof(p_Type)))
#define GABLE_calloc(p_Count, p_Type) \
    ((p_Type*) calloc((p_Count), sizeof(p_Type)))
#define GABLE_realloc(p_Ptr, p_Count, p_Type) \
    ((p_Type*) realloc((p_Ptr), (p_Count) * sizeof(p_Type)))
#define GABLE_free(p_Ptr) \
    if (p_Ptr != NULL) { free(p_Ptr); } p_Ptr = NULL;

// Helper Macros - Bit Checking and Manipulation ///////////////////////////////////////////////////

#define GABLE_bit(p_Value, p_Bit) \
    (((p_Value) >> (p_Bit)) & 0x01)
#define GABLE_setbit(p_Value, p_Bit) \
    ((p_Value) |= (0x01 << (p_Bit)))
#define GABLE_clearbit(p_Value, p_Bit) \
    ((p_Value) &= ~(0x01 << (p_Bit)))
#define GABLE_togglebit(p_Value, p_Bit) \
    ((p_Value) ^= (0x01 << (p_Bit)))
#define GABLE_changebit(p_Value, p_Bit, p_Set) \
    if (p_Set) { GABLE_setbit(p_Value, p_Bit); } \
    else { GABLE_clearbit(p_Value, p_Bit); }

// Typedefs ////////////////////////////////////////////////////////////////////////////////////////

typedef int8_t      Int8;
typedef int16_t     Int16;
typedef int32_t     Int32;
typedef int64_t     Int64;

typedef uint8_t     Uint8;
typedef uint16_t    Uint16;
typedef uint32_t    Uint32;
typedef uint64_t    Uint64;

typedef float       Float32;
typedef double      Float64;

typedef char        Char;
typedef bool        Bool;
typedef int32_t     Enum;

typedef size_t      Size;
typedef size_t      Index;
typedef size_t      Count;
typedef time_t      Time;

// Constants - GABLE Memory Map ////////////////////////////////////////////////////////////////////

#define GABLE_GB_ROM_START       0x0000
#define GABLE_GB_ROM0_START      0x0000
#define GABLE_GB_ROM0_END        0x3FFF
#define GABLE_GB_ROMX_START      0x4000
#define GABLE_GB_ROMX_END        0x7FFF
#define GABLE_GB_ROM_END         0x7FFF
#define GABLE_GB_ROM_BANK_SIZE   0x4000
#define GABLE_GB_ROM_SIZE        0x8000

#define GABLE_GB_VRAM_START      0x8000
#define GABLE_GB_VRAM_END        0x9FFF

#define GABLE_GB_SRAM_START      0xA000
#define GABLE_GB_SRAM_END        0xBFFF
#define GABLE_GB_SRAM_BANK_SIZE  0x2000

#define GABLE_GB_WRAM_START      0xC000
#define GABLE_GB_WRAM0_START     0xC000
#define GABLE_GB_WRAM0_END       0xCFFF
#define GABLE_GB_WRAMX_START     0xD000
#define GABLE_GB_WRAMX_END       0xDFFF
#define GABLE_GB_WRAM_END        0xDFFF
#define GABLE_GB_WRAM_BANK_SIZE  0x1000
#define GABLE_GB_WRAM_SIZE       0x2000

#define GABLE_NETRAM_START       0xE000
#define GABLE_NETRAM_END         0xE0FF
#define GABLE_NETRAM_BANK_SIZE   0x0100
#define GABLE_NETRAM_SIZE        0x1000

#define GABLE_GB_ECHO_START      0xE100
#define GABLE_GB_ECHO_END        0xFDFF

#define GABLE_GB_OAM_START       0xFE00
#define GABLE_GB_OAM_END         0xFE9F

#define GABLE_GB_WAVE_START      0xFF30
#define GABLE_GB_WAVE_END        0xFF3F

#define GABLE_GB_IO_START        0xFF00
#define GABLE_GB_IO_END          0xFF7F

#define GABLE_GB_HRAM_START      0xFF80
#define GABLE_GB_HRAM_END        0xFFFE

// Register and Flag Type Enumerations /////////////////////////////////////////////////////////////

/**
 * @brief Enumerates the 8-bit and 16-bit registers of the Sharp LR35902 CPU.
 * 
 * @note Because the GABLE Engine is intended to create "Gameboy-like" games, and is not an emulator,
 *       the program counter is not included in this enumeration.
 */
typedef enum GABLE_RegisterType
{
    GABLE_RT_A,     ///< @brief The CPU's 8-bit accumulator register.
    GABLE_RT_F,     ///< @brief The CPU's 8-bit flags register.
    GABLE_RT_B,     ///< @brief One of the CPU's 8-bit general-purpose registers.
    GABLE_RT_C,     ///< @brief One of the CPU's 8-bit general-purpose registers.
    GABLE_RT_D,     ///< @brief One of the CPU's 8-bit general-purpose registers.
    GABLE_RT_E,     ///< @brief One of the CPU's 8-bit general-purpose registers.
    GABLE_RT_H,     ///< @brief One of the CPU's 8-bit general-purpose registers.
    GABLE_RT_L,     ///< @brief One of the CPU's 8-bit general-purpose registers.

    GABLE_RT_AF,    ///< @brief One of the CPU's 16-bit register pairs.
    GABLE_RT_BC,    ///< @brief One of the CPU's 16-bit register pairs.
    GABLE_RT_DE,    ///< @brief One of the CPU's 16-bit register pairs.
    GABLE_RT_HL,    ///< @brief One of the CPU's 16-bit register pairs.

    GABLE_RT_SP,    ///< @brief The CPU's 16-bit stack pointer register.
} GABLE_RegisterType;

/**
 * @brief Enumerates the flags of the Sharp LR35902 CPU.
 * 
 * @note The flags register is an 8-bit register, with the following layout:
 * 
 * @code
 *  7 6 5 4 3 2 1 0
 *  Z N H C 0 0 0 0
 * @endcode
 */
typedef enum GABLE_FlagType
{
    GABLE_FT_Z = 7, ///< @brief Zero Flag
    GABLE_FT_N = 6, ///< @brief Subtract Flag
    GABLE_FT_H = 5, ///< @brief Half-Carry Flag
    GABLE_FT_C = 4, ///< @brief Carry Flag
} GABLE_FlagType;

/**
 * @brief Enumerates the condition types for the Sharp LR35902 CPU's control-transfer instructions.
 */
typedef enum GABLE_ConditionType
{
    GABLE_CT_NONE,  ///< @brief No condition.
    GABLE_CT_NZ,    ///< @brief Not Zero.
    GABLE_CT_Z,     ///< @brief Zero.
    GABLE_CT_NC,    ///< @brief Not Carry.
    GABLE_CT_C,     ///< @brief Carry.
} GABLE_ConditionType;

// Hardware Port Register Enumeration //////////////////////////////////////////////////////////////

/// @brief Enumerates the hardware ports of the Nintendo Game Boy, the Game Boy Color, and
///        emulated hardware exclusive to the GABLE Engine.
typedef enum GABLE_HardwarePort
{
    GABLE_HP_JOYP      = 0xFF00,    ///< @brief `JOYP` / `P1` - Joypad Register
    GABLE_HP_NTS       = 0xFF01,    ///< @brief `NTS` - GABLE Only - Network Transfer Size
    GABLE_HP_NTC       = 0xFF02,    ///< @brief `NTC` - GABLE Only - Network Transfer Control
    GABLE_HP_DIV       = 0xFF04,    ///< @brief `DIV` - Timer Divider Register
    GABLE_HP_TIMA      = 0xFF05,    ///< @brief `TIMA` - Timer Counter
    GABLE_HP_TMA       = 0xFF06,    ///< @brief `TMA` - Timer Modulo
    GABLE_HP_TAC       = 0xFF07,    ///< @brief `TAC` - Timer Control
    GABLE_HP_RTCS      = 0xFF08,    ///< @brief `RTCS` - GABLE Only - Real-Time Clock Seconds
    GABLE_HP_RTCM      = 0xFF09,    ///< @brief `RTCM` - GABLE Only - Real-Time Clock Minutes
    GABLE_HP_RTCH      = 0xFF0A,    ///< @brief `RTCH` - GABLE Only - Real-Time Clock Hours
    GABLE_HP_RTCDH     = 0xFF0B,    ///< @brief `RTCD` - GABLE Only - Real-Time Clock Day Counter, High Byte
    GABLE_HP_RTCDL     = 0xFF0C,    ///< @brief `RTCL` - GABLE Only - Real-Time Clock Day Counter, Low Byte
    GABLE_HP_RTCL      = 0xFF0D,    ///< @brief `RTCL` - GABLE Only - Real-Time Clock Latch
    GABLE_HP_IF        = 0xFF0F,    ///< @brief `IF` - Interrupt Flag
    GABLE_HP_NR10      = 0xFF10,    ///< @brief `NR10` - Pulse Channel 1 Frequency Sweep Control
    GABLE_HP_NR11      = 0xFF11,    ///< @brief `NR11` - Pulse Channel 1 Sound Length/Wave Pattern Duty
    GABLE_HP_NR12      = 0xFF12,    ///< @brief `NR12` - Pulse Channel 1 Volume Envelope
    GABLE_HP_NR13      = 0xFF13,    ///< @brief `NR13` - Pulse Channel 1 Frequency Low
    GABLE_HP_NR14      = 0xFF14,    ///< @brief `NR14` - Pulse Channel 1 Frequency High + Control
    GABLE_HP_NR21      = 0xFF16,    ///< @brief `NR21` - Pulse Channel 2 Sound Length/Wave Pattern Duty
    GABLE_HP_NR22      = 0xFF17,    ///< @brief `NR22` - Pulse Channel 2 Volume Envelope
    GABLE_HP_NR23      = 0xFF18,    ///< @brief `NR23` - Pulse Channel 2 Frequency Low
    GABLE_HP_NR24      = 0xFF19,    ///< @brief `NR24` - Pulse Channel 2 Frequency High + Control
    GABLE_HP_NR30      = 0xFF1A,    ///< @brief `NR30` - Wave Channel DAC Power Control
    GABLE_HP_NR31      = 0xFF1B,    ///< @brief `NR31` - Wave Channel Sound Length
    GABLE_HP_NR32      = 0xFF1C,    ///< @brief `NR32` - Wave Channel Volume Control
    GABLE_HP_NR33      = 0xFF1D,    ///< @brief `NR33` - Wave Channel Frequency Low
    GABLE_HP_NR34      = 0xFF1E,    ///< @brief `NR34` - Wave Channel Frequency High + Control
    GABLE_HP_NR41      = 0xFF20,    ///< @brief `NR41` - Noise Channel Sound Length
    GABLE_HP_NR42      = 0xFF21,    ///< @brief `NR42` - Noise Channel Volume Envelope
    GABLE_HP_NR43      = 0xFF22,    ///< @brief `NR43` - Noise Channel LFSR Control
    GABLE_HP_NR44      = 0xFF23,    ///< @brief `NR44` - Noise Channel Frequency Control
    GABLE_HP_NR50      = 0xFF24,    ///< @brief `NR50` - Channel Control / Volume
    GABLE_HP_NR51      = 0xFF25,    ///< @brief `NR51` - Selection of Sound Output Terminal
    GABLE_HP_NR52      = 0xFF26,    ///< @brief `NR52` - Sound On/Off
    GABLE_HP_LCDC      = 0xFF40,    ///< @brief `LCDC` - LCD Control / Display Control
    GABLE_HP_STAT      = 0xFF41,    ///< @brief `STAT` - LCD Status / Display Status
    GABLE_HP_SCY       = 0xFF42,    ///< @brief `SCY` - Background Viewport Scroll Y
    GABLE_HP_SCX       = 0xFF43,    ///< @brief `SCX` - Background Viewport Scroll X
    GABLE_HP_LY        = 0xFF44,    ///< @brief `LY` - LCDC Y-Coordinate / Current Scanline
    GABLE_HP_LYC       = 0xFF45,    ///< @brief `LYC` - LY Compare / Line Compare
    GABLE_HP_DMA       = 0xFF46,    ///< @brief `DMA` - OAM DMA Transfer and Start Address
    GABLE_HP_BGP       = 0xFF47,    ///< @brief `BGP` - DMG Only - Background Palette Data
    GABLE_HP_OBP0      = 0xFF48,    ///< @brief `OBP0` - DMG Only - Object Palette 0 Data
    GABLE_HP_OBP1      = 0xFF49,    ///< @brief `OBP1` - DMG Only - Object Palette 1 Data
    GABLE_HP_WY        = 0xFF4A,    ///< @brief `WY` - Window Position Y
    GABLE_HP_WX        = 0xFF4B,    ///< @brief `WX` - Window Position X
    GABLE_HP_KEY1      = 0xFF4D,    ///< @brief `KEY1` - CGB Only - Prepare Speed Switch
    GABLE_HP_VBK       = 0xFF4F,    ///< @brief `VBK` - CGB Only - VRAM Bank Number
    GABLE_HP_HDMA1     = 0xFF51,    ///< @brief `HDMA1` - CGB Only - GDMA / HDMA Source, High Byte
    GABLE_HP_HDMA2     = 0xFF52,    ///< @brief `HDMA2` - CGB Only - GDMA / HDMA Source, Low Byte
    GABLE_HP_HDMA3     = 0xFF53,    ///< @brief `HDMA3` - CGB Only - GDMA / HDMA Destination, High Byte
    GABLE_HP_HDMA4     = 0xFF54,    ///< @brief `HDMA4` - CGB Only - GDMA / HDMA Destination, Low Byte
    GABLE_HP_HDMA5     = 0xFF55,    ///< @brief `HDMA5` - CGB Only - GDMA / HDMA Transfer, Mode, Start
    GABLE_HP_RP        = 0xFF56,    ///< @brief `RP` - CGB Only - Infrared Communications Port
    GABLE_HP_BGPI      = 0xFF68,    ///< @brief `BGPI` - CGB Only - Background Palette Index
    GABLE_HP_BGPD      = 0xFF69,    ///< @brief `BGPD` - CGB Only - Background Palette Data
    GABLE_HP_OBPI      = 0xFF6A,    ///< @brief `OBPI` - CGB Only - Object Palette Index
    GABLE_HP_OBPD      = 0xFF6B,    ///< @brief `OBPD` - CGB Only - Object Palette Data
    GABLE_HP_OPRI      = 0xFF6C,    ///< @brief `OPRI` - CGB Only - Object Priority
    GABLE_HP_GRPM      = 0xFF6D,    ///< @brief `GRPM` - GABLE Only - PPU Graphics Mode (0=DMG, 1=CGB)
    GABLE_HP_SVBK      = 0xFF70,    ///< @brief `SVBK` - CGB Only - WRAM Bank Number
    GABLE_HP_SSBK      = 0xFF71,    ///< @brief `SSBK` - GABLE Only - SRAM Bank Number
    GABLE_HP_DSBKH     = 0xFF72,    ///< @brief `DSBKH` - GABLE Only - Data Store Bank Number, High Byte
    GABLE_HP_DSBKL     = 0xFF73,    ///< @brief `DSBKL` - GABLE Only - Data Store Bank Number, Low Byte
    GABLE_HP_PCM12     = 0xFF76,    ///< @brief `PCM12` - CGB Only - PCM Channel 1/2 Output
    GABLE_HP_PCM34     = 0xFF77,    ///< @brief `PCM34` - CGB Only - PCM Channel 3/4 Output
    GABLE_HP_IE        = 0xFFFF     ///< @brief `IE` - Interrupt Enable
} GABLE_HardwarePort;
