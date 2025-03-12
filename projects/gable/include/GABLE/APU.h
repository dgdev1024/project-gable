/**
 * @file     GABLE/APU.h
 * @brief    Contains the GABLE Engine's APU context structure and public functions.
 * 
 * The GABLE Engine's APU context structure seeks to simulate the Game Boy's audio-processing unit
 * (APU) hardware. The APU is responsible for generating sound effects and music in games built on
 * the engine, output via four audio channels: two square-wave pulse channels (we'll call them
 * `PC1` and `PC2`), a programmable, sampled wave channel (`WC`), and a noise channel (`NC`).
 * 
 * The APU provides the following hardware registers:
 * 
 * - `NR52` (Audio Master Control): The APU's master control register is used to enable or disable
 *   the APU hardware, as well as to check the enable status of the four audio channels. This
 *   register's bits are defined as follows:
 *     - Bit 0 - `PC1` Enable: Set if `PC1` is enabled; clear otherwise. Read-only.
 *     - Bit 1 - `PC2` Enable: Set if `PC2` is enabled; clear otherwise. Read-only.
 *     - Bit 2 - `WC` Enable: Set if `WC` is enabled; clear otherwise. Read-only.
 *     - Bit 3 - `NC` Enable: Set if `NC` is enabled; clear otherwise. Read-only.
 *     - Bit 7 - APU Enable: Set to enable the APU; clear to disable the APU. Clearing this bit
 *       also resets all of the APU's hardware registers and makes them read-only, except for this
 *       register.
 * 
 * - `NR51` (Sound Panning): The APU's sound panning register is used to control the stereo panning
 *   of the four audio channels. This register's bits are defined as follows:
 *     - Bit 0 - `PC1` Right: Set to output `PC1` to the right speaker; clear otherwise.
 *     - Bit 1 - `PC2` Right: Set to output `PC2` to the right speaker; clear otherwise.
 *     - Bit 2 - `WC` Right: Set to output `WC` to the right speaker; clear otherwise.
 *     - Bit 3 - `NC` Right: Set to output `NC` to the right speaker; clear otherwise.
 *     - Bit 4 - `PC1` Left: Set to output `PC1` to the left speaker; clear otherwise.
 *     - Bit 5 - `PC2` Left: Set to output `PC2` to the left speaker; clear otherwise.
 *     - Bit 6 - `WC` Left: Set to output `WC` to the left speaker; clear otherwise.
 *     - Bit 7 - `NC` Left: Set to output `NC` to the left speaker; clear otherwise.
 * 
 * - `NR50` (Master Volume Control and VIN Panning): The APU's master volume control and VIN panning
 *   register is used to control the overall volume of the APU's audio output, as well as the panning
 *   of the `VIN` channel, an external audio channel which was never used by any Game Boy games.
 *   The bits of this register control the following settings:
 *      - Bits 0-2 - Right Speaker Volume: The volume level of the right speaker, from 0 (mute) to
 *        7 (max volume).
 *      - Bit 3: `VIN` Right: Set to output the `VIN` channel to the right speaker; clear otherwise.
 *      - Bits 4-6 - Left Speaker Volume: The volume level of the left speaker, from 0 (mute) to 7
 *        (max volume).
 *      - Bit 7: `VIN` Left: Set to output the `VIN` channel to the left speaker; clear otherwise.
 * 
 * - `NR10` (`PC1` Frequency Sweep): `PC1` comes with a frequency sweep unit that can change the
 *   frequency of the square wave at a specified rate. This register controls the sweep unit's
 *   settings. The bits of this register control the following settings:
 *      - Bits 0-2 - Individual Step: Controls how much the channel's frequency changes with each
 *        iteration of the frequency sweep.
 *      - Bit 3 - Direction: Controls the direction of the frequency sweep (Set = decrease;
 *        Clear = increase).
 *      - Bits 4-6 - Sweep Pace: Controls the duration of each sweep step, in units of 128 Hz ticks.
 * 
 * - `NR11` (`PC1` Length Timer & Duty Cycle): This register controls `PC1`s initial length timer
 *   value, as well as its wave pattern duty cycle. This register's bits control the following
 *   settings:
 *      - Bits 0-5 - Initial Length Timer: The value to set the length timer to when the channel is
 *        triggered. Write-only.
 *      - Bits 6-7 - Wave Pattern Duty Cycle: The duty cycle of the square wave, which controls the
 *        ratio of high to low signal levels, affecting how the square wave sounds. The possible
 *        enumerated duty cycles are as follows:
 *          - `0b00` - 12.5% duty cycle: 1 part high, 7 parts low. Sounds like a thin pulse wave.
 *          - `0b01` - 25% duty cycle: 2 parts high, 6 parts low. Sounds like a medium pulse wave.
 *          - `0b10` - 50% duty cycle: 4 parts high, 4 parts low. Sounds like a square wave.
 *          - `0b11` - 75% duty cycle: 6 parts high, 2 parts low. Sounds like a wide pulse wave, and
 *            sounds just like the 25% duty cycle wave.
 * 
 * - `NR12` (`PC1` Volume & Envelope): This register controls the initial volume of `PC1`, as well
 *   as the channel's envelope sweep settings. The bits of this register control the following
 *   settings:
 *      - Bits 0-2 - Sweep Pace: Controls how much the channel's volume changes with each iteration
 *        of the envelope sweep, in units of 64 Hz ticks. A value of 0 stops the envelope sweep.
 *      - Bit 3 - Direction: Controls the direction of the envelope sweep (Set = increase; Clear =
 *        decrease).
 *      - Bits 4-7 - Initial Volume: The volume level to set the channel to when the channel is
 *        triggered, from 0 (mute) to 15 (max volume).
 *      - Setting Bits 3-7 to 0 disables the channel's digital-to-analog converter (DAC), and
 *        thereby disables the channel.
 * 
 * - `NR13` (`PC1` Period Low Byte): This register contains the lower 8 bits of `PC1`'s 11-bit
 *   frequency period value. Write-only.
 * 
 * - `NR14` (`PC1` Period High Bits & Control): This register contains the upper 3 bits of `PC1`'s
 *   11-bit frequency period value, as well as the channel's length timer enable and trigger bits.
 *   The bits of this register control the following settings:
 *      - Bits 0-2 - Period High Bits: The upper 3 bits of the 11-bit frequency period value. Write-only.
 *      - Bit 6 - Length Timer Enable: Set to enable the channel's length timer; clear otherwise.
 *      - Bit 7 - Trigger: Write to this bit to trigger the channel's sound generation. Write-only.
 * 
 * - `NR21`, `NR22`, `NR23`, and `NR24` work exactly the same as `NR11`, `NR12`, `NR13`, and `NR14`,
 *   but for `PC2` instead of `PC1`. Note that `PC2` does not have a frequency sweep unit.
 * 
 * - `NR30` (`WC` DAC Enable): This register controls the digital-to-analog converter (DAC) of the
 *   wave channel. The bits of this register control the following settings:
 *      - Bit 7 - DAC Power: Set to enable the channel's DAC; clear otherwise. Clearing this bit
 *        disables the channel's DAC and thereby disables the channel. Although not required, it is
 *        recommended to turn off this channel's DAC before writing to the APU's wave sample buffer.
 * 
 * - `NR31` (`WC` Length Timer): This register controls the initial length timer value of the wave
 *   channel. The bits of this register control the following settings:
 *      - Bits 0-7 - Initial Length Timer: The value to set the length timer to when the channel is
 *        triggered. Write-only.
 * 
 * - `NR32` (`WC` Output Level): The wave channel does not have a volume envelope like the pulse
 *   channels. Instead, it has a coarser, fixed volume control. This register controls that volume
 *   level. The bits of this register control the following settings:
 *      - Bits 5-6 - Output Level: The volume level of the channel, from 0 (mute) to 3 (max volume).
 *        The possible enumerated volume levels are as follows:
 *          - `0b00` - Mute: The channel is silent and outputs no sound.
 *          - `0b01` - 100% Volume: The channel outputs at full volume.
 *          - `0b10` - 50% Volume: The channel outputs at half volume.
 *          - `0b11` - 25% Volume: The channel outputs at quarter volume.
 * 
 * - `NR33` and `NR34` work exactly the same as `NR13` and `NR14`, but for `WC` instead of `PC1`.
 * 
 * - `NR41` (`NC` Length Timer): This register controls the initial length timer value of the noise
 *   channel. The bits of this register control the following settings:
 *      - Bits 0-5 - Initial Length Timer: The value to set the length timer to when the channel is
 *        triggered. Write-only.
 * 
 * - `NR42` (`NC` Volume & Envelope): This register controls the initial volume of the noise channel,
 *   as well as the channel's envelope sweep settings. This register works exactly the same as `NR12`,
 *   but for the noise channel instead of `PC1`.
 * 
 * - `NR43` (`NC` Frequency & Randomness): The noise channel uses a linear-feedback shift register
 *   (LFSR) to generate pseudo-random noise. This register controls the frequency of the noise channel
 *   and the feedback pattern of the LFSR. The bits of this register control the following settings:
 *      - Bits 0-2 - Clock Divider: Used along with the Clock Shift setting (see below) to determine
 *        the frequency of the noise channel.
 *      - Bit 3 - LFSR Width: Controls the width of the LFSR's feedback pattern. Set to 0 for a 15-bit
 *        pattern, or 1 for a 7-bit pattern.
 *      - Bits 4-7 - Clock Shift: Used along with the Clock Divider setting (see above) to determine
 *        the frequency of the noise channel.
 *      - The following formula is used to calculate the noise channel's frequency:
 *          - `F = 262144 / (D * (2 ^ S))`
 *          - `F` is the resultant frequency.
 *          - `D` is the Clock Divider setting. If `D` = 0, then `D` = 0.5, instead.
 *          - `S` is the Clock Shift setting.
 * 
 * - `NR44` (`NC` Control): This register controls the noise channel's length timer enable and trigger
 *   bits. The bits of this register control the following settings:
 *      - Bit 6 - Length Timer Enable: Set to enable the channel's length timer; clear otherwise.
 *      - Bit 7 - Trigger: Write to this bit to trigger the channel's sound generation. Write-only.
 * 
 * The APU component provides the following memory buffer:
 * 
 * - The Wave Pattern RAM (`WAVE`): The wave pattern RAM is a 16-byte buffer which stores 32 4-bit
 *   samples that make up the wave channel's waveform. The wave channel can play back these samples
 *   at a specified frequency to generate a custom waveform. The wave channel's output level setting
 *   (`NR32`) controls the volume level of the wave channel. The wave pattern RAM is mapped to the
 *   `0xFF30` to `0xFF3F` memory region.
 * 
 * The APU component is clocked at the same rate as the engine's timer and PPU components - at a
 * frequency of 4.194304 MHz.
 * - The pulse channels are clocked at a frequency of 1.048576 MHz - every 4 clock cycles.
 * - The wave channel is clocked at a frequency of 2.084576 MHz - every 2 clock cycles.
 * - The noise channel is clocked at a frequency dictated by the clock divider and clock shift settings
 *   in the `NR43` register (see the formula above).
 * 
 * When the pulse and wave channels are clocked, an internal period divider register is incremented,
 * resetting to the period value in the `NR*3` and `NR*4` registers when it overflows. The higher
 * the initial period value in these registers, the lower the period, the higher the resultant
 * frequency, and the shorter the wavelength of the sound wave.
 * - The resultant sample rate and size of the pulse channels' square waves are determined by the
 *   following formulae:
 *      - `R = 1048756 / (2048 - PV)`
 *      - `S = 131072 / (2048 - PV)`
 *      - `R` is the resultant sample rate.
 *      - `S` is the resultant sample size.
 *      - `PV` is the channels' current period values.
 * - The wave channel's sample rate is determined by the following formulae:
 *      - `R = 2097152 / (2048 - PV)`
 *      - `S = 65536 / (2048 - PV)`
 *      - `R`, `S`, and `PV` are the same as above.
 * 
 * When the noise channel is clocked, its LFSR is processed as follows:
 * - Bit 15 is set if the values of bits 0 and 1 are equal; otherwise, it is cleared.
 * - If `NR43`'s LFSR width is set, then bit 7 is set to the same value.
 * - The LFSR is shifted right by one bit.
 * - The new value of bit 0 determines the output of the noise channel.
 * 
 * The APU component is equipped with its own internal divider register, which is incremented every
 * time the 4th bit (or 5th bit in double-speed mode) of the engine timer's `DIV` register changes from
 * high to low. This happens at a frequency of 512 Hz, and we'll call this unit of time an "APU-DIV tick".
 * The following events occur at the following intervals:
 * - The channels' envelope sweep units are clocked every 8 APU-DIV ticks, at a frequency of 64 Hz.
 * - The channels' length timers are clocked every 2 APU-DIV ticks, at a frequency of 256 Hz.
 * - `PC1`'s frequency sweep unit is clocked every 4 APU-DIV ticks, at a frequency of 128 Hz.
 */

#pragma once
#include <GABLE/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief The size of the GABLE Engine's wave pattern RAM, in bytes.
 */
#define GABLE_WAVE_RAM_SIZE 16

/**
 * @brief The size of the GABLE Engine's wave pattern RAM, in nibbles.
 */
#define GABLE_WAVE_RAM_NIBBLES 32

/**
 * @brief The sample rate of the audio output by the GABLE Engine's APU, in Hz.
 */
#define GABLE_AUDIO_SAMPLE_RATE 44100

/**
 * @brief The maximum octave of the GABLE Engine's audio channels, from 0 to 7.
 */
#define GABLE_MAX_OCTAVE 7

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward declaration of the GABLE Engine structure.
 */
typedef struct GABLE_Engine GABLE_Engine;

/**
 * @brief A forward declaration of the GABLE APU audio sample structure.
 */
typedef struct GABLE_AudioSample GABLE_AudioSample;

/**
 * @brief The GABLE Engine's APU structure.
 */
typedef struct GABLE_APU GABLE_APU;

/**
 * @brief The GABLE Engine's audio mix callback function.
 */
typedef void (*GABLE_AudioMixCallback) (GABLE_Engine*, const GABLE_AudioSample*);

// Enumerations ////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Enumerates the notes in the musical scale.
 */
typedef enum GABLE_AudioNote
{
    GABLE_NOTE_C = 0,           ///< @brief The C note.
    GABLE_NOTE_C_SHARP,         ///< @brief The C# note.
    GABLE_NOTE_D,               ///< @brief The D note.
    GABLE_NOTE_D_SHARP,         ///< @brief The D# note.
    GABLE_NOTE_E,               ///< @brief The E note.
    GABLE_NOTE_F,               ///< @brief The F note.
    GABLE_NOTE_F_SHARP,         ///< @brief The F# note.
    GABLE_NOTE_G,               ///< @brief The G note.
    GABLE_NOTE_G_SHARP,         ///< @brief The G# note.
    GABLE_NOTE_A,               ///< @brief The A note.
    GABLE_NOTE_A_SHARP,         ///< @brief The A# note.
    GABLE_NOTE_B,               ///< @brief The B note.

    GABLE_NOTE_COUNT,           ///< @brief The number of notes in the scale.
    GABLE_NOTE_REST = 0xFF      ///< @brief A rest note.
} GABLE_AudioNote;

/**
 * @brief Enumerates the APU's four audio channels.
 */
typedef enum GABLE_AudioChannel
{
    GABLE_AC_PC1 = 0,     ///< @brief The `PC1` audio channel.
    GABLE_AC_PC2,         ///< @brief The `PC2` audio channel.
    GABLE_AC_WC,          ///< @brief The `WC` audio channel.
    GABLE_AC_NC,          ///< @brief The `NC` audio channel.

    GABLE_AC_PULSE_1 = GABLE_AC_PC1,     ///< @brief The `PC1` audio channel.
    GABLE_AC_PULSE_2 = GABLE_AC_PC2,     ///< @brief The `PC2` audio channel.
    GABLE_AC_WAVE = GABLE_AC_WC,         ///< @brief The `WC` audio channel.
    GABLE_AC_NOISE = GABLE_AC_NC         ///< @brief The `NC` audio channel.
} GABLE_AudioChannel;

/**
 * @brief Enumerates the sweep directions of `PC1`'s frequency sweep unit.
 */
typedef enum GABLE_FrequencySweepDirection
{
    GABLE_FSD_INCREASE = 0,     ///< @brief Increase the frequency.
    GABLE_FSD_DECREASE          ///< @brief Decrease the frequency.
} GABLE_FrequencySweepDirection;

/**
 * @brief Enumerates the sweep direction of the pulse and noise channels' volume envelope units.
 */
typedef enum GABLE_EnvelopeSweepDirection
{
    GABLE_ESD_DECREASE = 0,     ///< @brief Decrease the volume.
    GABLE_ESD_INCREASE          ///< @brief Increase the volume.
} GABLE_EnvelopeSweepDirection;

/**
 * @brief Enumerates the possible duty cycles of the GABLE Engine's pulse channels.
 */
typedef enum GABLE_PulseDutyCycle
{
    GABLE_PDC_12_5 = 0,     ///< @brief 12.5% duty cycle: 1 part high, 7 parts low.
    GABLE_PDC_25,           ///< @brief 25% duty cycle: 2 parts high, 6 parts low.
    GABLE_PDC_50,           ///< @brief 50% duty cycle: 4 parts high, 4 parts low.
    GABLE_PDC_75            ///< @brief 75% duty cycle: 6 parts high, 2 parts low.
} GABLE_PulseDutyCycle;

/**
 * @brief Enumerates the possible output levels of the GABLE Engine's wave channel.
 */
typedef enum GABLE_WaveOutputLevel
{
    GABLE_WOL_MUTE = 0,     ///< @brief Mute: The channel is silent and outputs no sound.
    GABLE_WOL_100,          ///< @brief 100% Volume: The channel outputs at full volume.
    GABLE_WOL_50,           ///< @brief 50% Volume: The channel outputs at half volume.
    GABLE_WOL_25,           ///< @brief 25% Volume: The channel outputs at quarter volume.

    GABLE_WOL_FULL = GABLE_WOL_100,     ///< @brief Full volume: The channel outputs at full volume.
    GABLE_WOL_HALF = GABLE_WOL_50,      ///< @brief Half volume: The channel outputs at half volume.
    GABLE_WOL_QUARTER = GABLE_WOL_25    ///< @brief Quarter volume: The channel outputs at quarter volume.
} GABLE_WaveOutputLevel;

// Hardware Register Unions ////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the GABLE Engine's audio master control (`NR52`) register.
 */
typedef union GABLE_AudioMasterControl
{
    struct
    {
        Uint8 m_PC1Enable : 1;                  ///< @brief The `PC1` channel enable bit.
        Uint8 m_PC2Enable : 1;                  ///< @brief The `PC2` channel enable bit.
        Uint8 m_WCEnable : 1;                   ///< @brief The `WC` channel enable bit.
        Uint8 m_NCEnable : 1;                   ///< @brief The `NC` channel enable bit.
        Uint8 : 3;                              ///< @brief Unused bits.
        Uint8 m_Enable : 1;                     ///< @brief The APU enable bit.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_AudioMasterControl;

/**
 * @brief A union representing the GABLE Engine's sound panning (`NR51`) register.
 */
typedef union GABLE_SoundPanning
{
    struct
    {
        Uint8 m_PC1Right : 1;                   ///< @brief The `PC1` right speaker bit.
        Uint8 m_PC2Right : 1;                   ///< @brief The `PC2` right speaker bit.
        Uint8 m_WCRight : 1;                    ///< @brief The `WC` right speaker bit.
        Uint8 m_NCRight : 1;                    ///< @brief The `NC` right speaker bit.
        Uint8 m_PC1Left : 1;                    ///< @brief The `PC1` left speaker bit.
        Uint8 m_PC2Left : 1;                    ///< @brief The `PC2` left speaker bit.
        Uint8 m_WCLeft : 1;                     ///< @brief The `WC` left speaker bit.
        Uint8 m_NCLeft : 1;                     ///< @brief The `NC` left speaker bit.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_SoundPanning;

/**
 * @brief A union representing the GABLE Engine's master volume control and `VIN` panning (`NR50`)
 * register.
 */
typedef union GABLE_MasterVolumeControl
{
    struct
    {
        Uint8 m_RightVolume : 3;                ///< @brief The right speaker volume bits.
        Uint8 m_VINRight : 1;                   ///< @brief The `VIN` right speaker bit.
        Uint8 m_LeftVolume : 3;                 ///< @brief The left speaker volume bits.
        Uint8 m_VINLeft : 1;                    ///< @brief The `VIN` left speaker bit.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_MasterVolumeControl;

/**
 * @brief A union representing the GABLE Engine's `PC1` frequency sweep (`NR10`) register.
 */
typedef union GABLE_PulseFrequencySweep
{
    struct
    {
        Uint8 m_IndividualStep : 3;             ///< @brief The individual step bits.
        Uint8 m_Direction : 1;                  ///< @brief The direction bit.
        Uint8 m_SweepPace : 3;                  ///< @brief The sweep pace bits.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_PulseFrequencySweep;

/**
 * @brief A union representing the GABLE Engine's pulse channels' length timer and duty cycle 
 *        (`NR*1`) registers.
 */
typedef union GABLE_PulseLengthDuty
{
    struct
    {
        Uint8 m_InitialLength : 6;              ///< @brief The initial length timer bits.
        Uint8 m_DutyCycle : 2;                  ///< @brief The duty cycle bits.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_PulseLengthDuty;

/**
 * @brief A union representing the GABLE Engine's pulse and noise channels' volume and envelope
 *        (`NR*2`) registers.
 */
typedef union GABLE_VolumeEnvelope
{
    struct
    {
        Uint8 m_SweepPace : 3;                  ///< @brief The sweep pace bits.
        Uint8 m_Direction : 1;                  ///< @brief The direction bit.
        Uint8 m_InitialVolume : 4;              ///< @brief The initial volume bits.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_VolumeEnvelope;

/**
 * @brief A union representing the GABLE Engine's pulse and wave channels' period low byte (`NR*3`)
 *        registers.
 */
typedef union GABLE_PeriodLowByte
{
    struct
    {
        Uint8 m_PeriodLow;                      ///< @brief The period low byte.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_PeriodLowByte;

/**
 * @brief A union representing the GABLE Engine's audio channels' period high bits and control
 *        (`NR*4`) registers.
 * 
 * @note The `m_PeriodHigh` field is not used by the noise channel, and should not be accessed.
 */
typedef union GABLE_PeriodHighControl
{
    struct
    {
        Uint8 m_PeriodHigh : 3;                 ///< @brief The period high bits.
        Uint8 : 3;                              ///< @brief Unused bits.
        Uint8 m_LengthEnable : 1;               ///< @brief The length timer enable bit.
        Uint8 m_Trigger : 1;                    ///< @brief The trigger bit.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_PeriodHighControl;

/**
 * @brief A union representing the GABLE Engine's wave channel DAC enable (`NR30`) register.
 */
typedef union GABLE_WaveDACEnable
{
    struct
    {
        Uint8 : 7;                              ///< @brief Unused bits.
        Uint8 m_DACPower : 1;                   ///< @brief The DAC power bit.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_WaveDACEnable;

/**
 * @brief A union representing the GABLE Engine's wave channel length timer (`NR31`) register.
 */
typedef union GABLE_WaveLengthTimer
{
    struct
    {
        Uint8 m_InitialLength;                  ///< @brief The initial length timer.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_WaveLengthTimer;

/**
 * @brief A union representing the GABLE Engine's wave channel output level (`NR32`) register.
 */
typedef union GABLE_WaveOutputLevelControl
{
    struct
    {
        Uint8 : 5;                              ///< @brief Unused bits.
        Uint8 m_OutputLevel : 2;                ///< @brief The output level bits.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_WaveOutputLevelControl;

/**
 * @brief A union representing the GABLE Engine's noise channel length timer (`NR41`) register.
 */
typedef union GABLE_NoiseLengthTimer
{
    struct
    {
        Uint8 m_InitialLength : 6;              ///< @brief The initial length timer.
        Uint8 : 2;                              ///< @brief Unused bits.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_NoiseLengthTimer;

/**
 * @brief A union representing the GABLE Engine's noise channel frequency and randomness (`NR43`)
 *        register.
 */
typedef union GABLE_NoiseFrequencyRandomness
{
    struct
    {
        Uint8 m_ClockDivider : 3;               ///< @brief The clock divider bits.
        Uint8 m_LFSRWidth : 1;                  ///< @brief The LFSR width bit.
        Uint8 m_ClockShift : 4;                 ///< @brief The clock shift bits.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_NoiseFrequencyRandomness;

/**
 * @brief A union representing the GABLE Engine's noise channel control (`NR44`) register.
 */
typedef union GABLE_NoiseControl
{
    struct
    {
        Uint8 : 6;                              ///< @brief Unused bits.
        Uint8 m_LengthEnable : 1;               ///< @brief The length timer enable bit.
        Uint8 m_Trigger : 1;                    ///< @brief The trigger bit.
    };

    Uint8 m_Register;                           ///< @brief The raw register value.
} GABLE_NoiseControl;

// Audio Sample Structure //////////////////////////////////////////////////////////////////////////

/**
 * @brief Represents a single audio sample mixed by the GABLE Engine's APU.
 */
typedef struct GABLE_AudioSample
{
    Float32 m_Left;     ///< @brief The left speaker audio sample.
    Float32 m_Right;    ///< @brief The right speaker audio sample.
} GABLE_AudioSample;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief      Creates a new GABLE Engine APU instance.
 * 
 * @return     A pointer to the new GABLE Engine APU instance.
 */
GABLE_APU* GABLE_CreateAPU ();

/**
 * @brief      Destroys a GABLE Engine APU instance.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance to destroy.
 */
void GABLE_DestroyAPU (GABLE_APU* p_APU);

/**
 * @brief      Ticks the GABLE Engine's APU component.
 * 
 * @param      p_APU     A pointer to the GABLE Engine APU instance.
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 */
void GABLE_TickAPU (GABLE_APU* p_APU, GABLE_Engine* p_Engine);

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

/**
 * @brief      Reads a byte from the APU's wave pattern RAM.
 * 
 * @param      p_APU     A pointer to the GABLE Engine APU instance.
 * @param      p_Address The address in the wave pattern RAM to read from.
 * @param      p_Value   A pointer to the variable to store the byte read from the wave pattern RAM.
 * 
 * @return     `true` if the byte was read successfully; `false` otherwise.
 */
Bool GABLE_ReadWaveByte (const GABLE_APU* p_APU, Uint8 p_Address, Uint8* p_Value);

/**
 * @brief      Writes a byte to the APU's wave pattern RAM.
 * 
 * @param      p_APU     A pointer to the GABLE Engine APU instance.
 * @param      p_Address The address in the wave pattern RAM to write to.
 * @param      p_Value   The byte to write to the wave pattern RAM.
 * 
 * @return     `true` if the byte was written successfully; `false` otherwise.
 */
Bool GABLE_WriteWaveByte (GABLE_APU* p_APU, Uint8 p_Address, Uint8 p_Value);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief      Gets the audio master control register (`NR52`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The audio master control register of the APU.
 */
Uint8 GABLE_ReadNR52 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the sound panning register (`NR51`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The sound panning register of the APU.
 */
Uint8 GABLE_ReadNR51 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the master volume control register (`NR50`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The master volume control register of the APU.
 */
Uint8 GABLE_ReadNR50 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the `PC1` frequency sweep register (`NR10`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The `PC1` frequency sweep register of the APU.
 */
Uint8 GABLE_ReadNR10 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the `PC1` length timer and duty cycle register (`NR11`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The `PC1` length timer and duty cycle register of the APU.
 */
Uint8 GABLE_ReadNR11 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the `PC1` volume and envelope register (`NR12`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The `PC1` volume and envelope register of the APU.
 */
Uint8 GABLE_ReadNR12 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the `PC1` period high byte and control register (`NR14`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The `PC1` period high byte and control register of the APU.
 */
Uint8 GABLE_ReadNR14 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the `PC2` length timer and duty cycle register (`NR21`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The `PC2` length timer and duty cycle register of the APU.
 */
Uint8 GABLE_ReadNR21 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the `PC2` volume and envelope register (`NR22`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The `PC2` volume and envelope register of the APU.
 */
Uint8 GABLE_ReadNR22 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the `PC2` period high byte and control register (`NR24`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The `PC2` period high byte and control register of the APU.
 */
Uint8 GABLE_ReadNR24 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the wave DAC enable register (`NR30`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The wave DAC enable register of the APU.
 */
Uint8 GABLE_ReadNR30 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the wave output level register (`NR32`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The wave output level register of the APU.
 */
Uint8 GABLE_ReadNR32 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the wave period high byte and control register (`NR34`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The wave period high byte and control register of the APU.
 */
Uint8 GABLE_ReadNR34 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the noise length timer register (`NR41`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The noise length timer register of the APU.
 */
Uint8 GABLE_ReadNR41 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the noise volume and envelope register (`NR42`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The noise volume and envelope register of the APU.
 */
Uint8 GABLE_ReadNR42 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the noise frequency and randomness register (`NR43`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The noise frequency and randomness register of the APU.
 */
Uint8 GABLE_ReadNR43 (const GABLE_APU* p_APU);

/**
 * @brief      Gets the noise control register (`NR44`) of the GABLE Engine's APU.
 * 
 * @param      p_APU  A pointer to the GABLE Engine APU instance.
 * 
 * @return     The noise control register of the APU.
 */
Uint8 GABLE_ReadNR44 (const GABLE_APU* p_APU);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief      Sets the audio master control register (`NR52`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new audio master control register value.
 */
void GABLE_WriteNR52 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the sound panning register (`NR51`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new sound panning register value.
 */
void GABLE_WriteNR51 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the master volume control register (`NR50`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new master volume control register value.
 */
void GABLE_WriteNR50 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the `PC1` frequency sweep register (`NR10`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new `PC1` frequency sweep register value.
 */
void GABLE_WriteNR10 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the `PC1` length timer and duty cycle register (`NR11`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new `PC1` length timer and duty cycle register value.
 */
void GABLE_WriteNR11 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the `PC1` volume and envelope register (`NR12`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new `PC1` volume and envelope register value.
 */
void GABLE_WriteNR12 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the `PC1` period low byte register (`NR13`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new `PC1` period low byte register value.
 */

void GABLE_WriteNR13 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the `PC1` period high byte and control register (`NR14`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new `PC1` period high byte and control register value.
 */
void GABLE_WriteNR14 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the `PC2` length timer and duty cycle register (`NR21`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new `PC2` length timer and duty cycle register value.
 */
void GABLE_WriteNR21 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the `PC2` volume and envelope register (`NR22`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new `PC2` volume and envelope register value.
 */
void GABLE_WriteNR22 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the `PC2` period low byte register (`NR23`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new `PC2` period low byte register value.
 */
void GABLE_WriteNR23 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the `PC2` period high byte and control register (`NR24`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new `PC2` period high byte and control register value.
 */
void GABLE_WriteNR24 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the wave DAC enable register (`NR30`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new wave DAC enable register value.
 */
void GABLE_WriteNR30 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the wave length timer register (`NR31`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new wave length timer register value.
 */
void GABLE_WriteNR31 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the wave output level register (`NR32`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new wave output level register value.
 */
void GABLE_WriteNR32 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the wave period low byte register (`NR33`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new wave period low byte register value.
 */
void GABLE_WriteNR33 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the wave period high byte and control register (`NR34`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new wave period high byte and control register value.
 */
void GABLE_WriteNR34 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the noise length timer register (`NR41`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new noise length timer register value.
 */
void GABLE_WriteNR41 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the noise volume and envelope register (`NR42`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new noise volume and envelope register value.
 */
void GABLE_WriteNR42 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the noise frequency and randomness register (`NR43`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new noise frequency and randomness register value.
 */
void GABLE_WriteNR43 (GABLE_APU* p_APU, Uint8 p_Register);

/**
 * @brief      Sets the noise control register (`NR44`) of the GABLE Engine's APU.
 * 
 * @param      p_APU      A pointer to the GABLE Engine APU instance.
 * @param      p_Register The new noise control register value.
 */
void GABLE_WriteNR44 (GABLE_APU* p_APU, Uint8 p_Register);

// Public Functions - High-Level Functions /////////////////////////////////////////////////////////

/**
 * @brief      Sets the APU's audio mix callback function.
 * 
 * The audio mix callback function is called by the APU every time it generates a new audio sample.
 * The callback function is passed a pointer to the audio sample generated by the APU.
 * 
 * @param      p_Engine   A pointer to the GABLE Engine instance.
 * @param      p_Callback The audio mix callback function to set.
 */
void GABLE_SetAudioMixCallback (GABLE_Engine* p_Engine, GABLE_AudioMixCallback p_Callback);

/**
 * @brief      Gets the latest audio sample mixed by the GABLE Engine's APU.
 * 
 * @param      p_Engine  A pointer to the GABLE Engine instance.
 * 
 * @return     A pointer to the latest audio sample mixed by the APU.
 */
const GABLE_AudioSample* GABLE_GetLatestAudioSample (GABLE_Engine* p_Engine);

Float32 GABLE_GetFrequencyFromNote (GABLE_AudioNote p_Note, Uint8 p_Octave);

GABLE_PulseFrequencySweep GABLE_GetPC1FrequencySweep (GABLE_Engine* p_Engine);
GABLE_PulseDutyCycle GABLE_GetPulseDutyCycle (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel);
GABLE_VolumeEnvelope GABLE_GetVolumeEnvelope (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel);
Bool GABLE_GetDACEnable (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel);
GABLE_WaveOutputLevel GABLE_GetWaveOutputLevel (GABLE_Engine* p_Engine);
GABLE_NoiseFrequencyRandomness GABLE_GetNoiseLFSRControl (GABLE_Engine* p_Engine);
Bool GABLE_GetLengthTimerEnable (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel);
GABLE_MasterVolumeControl GABLE_GetMasterVolumeControl (GABLE_Engine* p_Engine);
GABLE_SoundPanning GABLE_GetSoundPanning (GABLE_Engine* p_Engine);
GABLE_AudioMasterControl GABLE_GetAudioMasterControl (GABLE_Engine* p_Engine);

void GABLE_SetPC1FrequencySweep (GABLE_Engine* p_Engine, GABLE_PulseFrequencySweep p_Register);
void GABLE_SetPulseLengthDuty (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel, GABLE_PulseLengthDuty p_Register);
void GABLE_SetVolumeEnvelope (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel, GABLE_VolumeEnvelope p_Register);
void GABLE_SetInitialPeriod (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel, Uint16 p_Period);
void GABLE_SetInitialFrequency (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel, Float32 p_Frequency);
void GABLE_SetInitialNote (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel, GABLE_AudioNote p_Note, Uint8 p_Octave);
void GABLE_SetWaveDACEnable (GABLE_Engine* p_Engine, Bool p_Enable);
void GABLE_SetWaveInitialLengthTimer (GABLE_Engine* p_Engine, Uint8 p_Length);
void GABLE_SetWaveOutputLevel (GABLE_Engine* p_Engine, GABLE_WaveOutputLevel p_Level);
void GABLE_SetNoiseInitialLengthTimer (GABLE_Engine* p_Engine, Uint8 p_Length);
void GABLE_SetNoiseLFSRControl (GABLE_Engine* p_Engine, GABLE_NoiseFrequencyRandomness p_Register);
void GABLE_SetLengthTimerEnable (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel, Bool p_Enable);
void GABLE_SetMasterVolumeControl (GABLE_Engine* p_Engine, GABLE_MasterVolumeControl p_Register);
void GABLE_SetSoundPanning (GABLE_Engine* p_Engine, GABLE_SoundPanning p_Register);
void GABLE_SetAudioEnable (GABLE_Engine* p_Engine, Bool p_Enable);
void GABLE_TriggerChannel (GABLE_Engine* p_Engine, GABLE_AudioChannel p_Channel);
