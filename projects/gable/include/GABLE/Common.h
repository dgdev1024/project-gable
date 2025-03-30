/// @file       GABLE/Common.h
/// @brief      Contains commonly-used includes, constants, macros and typedefs.

#pragma once

// Include Files ///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
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
#define GABLE_GB_TDATA_START     0x8000
#define GABLE_GB_TDATA0_START    0x8000
#define GABLE_GB_TDATA1_START    0x8800
#define GABLE_GB_TDATA2_START    0x9000
#define GABLE_GB_TDATA_END       0x97FF
#define GABLE_GB_SCRN_START      0x9800
#define GABLE_GB_SCRN0_START     0x9800
#define GABLE_GB_SCRN1_START     0x9C00
#define GABLE_GB_SCRN_END        0x9FFF
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

// Shortform Macros - General Memory Region Constants //////////////////////////////////////////////

#define G_VRAM                  GABLE_GB_VRAM_START
#define G_SCRN0                 GABLE_GB_SCRN0_START
#define G_SCRN1                 GABLE_GB_SCRN1_START
#define G_SRAM                  GABLE_GB_SRAM_START
#define G_RAM                   GABLE_GB_WRAM_START
#define G_RAMBANK               GABLE_GB_WRAMX_START
#define G_OAMRAM                GABLE_GB_OAM_START
#define G_NETRAM                GABLE_NETRAM_START
#define G_IO                    GABLE_GB_IO_START
#define G_AUD3WAVERAM           GABLE_GB_WAVE_START
#define G_HRAM                  GABLE_GB_HRAM_START

// Shortform Macros - Memory-Mapped Registers //////////////////////////////////////////////////////

/*
    The following macros are used to provide shortform names for the memory-mapped registers of the
    GABLE Engine's components, and their settings. This is done to make the code more readable and
    maintainable, and to provide a consistent naming convention for the registers and their settings.

    The code below is based on "hardware.inc", which is a file commonly used in the Game Boy homebrew
    development community in the development of games on the Game Boy. "hardware.inc" is released
    under the Creative Commons CC0-1.0 License, and is available at the following URL:

        `https://gbdev.io/gbdev/hardware.inc`
        `https://creativecommons.org/publicdomain/zero/1.0/`
*/

#define G_P1                (GABLE_HP_JOYP & 0xFF)
#define G_NTC               (GABLE_HP_NTC & 0xFF)
#define G_NTS               (GABLE_HP_NTS & 0xFF)
#define G_DIV               (GABLE_HP_DIV & 0xFF)
#define G_TIMA              (GABLE_HP_TIMA & 0xFF)
#define G_TMA               (GABLE_HP_TMA & 0xFF)
#define G_TAC               (GABLE_HP_TAC & 0xFF)
#define G_RTCS              (GABLE_HP_RTCS & 0xFF)
#define G_RTCM              (GABLE_HP_RTCM & 0xFF)
#define G_RTCH              (GABLE_HP_RTCH & 0xFF)
#define G_RTCDH             (GABLE_HP_RTCDH & 0xFF)
#define G_RTCDL             (GABLE_HP_RTCDL & 0xFF)
#define G_RTCL              (GABLE_HP_RTCL & 0xFF)
#define G_IF                (GABLE_HP_IF & 0xFF)
#define G_NR10              (GABLE_HP_NR10 & 0xFF)
#define G_NR11              (GABLE_HP_NR11 & 0xFF)
#define G_NR12              (GABLE_HP_NR12 & 0xFF)
#define G_NR13              (GABLE_HP_NR13 & 0xFF)
#define G_NR14              (GABLE_HP_NR14 & 0xFF)
#define G_NR21              (GABLE_HP_NR21 & 0xFF)
#define G_NR22              (GABLE_HP_NR22 & 0xFF)
#define G_NR23              (GABLE_HP_NR23 & 0xFF)
#define G_NR24              (GABLE_HP_NR24 & 0xFF)
#define G_NR30              (GABLE_HP_NR30 & 0xFF)
#define G_NR31              (GABLE_HP_NR31 & 0xFF)
#define G_NR32              (GABLE_HP_NR32 & 0xFF)
#define G_NR33              (GABLE_HP_NR33 & 0xFF)
#define G_NR34              (GABLE_HP_NR34 & 0xFF)
#define G_NR41              (GABLE_HP_NR41 & 0xFF)
#define G_NR42              (GABLE_HP_NR42 & 0xFF)
#define G_NR43              (GABLE_HP_NR43 & 0xFF)
#define G_NR44              (GABLE_HP_NR44 & 0xFF)
#define G_NR50              (GABLE_HP_NR50 & 0xFF)
#define G_NR51              (GABLE_HP_NR51 & 0xFF)
#define G_NR52              (GABLE_HP_NR52 & 0xFF)
#define G_LCDC              (GABLE_HP_LCDC & 0xFF)
#define G_STAT              (GABLE_HP_STAT & 0xFF)
#define G_SCY               (GABLE_HP_SCY & 0xFF)
#define G_SCX               (GABLE_HP_SCX & 0xFF)
#define G_LY                (GABLE_HP_LY & 0xFF)
#define G_LYC               (GABLE_HP_LYC & 0xFF)
#define G_DMA               (GABLE_HP_DMA & 0xFF)
#define G_BGP               (GABLE_HP_BGP & 0xFF)
#define G_OBP0              (GABLE_HP_OBP0 & 0xFF)
#define G_OBP1              (GABLE_HP_OBP1 & 0xFF)
#define G_WY                (GABLE_HP_WY & 0xFF)
#define G_WX                (GABLE_HP_WX & 0xFF)
#define G_KEY1              (GABLE_HP_KEY1 & 0xFF)
#define G_VBK               (GABLE_HP_VBK & 0xFF)
#define G_HDMA1             (GABLE_HP_HDMA1 & 0xFF) 
#define G_HDMA2             (GABLE_HP_HDMA2 & 0xFF)
#define G_HDMA3             (GABLE_HP_HDMA3 & 0xFF) 
#define G_HDMA4             (GABLE_HP_HDMA4 & 0xFF)
#define G_HDMA5             (GABLE_HP_HDMA5 & 0xFF)
#define G_RP                (GABLE_HP_RP & 0xFF)
#define G_BGPI              (GABLE_HP_BGPI & 0xFF)
#define G_BGPD              (GABLE_HP_BGPD & 0xFF)
#define G_OBPI              (GABLE_HP_OBPI & 0xFF)
#define G_OBPD              (GABLE_HP_OBPD & 0xFF)
#define G_OPRI              (GABLE_HP_OPRI & 0xFF)
#define G_GRPM              (GABLE_HP_GRPM & 0xFF)
#define G_SVBK              (GABLE_HP_SVBK & 0xFF)
#define G_SSBK              (GABLE_HP_SSBK & 0xFF)
#define G_DSBKH             (GABLE_HP_DSBKH & 0xFF)
#define G_DSBKL             (GABLE_HP_DSBKL & 0xFF)
#define G_PCM12             (GABLE_HP_PCM12 & 0xFF)
#define G_PCM34             (GABLE_HP_PCM34 & 0xFF)
#define G_IE                (GABLE_HP_IE & 0xFF)

#define G_AUD1SWEEP         G_NR10
#define G_AUD1LEN           G_NR11
#define G_AUD1ENV           G_NR12
#define G_AUD1LOW           G_NR13
#define G_AUD1HIGH          G_NR14
#define G_AUD2LEN           G_NR21
#define G_AUD2ENV           G_NR22
#define G_AUD2LOW           G_NR23
#define G_AUD2HIGH          G_NR24
#define G_AUD3ENA           G_NR30
#define G_AUD3LEN           G_NR31
#define G_AUD3LEVEL         G_NR32
#define G_AUD3LOW           G_NR33
#define G_AUD3HIGH          G_NR34
#define G_AUD4LEN           G_NR41
#define G_AUD4ENV           G_NR42
#define G_AUD4POLY          G_NR43
#define G_AUD4FREQ          G_NR44
#define G_AUDVOL            G_NR50
#define G_AUDTERM           G_NR51
#define G_AUDENA            G_NR52

// Shortform Macros - Memory-Mapped Register Bitfields /////////////////////////////////////////////

#define G_P1F_5             0b00100000
#define G_P1F_4             0b00010000
#define G_P1F_3             0b00001000
#define G_P1F_2             0b00000100
#define G_P1F_1             0b00000010
#define G_P1F_0             0b00000001
#define G_P1F_GET_DPAD      G_P1F_5
#define G_P1F_GET_BTN       G_P1F_4
#define G_P1F_GET_NONE      (G_P1F_5 | G_P1F_4)

#define G_NTCF_BANK         0b00001111
#define G_NTCF_DIR          0b00010000
#define G_NTCF_SEND         0b00010000
#define G_NTCF_RECV         0b00000000
#define G_NTCF_STAT         0b01100000
#define G_NTCF_READY        0b00000000
#define G_NTCF_BUSY         0b00100000
#define G_NTCF_TIMEOUT      0b01000000
#define G_NTCF_ERROR        0b01100000
#define G_NTCF_START        0b10000000
#define G_NTCF_ON           0b10000000
#define G_NTCF_OFF          0b00000000
#define G_NTCB_DIR          4
#define G_NTCB_START        7

#define G_TACF_STOP         0b00000000
#define G_TACF_START        0b00000100
#define G_TACF_4KHZ         0b00000000
#define G_TACF_262KHZ       0b00000001
#define G_TACF_65KHZ        0b00000010
#define G_TACF_16KHZ        0b00000011
#define G_TACB_START        2

#define G_AUD1SWEEP_UP      0b00000000
#define G_AUD1SWEEP_DOWN    0b00001000
#define G_AUD3ENA_OFF       0b00000000
#define G_AUD3ENA_ON        0b10000000
#define G_AUD3LEVEL_MUTE    0b00000000
#define G_AUD3LEVEL_100     0b00100000
#define G_AUD3LEVEL_50      0b01000000
#define G_AUD3LEVEL_25      0b01100000
#define G_AUD4POLY_15STEP   0b00000000
#define G_AUD4POLY_7STEP    0b00010000
#define G_AUDVOL_VIN_LEFT   0b10000000
#define G_AUDVOL_VIN_RIGHT  0b00001000
#define G_AUDTERM_4_LEFT    0b10000000
#define G_AUDTERM_3_LEFT    0b01000000
#define G_AUDTERM_2_LEFT    0b00100000
#define G_AUDTERM_1_LEFT    0b00010000
#define G_AUDTERM_4_RIGHT   0b00001000
#define G_AUDTERM_3_RIGHT   0b00000100
#define G_AUDTERM_2_RIGHT   0b00000010
#define G_AUDTERM_1_RIGHT   0b00000001
#define G_AUDENA_OFF        0b00000000
#define G_AUDENA_ON         0b10000000

#define G_LCDCF_OFF         0b00000000
#define G_LCDCF_ON          0b10000000
#define G_LCDCF_WIN9800     0b00000000
#define G_LCDCF_WIN9C00     0b01000000
#define G_LCDCF_WINOFF      0b00000000
#define G_LCDCF_WINON       0b00100000
#define G_LCDCF_BLK21       0b00000000
#define G_LCDCF_BLK01       0b00010000
#define G_LCDCF_BG9800      0b00000000
#define G_LCDCF_BG9C00      0b00001000
#define G_LCDCF_OBJ8        0b00000000
#define G_LCDCF_OBJ16       0b00000100
#define G_LCDCF_OBJOFF      0b00000000
#define G_LCDCF_OBJON       0b00000010
#define G_LCDCF_BGOFF       0b00000000
#define G_LCDCF_BGON        0b00000001
#define G_LCDCB_ON          7
#define G_LCDCB_WIN9C00     6
#define G_LCDCB_WINON       5
#define G_LCDCB_BLKS        4
#define G_LCDCB_BG9C00      3
#define G_LCDCB_OBJ16       2
#define G_LCDCB_OBJON       1
#define G_LCDCB_BGON        0

#define G_STATF_LYC           0b01000000
#define G_STATF_MODE10        0b00100000
#define G_STATF_MODE01        0b00010000
#define G_STATF_MODE00        0b00001000
#define G_STATF_LYCF          0b00000100
#define G_STATF_HBL           0b00000000
#define G_STATF_VBL           0b00000001
#define G_STATF_OAM           0b00000010
#define G_STATF_LCD           0b00000011
#define G_STATF_BUSY          0b00000010
#define G_STATB_LYC           6
#define G_STATB_MODE10        5
#define G_STATB_MODE01        4
#define G_STATB_MODE00        3
#define G_STATB_LYCF          2
#define G_STATB_BUSY          1

#define G_HDMA5F_MODE_GP     0b00000000
#define G_HDMA5F_MODE_HBL    0b10000000
#define G_HDMA5F_BUSY        0b10000000
#define G_HDMA5B_MODE        7

#define G_BGPIF_AUTOINC      0b10000000
#define G_BGPIB_AUTOINC      7
#define G_BCPSF_AUTOINC      0b10000000
#define G_BCPSB_AUTOINC      7
#define G_OBPIF_AUTOINC      0b10000000
#define G_OBPIB_AUTOINC      7
#define G_OCPSF_AUTOINC      0b10000000
#define G_OCPSB_AUTOINC      7

#define G_GRPMF_CGB          0b00000001
#define G_GRPMF_DMG          0b00000000
#define G_GRPMB_MODE         0

#define G_IEF_RTC            0b00100000
#define G_IEF_JOYPAD         0b00010000
#define G_IEF_NET            0b00001000
#define G_IEF_TIMER          0b00000100
#define G_IEF_STAT           0b00000010
#define G_IEF_VBLANK         0b00000001

// Shortform Macros - Common Sound Channel Flags ///////////////////////////////////////////////////

#define G_AUDLEN_DUTY_12_5          0b00000000
#define G_AUDLEN_DUTY_25            0b01000000
#define G_AUDLEN_DUTY_50            0b10000000
#define G_AUDLEN_DUTY_75            0b11000000
#define G_AUDENV_UP                 0b00001000
#define G_AUDENV_DOWN               0b00000000
#define G_AUDHIGH_RESTART           0b10000000
#define G_AUDHIGH_LENGTH_ON         0b01000000
#define G_AUDHIGH_LENGTH_OFF        0b00000000

// Shortform Macros - Joypad Buttons ///////////////////////////////////////////////////////////////

#define G_PADF_DOWN                 0x80
#define G_PADF_UP                   0x40
#define G_PADF_LEFT                 0x20
#define G_PADF_RIGHT                0x10
#define G_PADF_START                0x08
#define G_PADF_SELECT               0x04
#define G_PADF_B                    0x02
#define G_PADF_A                    0x01

#define G_PADB_DOWN                 7
#define G_PADB_UP                   6
#define G_PADB_LEFT                 5
#define G_PADB_RIGHT                4
#define G_PADB_START                3
#define G_PADB_SELECT               2
#define G_PADB_B                    1
#define G_PADB_A                    0

// Shortform Macros - Screen Macros ////////////////////////////////////////////////////////////////

#define G_SCRN_X                    160
#define G_SCRN_Y                    144
#define G_SCRN_X_B                  20
#define G_SCRN_Y_B                  18
#define G_SCRN_VX                   256
#define G_SCRN_VY                   256
#define G_SCRN_VX_B                 32
#define G_SCRN_VY_B                 32

// Shortform Macros - Object Attribute Memory (OAM) ////////////////////////////////////////////////

#define G_OAMA_Y                    0
#define G_OAMA_X                    1
#define G_OAMA_TILE                 2
#define G_OAMA_ATTR                 3
#define G_OAMA_SIZE                 4
#define G_OAM_Y_OFS                 16
#define G_OAM_X_OFS                 8
#define G_OAM_COUNT                 40
#define G_OAMF_PRI                  0x80
#define G_OAMF_YFLIP                0x40
#define G_OAMF_XFLIP                0x20
#define G_OAMF_PAL0                 0x00
#define G_OAMF_PAL1                 0x10
#define G_OAMF_BANK0                0x00
#define G_OAMF_BANK1                0x08
#define G_OAMF_PALMASK              0b00000111
#define G_OAMB_PRI                  7
#define G_OAMB_YFLIP                6
#define G_OAMB_XFLIP                5
#define G_OAMB_PAL                  4
#define G_OAMB_BANK                 3

// Shortform Macros - "CPU" Registers, Flags, and Execution Conditions /////////////////////////////

#define G_A                     GABLE_RT_A
#define G_F                     GABLE_RT_F
#define G_B                     GABLE_RT_B
#define G_C                     GABLE_RT_C
#define G_D                     GABLE_RT_D
#define G_E                     GABLE_RT_E
#define G_H                     GABLE_RT_H
#define G_L                     GABLE_RT_L
#define G_AF                    GABLE_RT_AF
#define G_BC                    GABLE_RT_BC
#define G_DE                    GABLE_RT_DE
#define G_HL                    GABLE_RT_HL
#define G_SP                    GABLE_RT_SP
#define G_FLAG_Z                GABLE_FT_Z
#define G_FLAG_N                GABLE_FT_N
#define G_FLAG_H                GABLE_FT_H
#define G_FLAG_C                GABLE_FT_C
#define G_NOCOND                GABLE_CT_NONE
#define G_COND_NZ               GABLE_CT_NZ
#define G_COND_Z                GABLE_CT_Z
#define G_COND_NC               GABLE_CT_NC
#define G_COND_C                GABLE_CT_C

// Shortform Macros - Internal Macros //////////////////////////////////////////////////////////////

#define __G_MERGE2__(A, B) A##B
#define __G_MERGE3__(A, B, C) A##B##C
#define __G_NEXTU_LABEL__(A, B) __G_MERGE3__(A, __NEXTU, B)
#define __G_RSSET_LABEL__(A) __G_MERGE2__(__RSSET, A)

// Shortform Macros - RAM Enumeration Macros ///////////////////////////////////////////////////////

#define G_STRUCT(K) K##_OFFSET, K##_STARTS = (K##_OFFSET - 1),
#define G_DB(K) K##_OFFSET,
#define G_DW(K) K##_OFFSET, K##_END = K##_OFFSET + 1,
#define G_DL(K) K##_OFFSET, K##_END = K##_OFFSET + 3,
#define G_DS(K, C) K##_OFFSET, K##_END = K##_OFFSET + (C - 1),
#define G_UNION(K) K##__UNION, K##__STARTU = (K##__UNION - 1),
#define G_NEXTU(K) __G_NEXTU_LABEL__(K, __LINE__) = (K##__UNION - 1),
#define G_ENDU(K, M) K##__ENDU = (K##__STARTU + M),

#define G_WADDR(K) (GABLE_GB_WRAM0_START + K##_OFFSET)
#define G_W0ADDR(K) (GABLE_GB_WRAM0_START + K##_OFFSET)
#define G_WXADDR(K) (GABLE_GB_WRAMX_START + K##_OFFSET)
#define G_VADDR(K) (GABLE_GB_VRAM_START + K##_OFFSET)
#define G_SADDR(K) (GABLE_GB_SRAM_START + K##_OFFSET)
#define G_HADDR(K) (GABLE_GB_HRAM_START + K##_OFFSET)

// Shortform Macros - Structure Offset Macros //////////////////////////////////////////////////////

#define G_RSSET(C)              __G_RSSET_LABEL__(__LINE__) = (C - 1),
#define G_RSRESET               G_RSSET(0)
#define G_RBS(K, C)             K, K##__END = K + C,
#define G_RB(K)                 G_RBS(K, 1)
#define G_RWS(K, C)             K, K##__END = K + (C * 2),
#define G_RW(K)                 G_RWS(K, 1)
#define G_RLS(K, C)             K, K##__END = K + (C * 4),
#define G_RL(K)                 G_RLS(K, 1)
