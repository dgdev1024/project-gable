/// @file GABLE/APU.c

#include <GABLE/Engine.h>
#include <GABLE/Timer.h>
#include <GABLE/APU.h>

// Private Constants ///////////////////////////////////////////////////////////////////////////////

static const Uint8 GABLE_WAVE_DUTY_PATTERNS[4] = {
    [GABLE_PDC_12_5]    = 0b00000001,
    [GABLE_PDC_25]      = 0b00000011,
    [GABLE_PDC_50]      = 0b00001111,
    [GABLE_PDC_75]      = 0b00111111
};

static const Float32 GABLE_NOTE_FREQUENCY_TABLE[] = {

    // Octave 0
    [GABLE_NOTE_C]              = 16.35f,
    [GABLE_NOTE_C_SHARP]        = 17.32f,
    [GABLE_NOTE_D]              = 18.35f,
    [GABLE_NOTE_D_SHARP]        = 19.45f,
    [GABLE_NOTE_E]              = 20.60f,
    [GABLE_NOTE_F]              = 21.83f,
    [GABLE_NOTE_F_SHARP]        = 23.12f,
    [GABLE_NOTE_G]              = 24.50f,
    [GABLE_NOTE_G_SHARP]        = 25.96f,
    [GABLE_NOTE_A]              = 27.50f,
    [GABLE_NOTE_A_SHARP]        = 29.14f,
    [GABLE_NOTE_B]              = 30.87f,

    // Octave 1
    [GABLE_NOTE_C + 12]         = 32.70f,
    [GABLE_NOTE_C_SHARP + 12]   = 34.65f,
    [GABLE_NOTE_D + 12]         = 36.71f,
    [GABLE_NOTE_D_SHARP + 12]   = 38.89f,
    [GABLE_NOTE_E + 12]         = 41.20f,
    [GABLE_NOTE_F + 12]         = 43.65f,
    [GABLE_NOTE_F_SHARP + 12]   = 46.25f,
    [GABLE_NOTE_G + 12]         = 49.00f,
    [GABLE_NOTE_G_SHARP + 12]   = 51.91f,
    [GABLE_NOTE_A + 12]         = 55.00f,
    [GABLE_NOTE_A_SHARP + 12]   = 58.27f,
    [GABLE_NOTE_B + 12]         = 61.74f,

    // Octave 2
    [GABLE_NOTE_C + 24]         = 65.41f,
    [GABLE_NOTE_C_SHARP + 24]   = 69.30f,
    [GABLE_NOTE_D + 24]         = 73.42f,
    [GABLE_NOTE_D_SHARP + 24]   = 77.78f,
    [GABLE_NOTE_E + 24]         = 82.41f,
    [GABLE_NOTE_F + 24]         = 87.31f,
    [GABLE_NOTE_F_SHARP + 24]   = 92.50f,
    [GABLE_NOTE_G + 24]         = 98.00f,
    [GABLE_NOTE_G_SHARP + 24]   = 103.83f,
    [GABLE_NOTE_A + 24]         = 110.00f,
    [GABLE_NOTE_A_SHARP + 24]   = 116.54f,
    [GABLE_NOTE_B + 24]         = 123.47f,

    // Octave 3
    [GABLE_NOTE_C + 36]         = 130.81f,
    [GABLE_NOTE_C_SHARP + 36]   = 138.59f,
    [GABLE_NOTE_D + 36]         = 146.83f,
    [GABLE_NOTE_D_SHARP + 36]   = 155.56f,
    [GABLE_NOTE_E + 36]         = 164.81f,
    [GABLE_NOTE_F + 36]         = 174.61f,
    [GABLE_NOTE_F_SHARP + 36]   = 185.00f,
    [GABLE_NOTE_G + 36]         = 196.00f,
    [GABLE_NOTE_G_SHARP + 36]   = 207.65f,
    [GABLE_NOTE_A + 36]         = 220.00f,
    [GABLE_NOTE_A_SHARP + 36]   = 233.08f,
    [GABLE_NOTE_B + 36]         = 246.94f,

    // Octave 4
    [GABLE_NOTE_C + 48]         = 261.63f,
    [GABLE_NOTE_C_SHARP + 48]   = 277.18f,
    [GABLE_NOTE_D + 48]         = 293.66f,
    [GABLE_NOTE_D_SHARP + 48]   = 311.13f,
    [GABLE_NOTE_E + 48]         = 329.63f,
    [GABLE_NOTE_F + 48]         = 349.23f,
    [GABLE_NOTE_F_SHARP + 48]   = 369.99f,
    [GABLE_NOTE_G + 48]         = 392.00f,
    [GABLE_NOTE_G_SHARP + 48]   = 415.30f,
    [GABLE_NOTE_A + 48]         = 440.00f,
    [GABLE_NOTE_A_SHARP + 48]   = 466.16f,
    [GABLE_NOTE_B + 48]         = 493.88f,

    // Octave 5
    [GABLE_NOTE_C + 60]         = 523.25f,
    [GABLE_NOTE_C_SHARP + 60]   = 554.37f,
    [GABLE_NOTE_D + 60]         = 587.33f,
    [GABLE_NOTE_D_SHARP + 60]   = 622.25f,
    [GABLE_NOTE_E + 60]         = 659.25f,
    [GABLE_NOTE_F + 60]         = 698.46f,
    [GABLE_NOTE_F_SHARP + 60]   = 739.99f,
    [GABLE_NOTE_G + 60]         = 783.99f,
    [GABLE_NOTE_G_SHARP + 60]   = 830.61f,
    [GABLE_NOTE_A + 60]         = 880.00f,
    [GABLE_NOTE_A_SHARP + 60]   = 932.33f,
    [GABLE_NOTE_B + 60]         = 987.77f,

    // Octave 6
    [GABLE_NOTE_C + 72]         = 1046.50f,
    [GABLE_NOTE_C_SHARP + 72]   = 1108.73f,
    [GABLE_NOTE_D + 72]         = 1174.66f,
    [GABLE_NOTE_D_SHARP + 72]   = 1244.51f,
    [GABLE_NOTE_E + 72]         = 1318.51f,
    [GABLE_NOTE_F + 72]         = 1396.91f,
    [GABLE_NOTE_F_SHARP + 72]   = 1479.98f,
    [GABLE_NOTE_G + 72]         = 1567.98f,
    [GABLE_NOTE_G_SHARP + 72]   = 1661.22f,
    [GABLE_NOTE_A + 72]         = 1760.00f,
    [GABLE_NOTE_A_SHARP + 72]   = 1864.66f,
    [GABLE_NOTE_B + 72]         = 1975.53f,

    // Octave 7
    [GABLE_NOTE_C + 84]         = 2093.00f,
    [GABLE_NOTE_C_SHARP + 84]   = 2217.46f,
    [GABLE_NOTE_D + 84]         = 2349.32f,
    [GABLE_NOTE_D_SHARP + 84]   = 2489.02f,
    [GABLE_NOTE_E + 84]         = 2637.02f,
    [GABLE_NOTE_F + 84]         = 2793.83f,
    [GABLE_NOTE_F_SHARP + 84]   = 2959.96f,
    [GABLE_NOTE_G + 84]         = 3135.96f,
    [GABLE_NOTE_G_SHARP + 84]   = 3322.44f,
    [GABLE_NOTE_A + 84]         = 3520.00f,
    [GABLE_NOTE_A_SHARP + 84]   = 3729.31f,
    [GABLE_NOTE_B + 84]         = 3951.07f

};

// Audio Channel Structures ////////////////////////////////////////////////////////////////////////

typedef struct GABLE_PulseChannel
{

    // Hardware Registers
    GABLE_PulseFrequencySweep       m_FrequencySweep;           ///< @brief The frequency sweep unit (`PC1` only).
    GABLE_PulseLengthDuty           m_LengthDuty;               ///< @brief The length timer and duty cycle control.
    GABLE_VolumeEnvelope            m_VolumeEnvelope;           ///< @brief The volume and envelope control.
    GABLE_PeriodLowByte             m_PeriodLow;                ///< @brief The period low byte.
    GABLE_PeriodHighControl         m_PeriodHighControl;        ///< @brief The period high bits and control control.

    // Internal Registers
    Uint16                          m_CurrentPeriod;            ///< @brief The current period of the channel.
    Uint16                          m_PeriodDivider;            ///< @brief The period divider.
    Uint8                           m_CurrentLengthTimer;       ///< @brief The current length timer.
    Uint8                           m_CurrentVolume;            ///< @brief The current volume.
    Uint8                           m_CurrentWavePointer;       ///< @brief Points to the current wave sample in the current duty cycle pattern.
    Uint8                           m_CurrentFrequencyTicks;    ///< @brief The frequency sweep unit's current tick counter (`PC1` only).
    Uint8                           m_CurrentEnvelopeTicks;     ///< @brief The envelope sweep unit's current tick counter.

    // Digital-to-Analog Converter (DAC)
    Bool                            m_DACEnabled;               ///< @brief The channel's DAC enable flag.
    Uint8                           m_DACInput;                 ///< @brief The DAC's digital input.
    Float32                         m_DACOutput;                ///< @brief The DAC's analog output.

} GABLE_PulseChannel;

typedef struct GABLE_WaveChannel
{
    
    // Hardware Registers
    GABLE_WaveDACEnable             m_DACEnable;                    ///< @brief The wave DAC enable control.
    GABLE_WaveOutputLevelControl    m_OutputLevel;                  ///< @brief The wave output level control.
    GABLE_WaveLengthTimer           m_LengthTimer;                  ///< @brief The wave length timer control.
    GABLE_PeriodLowByte             m_PeriodLow;                    ///< @brief The period low byte.
    GABLE_PeriodHighControl         m_PeriodHighControl;            ///< @brief The period high bits and control unit.

    // Wave Memory Buffer
    Uint8                           m_WaveRAM[GABLE_WAVE_RAM_SIZE]; ///< @brief The wave channel's wave RAM buffer.

    // Internal Registers
    Uint16                          m_CurrentPeriod;                ///< @brief The current period of the channel.
    Uint16                          m_PeriodDivider;                ///< @brief The period divider.
    Uint8                           m_CurrentLengthTimer;           ///< @brief The current length timer.
    Uint8                           m_CurrentSampleIndex;           ///< @brief Points to the current 4-bit sample in the wave RAM buffer.

    // Digital-to-Analog Converter (DAC) - The wave channel's DAC enable flag is stored in the `m_DACEnable` register.
    Uint8                           m_DACInput;                     ///< @brief The DAC's digital input.
    Float32                         m_DACOutput;                    ///< @brief The DAC's analog output.

} GABLE_WaveChannel;

typedef struct GABLE_NoiseChannel
{

    // Hardware Registers
    GABLE_NoiseLengthTimer          m_LengthTimer;                  ///< @brief The noise length timer control.
    GABLE_VolumeEnvelope            m_VolumeEnvelope;               ///< @brief The volume and envelope control.
    GABLE_NoiseFrequencyRandomness  m_FrequencyRandomness;          ///< @brief The noise channel's LFSR control.
    GABLE_NoiseControl              m_Control;                      ///< @brief The noise channel's control register.

    // Internal Registers
    Uint16                          m_LFSR;                         ///< @brief The noise channel's linear feedback shift register.
    Uint8                           m_CurrentLengthTimer;           ///< @brief The current length timer.
    Uint8                           m_CurrentVolume;                ///< @brief The current volume.
    Uint8                           m_CurrentEnvelopeTicks;         ///< @brief The envelope sweep unit's current tick counter.
    Uint64                          m_CurrentClockFrequency;        ///< @brief The current clock frequency of the noise channel.

    // Digital-to-Analog Converter (DAC)
    Bool                            m_DACEnabled;                   ///< @brief The channel's DAC enable flag.
    Uint8                           m_DACInput;                     ///< @brief The DAC's digital input.
    Float32                         m_DACOutput;                    ///< @brief The DAC's analog output.

} GABLE_NoiseChannel;

// Audio Processing Unit (APU) Structure ///////////////////////////////////////////////////////////

typedef struct GABLE_APU
{

    // Control Registers
    GABLE_AudioMasterControl        m_MasterControl;                ///< @brief The audio master control register.
    GABLE_SoundPanning              m_SoundPanning;                 ///< @brief The sound panning control register.
    GABLE_MasterVolumeControl       m_MasterVolumeControl;          ///< @brief The master volume control and VIN panning register.

    // Audio Channels
    GABLE_PulseChannel              m_PulseChannel1;                ///< @brief The first pulse channel.
    GABLE_PulseChannel              m_PulseChannel2;                ///< @brief The second pulse channel.
    GABLE_WaveChannel               m_WaveChannel;                  ///< @brief The wave channel.
    GABLE_NoiseChannel              m_NoiseChannel;                 ///< @brief The noise channel.

    // Audio Sample Buffer
    GABLE_AudioSample               m_AudioSample;                  ///< @brief The current audio sample mixed by the APU.

    // Mix Handler and State
    GABLE_AudioMixCallback          m_MixCallback;                  ///< @brief The audio mix callback function.
    Float32                         m_PreviousLeftInput;            ///< @brief The previous left speaker input.
    Float32                         m_PreviousRightInput;           ///< @brief The previous right speaker input.
    Float32                         m_PreviousLeftOutput;           ///< @brief The previous left speaker output.
    Float32                         m_PreviousRightOutput;          ///< @brief The previous right speaker output.

    // Internal Registers
    Uint16                          m_Divider;                      ///< @brief The APU's internal divider.
    Uint64                          m_MixClockFrequency;            ///< @brief How often should the mix callback be called.

} GABLE_APU;

// Static Function Prototypes //////////////////////////////////////////////////////////////////////

static void GABLE_TriggerChannelInternal (GABLE_APU* p_APU, GABLE_AudioChannel p_Channel);
static Uint8 GABLE_ReadWaveNibble (const GABLE_APU* p_APU, Uint8 p_Index);
static void GABLE_WriteWaveNibble (GABLE_APU* p_APU, Uint8 p_Index, Uint8 p_Value);
static void GABLE_TickPulseChannels (GABLE_APU* p_APU);
static void GABLE_TickWaveChannel (GABLE_APU* p_APU);
static void GABLE_TickNoiseChannel (GABLE_APU* p_APU);
static void GABLE_TickLengthTimers (GABLE_APU* p_APU);
static void GABLE_TickFrequencySweep (GABLE_APU* p_APU);
static void GABLE_TickEnvelopeSweeps (GABLE_APU* p_APU);
static void GABLE_UpdateAudioSample (GABLE_Engine* p_Engine, GABLE_APU* p_APU);

// Static Functions ////////////////////////////////////////////////////////////////////////////////

void GABLE_TriggerChannelInternal (GABLE_APU* p_APU, GABLE_AudioChannel p_Channel)
{
    // Channels can only be triggered if the APU is enabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    switch (p_Channel)
    {
        case GABLE_AC_PULSE_1:
        {
            GABLE_PulseChannel* p_Channel = &p_APU->m_PulseChannel1;

            // Trigger the channel
            p_Channel->m_CurrentLengthTimer = p_Channel->m_LengthDuty.m_InitialLength;
            p_Channel->m_CurrentVolume = p_Channel->m_VolumeEnvelope.m_InitialVolume;
            p_Channel->m_CurrentPeriod = (p_Channel->m_PeriodHighControl.m_PeriodHigh << 8) | p_Channel->m_PeriodLow.m_PeriodLow;
            p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
            p_Channel->m_CurrentWavePointer = 0;
            p_Channel->m_CurrentFrequencyTicks = 0;
            p_Channel->m_CurrentEnvelopeTicks = 0;

            // Enable the channel only if its DAC is enabled.
            p_APU->m_MasterControl.m_PC1Enable = p_Channel->m_DACEnabled;

        } break;

        case GABLE_AC_PULSE_2:
        {
            GABLE_PulseChannel* p_Channel = &p_APU->m_PulseChannel2;

            // Trigger the channel
            p_Channel->m_CurrentLengthTimer = p_Channel->m_LengthDuty.m_InitialLength;
            p_Channel->m_CurrentVolume = p_Channel->m_VolumeEnvelope.m_InitialVolume;
            p_Channel->m_CurrentPeriod = (p_Channel->m_PeriodHighControl.m_PeriodHigh << 8) | p_Channel->m_PeriodLow.m_PeriodLow;
            p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
            p_Channel->m_CurrentWavePointer = 0;
            p_Channel->m_CurrentFrequencyTicks = 0;
            p_Channel->m_CurrentEnvelopeTicks = 0;

            // Enable the channel only if its DAC is enabled.
            p_APU->m_MasterControl.m_PC2Enable = p_Channel->m_DACEnabled;

        } break;

        case GABLE_AC_WAVE:
        {
            GABLE_WaveChannel* p_Channel = &p_APU->m_WaveChannel;

            // Trigger the channel
            p_Channel->m_CurrentLengthTimer = p_Channel->m_LengthTimer.m_InitialLength;
            p_Channel->m_CurrentPeriod = (p_Channel->m_PeriodHighControl.m_PeriodHigh << 8) | p_Channel->m_PeriodLow.m_PeriodLow;
            p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
            p_Channel->m_CurrentSampleIndex = 0;

            // Enable the channel only if its DAC is enabled.
            p_APU->m_MasterControl.m_WCEnable = p_Channel->m_DACEnable.m_DACPower;

        } break;

        case GABLE_AC_NOISE:
        {
            GABLE_NoiseChannel* p_Channel = &p_APU->m_NoiseChannel;

            // Trigger the channel
            p_Channel->m_CurrentLengthTimer = p_Channel->m_LengthTimer.m_InitialLength;
            p_Channel->m_CurrentVolume = p_Channel->m_VolumeEnvelope.m_InitialVolume;
            p_Channel->m_LFSR = 0;
            p_Channel->m_CurrentEnvelopeTicks = 0;

            // Enable the channel only if its DAC is enabled.
            p_APU->m_MasterControl.m_NCEnable = p_Channel->m_DACEnabled;

        } break;
    }
}

Uint8 GABLE_ReadWaveNibble (const GABLE_APU* p_APU, Uint8 p_Index)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    GABLE_expect(p_Index < GABLE_WAVE_RAM_NIBBLES, "Wave RAM index out of bounds!");
    
    // Get the byte index and the nibble index.
    Uint8 l_ByteIndex = p_Index / 2;
    Uint8 l_NibbleIndex = p_Index % 2;

    // Get the byte from the wave RAM buffer.
    Uint8 l_Byte = p_APU->m_WaveChannel.m_WaveRAM[l_ByteIndex];

    // Return the nibble from the byte.
    return (l_NibbleIndex == 0) ? ((l_Byte >> 4) & 0xF) : (l_Byte & 0xF);
}

void GABLE_WriteWaveNibble (GABLE_APU* p_APU, Uint8 p_Index, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    GABLE_expect(p_Index < GABLE_WAVE_RAM_NIBBLES, "Wave RAM index out of bounds!");
    
    // Get the byte index and the nibble index.
    Uint8 l_ByteIndex = p_Index / 2;
    Uint8 l_NibbleIndex = p_Index % 2;

    // Get the byte from the wave RAM buffer.
    Uint8 l_Byte = p_APU->m_WaveChannel.m_WaveRAM[l_ByteIndex];

    // Set the nibble in the byte.
    if (l_NibbleIndex == 0)
    {
        l_Byte &= 0x0F;
        l_Byte |= (p_Value << 4);
    }
    else
    {
        l_Byte &= 0xF0;
        l_Byte |= p_Value;
    }

    // Write the byte back to the wave RAM buffer.
    p_APU->m_WaveChannel.m_WaveRAM[l_ByteIndex] = l_Byte;
}

void GABLE_TickPulseChannels (GABLE_APU* p_APU)
{

    // Point to PC1.
    GABLE_PulseChannel* p_Channel = &p_APU->m_PulseChannel1;

    // If the channel is enabled, then increment its period divider. When that value exceeds 2047 (0x7FF),
    // the divider overflows and is then reset to the channel's period.
    if (p_APU->m_MasterControl.m_PC1Enable && ++p_Channel->m_PeriodDivider > 0x800)
    {

        // Reset the period divider to the channel's current period. The difference between this
        // value and the overflow value (0x800) determines the channel's frequency.
        p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;

        // Advance the wave pointer by one place.
        p_Channel->m_CurrentWavePointer = (p_Channel->m_CurrentWavePointer + 1) & 0b111;

        // The channel's DAC input value is set to the pointed-to bit in the current duty cycle pattern...
        p_Channel->m_DACInput = (
            GABLE_WAVE_DUTY_PATTERNS[p_Channel->m_LengthDuty.m_DutyCycle] 
                >> p_Channel->m_CurrentWavePointer
        ) & 0b1;

        // ...then multiplied by the channel's current volume.
        p_Channel->m_DACInput *= p_Channel->m_CurrentVolume;

        // The channel's DAC input is then translated to an analog DAC output value, by...
        // - Dividing the input by 7.5, then subtracting 1.0.
        p_Channel->m_DACOutput = -(((Float32) p_Channel->m_DACInput / 7.5f) - 1.0f);

    }

    // Repeat the above process for PC2.
    p_Channel = &p_APU->m_PulseChannel2;
    if (p_APU->m_MasterControl.m_PC2Enable && ++p_Channel->m_PeriodDivider > 0x800)
    {
        p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
        p_Channel->m_CurrentWavePointer = (p_Channel->m_CurrentWavePointer + 1) & 0b111;
        p_Channel->m_DACInput = (
            GABLE_WAVE_DUTY_PATTERNS[p_Channel->m_LengthDuty.m_DutyCycle] 
                >> p_Channel->m_CurrentWavePointer
        ) & 0b1;
        p_Channel->m_DACInput *= p_Channel->m_CurrentVolume;
        p_Channel->m_DACOutput = -(((Float32) p_Channel->m_DACInput / 7.5f) - 1.0f);
    }

}

void GABLE_TickWaveChannel (GABLE_APU* p_APU)
{

    // Point to the wave channel.
    GABLE_WaveChannel* p_Channel = &p_APU->m_WaveChannel;

    // If the channel is enabled, then increment its period divider. When that value exceeds 2047 (0x7FF),
    // the divider overflows and is then reset to the channel's period.
    if (p_APU->m_MasterControl.m_WCEnable && ++p_Channel->m_PeriodDivider > 0x800)
    {

        // Reset the period divider to the channel's current period. The difference between this
        // value and the overflow value (0x800) determines the channel's frequency.
        p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
        
        // Advance the sample index by one place.
        p_Channel->m_CurrentSampleIndex = (p_Channel->m_CurrentSampleIndex + 1) % GABLE_WAVE_RAM_NIBBLES;

        // The channel's DAC input value is set to the pointed-to 4-bit sample in the wave RAM buffer...
        p_Channel->m_DACInput = GABLE_ReadWaveNibble(p_APU, p_Channel->m_CurrentSampleIndex);

        // ...then affected by the channel's wave output level.
        switch (p_Channel->m_OutputLevel.m_OutputLevel)
        {
            case GABLE_WOL_MUTE:
                p_Channel->m_DACInput = 0;
                break;
            case GABLE_WOL_FULL:
                break;
            case GABLE_WOL_HALF:
                p_Channel->m_DACInput >>= 1;
                break;
            case GABLE_WOL_QUARTER:
                p_Channel->m_DACInput >>= 2;
                break;
        }

        // The channel's DAC input is then translated to an analog DAC output value, by...
        // - Dividing the input by 7.5, then subtracting 1.0.
        p_Channel->m_DACOutput = -(((Float32) p_Channel->m_DACInput / 7.5f) - 1.0f);

    }

}

void GABLE_TickNoiseChannel (GABLE_APU* p_APU)
{

    // Point to the noise channel.
    GABLE_NoiseChannel* p_Channel = &p_APU->m_NoiseChannel;

    // If the channel is enabled, then update the channel's LFSR.
    if (p_APU->m_MasterControl.m_NCEnable)
    {

        // Get bits 0 and 1 of the LFSR.
        Uint8 bit0 = (p_Channel->m_LFSR >> 0) & 0b1;
        Uint8 bit1 = (p_Channel->m_LFSR >> 1) & 0b1;

        // Determine the new value of bit 15 (and 7 if the LFSR width is set).
        Uint8 bit15 = (bit0 == bit1) ? 1 : 0;

        // Set the new value of bit 15 (and 7 if the LFSR width is set).
        p_Channel->m_LFSR |= (bit15 << 15);
        if (p_Channel->m_FrequencyRandomness.m_LFSRWidth)
        {
            p_Channel->m_LFSR |= (bit15 << 7);
        }

        // Shift the LFSR right by one bit, then clear the new value of bit 15 (and 7 if the LFSR width is set).
        p_Channel->m_LFSR >>= 1;
        p_Channel->m_LFSR &= ~(1 << 15);
        if (p_Channel->m_FrequencyRandomness.m_LFSRWidth)
        {
            p_Channel->m_LFSR &= ~(1 << 7);
        }

        // The new value of bit 0 determines the DAC input value, which is then multiplied by the channel's current volume.
        p_Channel->m_DACInput = (p_Channel->m_LFSR & 0b1) * p_Channel->m_CurrentVolume;

        // The channel's DAC input is then translated to an analog DAC output value, by...
        // - Dividing the input by 7.5, then subtracting 1.0.
        p_Channel->m_DACOutput = -(((Float32) p_Channel->m_DACInput / 7.5f) - 1.0f);

    }

}

void GABLE_TickLengthTimers (GABLE_APU* p_APU)
{

    // Point to PC1.
    GABLE_PulseChannel* p_Channel = &p_APU->m_PulseChannel1;

    // If the channel and its length timer are enabled, then increment the length timer.
    if (p_APU->m_MasterControl.m_PC1Enable && p_Channel->m_CurrentLengthTimer)
    {
        
        // Increment the length timer. If it reaches `0b111111`, then the channel is disabled.
        if (++p_Channel->m_CurrentLengthTimer >= 0b111111)
        {
            p_APU->m_MasterControl.m_PC1Enable = false;
        }

    }

    // Repeat the above process for PC2.
    p_Channel = &p_APU->m_PulseChannel2;
    if (p_APU->m_MasterControl.m_PC2Enable && p_Channel->m_CurrentLengthTimer)
    {
        if (++p_Channel->m_CurrentLengthTimer >= 0b111111)
        {
            p_APU->m_MasterControl.m_PC2Enable = false;
        }
    }

    // Repeat the above process for WC. For the wave channel, the length timer threshold is
    // `0b11111111` (255).
    GABLE_WaveChannel* p_WaveChannel = &p_APU->m_WaveChannel;
    if (p_APU->m_MasterControl.m_WCEnable && p_WaveChannel->m_CurrentLengthTimer)
    {
        if (++p_WaveChannel->m_CurrentLengthTimer == 0b11111111)
        {
            p_APU->m_MasterControl.m_WCEnable = false;
        }
    }

    // Repeat the above process for NC.
    GABLE_NoiseChannel* p_NoiseChannel = &p_APU->m_NoiseChannel;
    if (p_APU->m_MasterControl.m_NCEnable && p_NoiseChannel->m_CurrentLengthTimer)
    {
        if (++p_NoiseChannel->m_CurrentLengthTimer >= 0b111111)
        {
            p_APU->m_MasterControl.m_NCEnable = false;
        }
    }

}

void GABLE_TickFrequencySweep (GABLE_APU* p_APU)
{

    // Point to PC1.
    GABLE_PulseChannel* p_Channel = &p_APU->m_PulseChannel1;

    // Check to see if the channel is enabled, and the step setting in its frequency sweep unit is
    // non-zero.
    if (p_APU->m_MasterControl.m_PC1Enable && p_Channel->m_FrequencySweep.m_IndividualStep)
    {

        // Figure out how much to shift the period divider by.
        Uint16 l_PeriodDelta = (p_Channel->m_CurrentPeriod >> p_Channel->m_FrequencySweep.m_IndividualStep);

        // If the frequency sweep unit is sweeping the frequency upwards, and increasing the period
        // by the above-calculated delta would exceed 2047 (0x7FF), then the channel is disabled,
        // instead. Early out in this case, too.
        if (
            p_Channel->m_FrequencySweep.m_Direction == GABLE_FSD_INCREASE &&
            p_Channel->m_CurrentPeriod + l_PeriodDelta > 0x7FF
        )
        {
            p_APU->m_MasterControl.m_PC1Enable = false;
            return;
        }

        // Incremeht the frequency sweep unit's tick counter. Once it reaches the sweep pace value,
        // then update the frequency sweep unit.
        if (++p_Channel->m_CurrentFrequencyTicks >= p_Channel->m_FrequencySweep.m_SweepPace)
        {
            
            // Reset the frequency sweep unit's tick counter.
            p_Channel->m_CurrentFrequencyTicks = 0;

            // If the frequency sweep unit is sweeping the frequency upwards, then increase the period
            // by the delta value.
            if (p_Channel->m_FrequencySweep.m_Direction == GABLE_FSD_INCREASE)
            {
                p_Channel->m_CurrentPeriod += l_PeriodDelta;
            }

            // If the frequency sweep unit is sweeping the frequency downwards, then decrease the period
            // by the delta value.
            else
            {
                p_Channel->m_CurrentPeriod -= l_PeriodDelta;
            }

            // Update the period divider with the new period value.
            p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
            
        }

    }

}

void GABLE_TickEnvelopeSweeps (GABLE_APU* p_APU)
{

    // Point to `PC1`.
    GABLE_PulseChannel* p_Channel = &p_APU->m_PulseChannel1;

    // If the channel is enabled, and the envelope sweep unit's pace is non-zero, then increment the
    // envelope sweep unit's tick counter.
    if (p_APU->m_MasterControl.m_PC1Enable && p_Channel->m_VolumeEnvelope.m_SweepPace > 0)
    {

        // Once the tick counter reaches the sweep pace value, then update the envelope sweep unit.
        if (++p_Channel->m_CurrentEnvelopeTicks >= p_Channel->m_VolumeEnvelope.m_SweepPace)
        {

            // Reset the envelope sweep unit's tick counter.
            p_Channel->m_CurrentEnvelopeTicks = 0;

            // If the envelope sweep unit is increasing the volume, and the volume is less than 15,
            // then increment the volume.
            if (
                p_Channel->m_VolumeEnvelope.m_Direction == GABLE_ESD_INCREASE &&
                p_Channel->m_CurrentVolume < 0xF
            )
            {
                p_Channel->m_CurrentVolume++;
            }

            // If the envelope sweep unit is decreasing the volume, and the volume is greater than 0,
            // then decrement the volume.
            else if (
                p_Channel->m_VolumeEnvelope.m_Direction == GABLE_ESD_DECREASE &&
                p_Channel->m_CurrentVolume > 0
            )
            {
                p_Channel->m_CurrentVolume--;
            }

        }

    }

    // Repeat the above process for `PC2`.
    p_Channel = &p_APU->m_PulseChannel2;
    if (p_APU->m_MasterControl.m_PC2Enable && p_Channel->m_VolumeEnvelope.m_SweepPace > 0)
    {
        if (++p_Channel->m_CurrentEnvelopeTicks >= p_Channel->m_VolumeEnvelope.m_SweepPace)
        {
            if (
                p_Channel->m_VolumeEnvelope.m_Direction == GABLE_ESD_INCREASE &&
                p_Channel->m_CurrentVolume < 0xF
            )
            {
                p_Channel->m_CurrentVolume++;
            }
            else if (
                p_Channel->m_VolumeEnvelope.m_Direction == GABLE_ESD_DECREASE &&
                p_Channel->m_CurrentVolume > 0
            )
            {
                p_Channel->m_CurrentVolume--;
            }
        }
    }

    // Repeat the above process for `NC`.
    GABLE_NoiseChannel* p_NoiseChannel = &p_APU->m_NoiseChannel;
    if (p_APU->m_MasterControl.m_NCEnable && p_NoiseChannel->m_VolumeEnvelope.m_SweepPace > 0)
    {
        if (++p_NoiseChannel->m_CurrentEnvelopeTicks >= p_NoiseChannel->m_VolumeEnvelope.m_SweepPace)
        {
            if (
                p_NoiseChannel->m_VolumeEnvelope.m_Direction == GABLE_ESD_INCREASE &&
                p_NoiseChannel->m_CurrentVolume < 0xF
            )
            {
                p_NoiseChannel->m_CurrentVolume++;
            }
            else if (
                p_NoiseChannel->m_VolumeEnvelope.m_Direction == GABLE_ESD_DECREASE &&
                p_NoiseChannel->m_CurrentVolume > 0
            )
            {
                p_NoiseChannel->m_CurrentVolume--;
            }
        }
    }

}

void GABLE_UpdateAudioSample (GABLE_Engine* p_Engine, GABLE_APU* p_APU)
{

    // Reset the audio sample.
    p_APU->m_AudioSample.m_Left = 0.0f;
    p_APU->m_AudioSample.m_Right = 0.0f;

    // For each channel, if the channel and its DAC are enabled, then add the channel's DAC output
    // to the audio sample.
    if (p_APU->m_MasterControl.m_PC1Enable && p_APU->m_PulseChannel1.m_DACEnabled)
    {
        if (p_APU->m_SoundPanning.m_PC1Left)
        {
            p_APU->m_AudioSample.m_Left += p_APU->m_PulseChannel1.m_DACOutput;
        }

        if (p_APU->m_SoundPanning.m_PC1Right)
        {
            p_APU->m_AudioSample.m_Right += p_APU->m_PulseChannel1.m_DACOutput;
        }
    }

    if (p_APU->m_MasterControl.m_PC2Enable && p_APU->m_PulseChannel2.m_DACEnabled)
    {
        if (p_APU->m_SoundPanning.m_PC2Left)
        {
            p_APU->m_AudioSample.m_Left += p_APU->m_PulseChannel2.m_DACOutput;
        }

        if (p_APU->m_SoundPanning.m_PC2Right)
        {
            p_APU->m_AudioSample.m_Right += p_APU->m_PulseChannel2.m_DACOutput;
        }
    }

    if (p_APU->m_MasterControl.m_WCEnable && p_APU->m_WaveChannel.m_DACEnable.m_DACPower)
    {
        if (p_APU->m_SoundPanning.m_WCLeft)
        {
            p_APU->m_AudioSample.m_Left += p_APU->m_WaveChannel.m_DACOutput;
        }

        if (p_APU->m_SoundPanning.m_WCRight)
        {
            p_APU->m_AudioSample.m_Right += p_APU->m_WaveChannel.m_DACOutput;
        }
    }

    if (p_APU->m_MasterControl.m_NCEnable && p_APU->m_NoiseChannel.m_DACEnabled)
    {
        if (p_APU->m_SoundPanning.m_NCLeft)
        {
            p_APU->m_AudioSample.m_Left += p_APU->m_NoiseChannel.m_DACOutput;
        }

        if (p_APU->m_SoundPanning.m_NCRight)
        {
            p_APU->m_AudioSample.m_Right += p_APU->m_NoiseChannel.m_DACOutput;
        }
    }

    // Affect the total DAC output by the master volume control.
    p_APU->m_AudioSample.m_Left *= p_APU->m_MasterVolumeControl.m_LeftVolume / 7.5f;
    p_APU->m_AudioSample.m_Right *= p_APU->m_MasterVolumeControl.m_RightVolume / 7.5f;

    // Apply a high-pass filter to the audio sample to remove DC offset.
    // The filter coefficient alpha is chosen to match the Game Boy APU hardware's behavior.
    static const Float32 L_ALPHA = 0.999958f; // Example value, adjust as needed

    float l_NewLeftOutput = p_APU->m_AudioSample.m_Left - p_APU->m_PreviousLeftInput + L_ALPHA * p_APU->m_PreviousLeftOutput;
    float l_NewRightOutput = p_APU->m_AudioSample.m_Right - p_APU->m_PreviousRightInput + L_ALPHA * p_APU->m_PreviousRightOutput;

    p_APU->m_PreviousLeftInput = p_APU->m_AudioSample.m_Left;
    p_APU->m_PreviousRightInput = p_APU->m_AudioSample.m_Right;

    p_APU->m_AudioSample.m_Left = l_NewLeftOutput;
    p_APU->m_AudioSample.m_Right = l_NewRightOutput;

    p_APU->m_PreviousLeftOutput = l_NewLeftOutput;
    p_APU->m_PreviousRightOutput = l_NewRightOutput;

    // Divide the output by the number of channels mixed to bring the overall output between
    // -1.0 and 1.0.
    p_APU->m_AudioSample.m_Left /= 4.0f;
    p_APU->m_AudioSample.m_Right /= 4.0f;

    // If the mix callback is set, then call it with the audio sample.
    if (p_APU->m_MixCallback != NULL)
    {
        p_APU->m_MixCallback(p_Engine, &p_APU->m_AudioSample);
    }

}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_APU* GABLE_CreateAPU ()
{

    // Allocate the GABLE APU instance.
    GABLE_APU* l_APU = GABLE_calloc(1, GABLE_APU);
    GABLE_pexpect(l_APU != NULL, "Failed to allocate GABLE APU instance");
    GABLE_ResetAPU(l_APU);

    // Return the APU instance.
    return l_APU;

}

void GABLE_ResetAPU (GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // Reset the APU structure's memory.
    memset(p_APU, 0, sizeof(GABLE_APU));

    // Reset the APU's hardware registers.
    //
    // First, the primary control registers:
    /* NR52 = 0xF1 */   p_APU->m_MasterControl.m_Register = 0xF1;
    /* NR51 = 0xF3 */   p_APU->m_SoundPanning.m_Register = 0xF3;
    /* NR50 = 0x77 */   p_APU->m_MasterVolumeControl.m_Register = 0x77;

    // Next, the pulse channel 1 registers:
    /* NR10 = 0x80 */   p_APU->m_PulseChannel1.m_FrequencySweep.m_Register = 0x80;
    /* NR11 = 0xBF */   p_APU->m_PulseChannel1.m_LengthDuty.m_Register = 0xBF;
    /* NR12 = 0xF3 */   p_APU->m_PulseChannel1.m_VolumeEnvelope.m_Register = 0xF3;
    /* NR13 = 0xFF */   p_APU->m_PulseChannel1.m_PeriodLow.m_Register = 0xFF;
    /* NR14 = 0xBF */   p_APU->m_PulseChannel1.m_PeriodHighControl.m_Register = 0xBF;

    // Next, the pulse channel 2 registers:
    /* NR21 = 0x3F */   p_APU->m_PulseChannel2.m_LengthDuty.m_Register = 0x3F;
    /* NR22 = 0x00 */   p_APU->m_PulseChannel2.m_VolumeEnvelope.m_Register = 0x00;
    /* NR23 = 0xFF */   p_APU->m_PulseChannel2.m_PeriodLow.m_Register = 0xFF;
    /* NR24 = 0xBF */   p_APU->m_PulseChannel2.m_PeriodHighControl.m_Register = 0xBF;

    // Next, the wave channel registers:
    /* NR30 = 0x7F */   p_APU->m_WaveChannel.m_DACEnable.m_Register = 0x7F;
    /* NR31 = 0xFF */   p_APU->m_WaveChannel.m_LengthTimer.m_Register = 0xFF;
    /* NR32 = 0x9F */   p_APU->m_WaveChannel.m_OutputLevel.m_Register = 0x9F;
    /* NR33 = 0xFF */   p_APU->m_WaveChannel.m_PeriodLow.m_Register = 0xFF;
    /* NR34 = 0xBF */   p_APU->m_WaveChannel.m_PeriodHighControl.m_Register = 0xBF;

    // Next, the noise channel registers:
    /* NR41 = 0xFF */   p_APU->m_NoiseChannel.m_LengthTimer.m_Register = 0xFF;
    /* NR42 = 0x00 */   p_APU->m_NoiseChannel.m_VolumeEnvelope.m_Register = 0x00;
    /* NR43 = 0x00 */   p_APU->m_NoiseChannel.m_FrequencyRandomness.m_Register = 0x00;
    /* NR44 = 0xBF */   p_APU->m_NoiseChannel.m_Control.m_Register = 0xBF;

    // Reset the APU's mix clock frequency and the noise channel's current clock frequency.
    p_APU->m_MixClockFrequency = (4194304 / GABLE_AUDIO_SAMPLE_RATE);
    if (p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockDivider == 0)
    {
        p_APU->m_NoiseChannel.m_CurrentClockFrequency = 
            262144 / (0.5 * pow(2, p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockShift));
    }
    else
    {
        p_APU->m_NoiseChannel.m_CurrentClockFrequency = 
            262144 / (p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockDivider 
                << p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockShift);
    }
}

void GABLE_DestroyAPU (GABLE_APU* p_APU)
{
    if (p_APU != NULL)
    {
        GABLE_free(p_APU);
    }
}

void GABLE_TickAPU (GABLE_APU* p_APU, GABLE_Engine* p_Engine)
{

    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Don't tick the APU if it's disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    // Get the engine's tick count and timer instance.
    Uint64 l_TickCycles = GABLE_GetCycleCount(p_Engine);
    GABLE_Timer* l_Timer = GABLE_GetTimer(p_Engine);

    // Tick the channels:
    // - Wave channel every two ticks.
    // - Pulse channels every four ticks.
    // - Noise channel at a rate of ticks dictated by the channel's clock frequency.
    if (l_TickCycles % 2 == 0) 
        { GABLE_TickWaveChannel(p_APU); }
    if (l_TickCycles % 4 == 0) 
        { GABLE_TickPulseChannels(p_APU); }
    if (l_TickCycles % p_APU->m_NoiseChannel.m_CurrentClockFrequency == 0) 
        { GABLE_TickNoiseChannel(p_APU); }

    // Check bit 12 of the timer's divider register. If it just changed from high to low, then a
    // "DIV-APU" tick has elapsed. This is used to tick the length timers and sweep units.
    if (GABLE_CheckTimerDividerBit(l_Timer, 12) == true)
    {

        // Increment the APU's internal divider.
        p_APU->m_Divider++;

        // Tick...
        // - ...the length timers every 2 DIV-APU ticks.
        // . ...`PC1`'s frequency sweep unit every 4 DIV-APU ticks.
        // - ...the envelope sweeps every 8 DIV-APU ticks.
        if (p_APU->m_Divider % 2 == 0) 
            { GABLE_TickLengthTimers(p_APU); }
        if (p_APU->m_Divider % 4 == 0) 
            { GABLE_TickFrequencySweep(p_APU); }
        if (p_APU->m_Divider % 8 == 0) 
            { GABLE_TickEnvelopeSweeps(p_APU); }
        
    }

    // If the engine's tick count is a multiple of the APU's mix clock frequency, then update the
    // audio sample.
    if (l_TickCycles % p_APU->m_MixClockFrequency == 0)
    {
        GABLE_UpdateAudioSample(p_Engine, p_APU);
    }

}

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

Bool GABLE_ReadWaveByte (const GABLE_APU* p_APU, Uint8 p_Address, Uint8* p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    GABLE_expect(p_Value != NULL, "Value pointer is NULL!");

    if (p_Address >= GABLE_WAVE_RAM_SIZE)
    {
        GABLE_error("Wave RAM address %u out of bounds.", p_Address);
        return false;
    }

    *p_Value = p_APU->m_WaveChannel.m_WaveRAM[p_Address];
    return true;
}

Bool GABLE_WriteWaveByte (GABLE_APU* p_APU, Uint8 p_Address, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    
    if (p_Address >= GABLE_WAVE_RAM_SIZE)
    {
        GABLE_error("Wave RAM address %u out of bounds.", p_Address);
        return false;
    }

    p_APU->m_WaveChannel.m_WaveRAM[p_Address] = p_Value;
    return true;
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadNR52 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_MasterControl.m_Register;
}

Uint8 GABLE_ReadNR51 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_SoundPanning.m_Register;
}

Uint8 GABLE_ReadNR50 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_MasterVolumeControl.m_Register;
}

Uint8 GABLE_ReadNR10 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_PulseChannel1.m_FrequencySweep.m_Register;
}

Uint8 GABLE_ReadNR11 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_PulseChannel1.m_LengthDuty.m_Register & 0b11000000;
}

Uint8 GABLE_ReadNR12 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_PulseChannel1.m_VolumeEnvelope.m_Register;
}

Uint8 GABLE_ReadNR14 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_PulseChannel1.m_PeriodHighControl.m_Register;
}

Uint8 GABLE_ReadNR21 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_PulseChannel2.m_LengthDuty.m_Register & 0b11000000;
}

Uint8 GABLE_ReadNR22 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_PulseChannel2.m_VolumeEnvelope.m_Register;
}

Uint8 GABLE_ReadNR24 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_PulseChannel2.m_PeriodHighControl.m_Register;
}

Uint8 GABLE_ReadNR30 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_WaveChannel.m_DACEnable.m_Register;
}

Uint8 GABLE_ReadNR32 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_WaveChannel.m_OutputLevel.m_Register;
}

Uint8 GABLE_ReadNR34 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_WaveChannel.m_PeriodHighControl.m_Register;
}

Uint8 GABLE_ReadNR42 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_NoiseChannel.m_VolumeEnvelope.m_Register;
}

Uint8 GABLE_ReadNR43 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_NoiseChannel.m_FrequencyRandomness.m_Register;
}

Uint8 GABLE_ReadNR44 (const GABLE_APU* p_APU)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    return p_APU->m_NoiseChannel.m_Control.m_Register & 0b01111111;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void GABLE_WriteNR52 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");
    p_APU->m_MasterControl.m_Register |= (p_Value & 0b11110000);

    // If the APU is disabled, then reset all hardware registers, except for `NR52`, and make
    // them read-only.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        p_APU->m_SoundPanning.m_Register = 0x00;
        p_APU->m_MasterVolumeControl.m_Register = 0x00;
        p_APU->m_PulseChannel1.m_FrequencySweep.m_Register = 0x00;
        p_APU->m_PulseChannel1.m_LengthDuty.m_Register = 0x00;
        p_APU->m_PulseChannel1.m_VolumeEnvelope.m_Register = 0x00;
        p_APU->m_PulseChannel1.m_PeriodHighControl.m_Register = 0x00;
        p_APU->m_PulseChannel2.m_LengthDuty.m_Register = 0x00;
        p_APU->m_PulseChannel2.m_VolumeEnvelope.m_Register = 0x00;
        p_APU->m_PulseChannel2.m_PeriodHighControl.m_Register = 0x00;
        p_APU->m_WaveChannel.m_DACEnable.m_Register = 0x00;
        p_APU->m_WaveChannel.m_OutputLevel.m_Register = 0x00;
        p_APU->m_WaveChannel.m_PeriodHighControl.m_Register = 0x00;
        p_APU->m_NoiseChannel.m_VolumeEnvelope.m_Register = 0x00;
        p_APU->m_NoiseChannel.m_FrequencyRandomness.m_Register = 0x00;
        p_APU->m_NoiseChannel.m_Control.m_Register = 0x00;
    }
}

void GABLE_WriteNR51 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_SoundPanning.m_Register = p_Value;
}

void GABLE_WriteNR50 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_MasterVolumeControl.m_Register = p_Value;
}

void GABLE_WriteNR10 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel1.m_FrequencySweep.m_Register = p_Value;
}

void GABLE_WriteNR11 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel1.m_LengthDuty.m_Register = p_Value;
}

void GABLE_WriteNR12 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel1.m_VolumeEnvelope.m_Register = p_Value;

    // If the volume is zero, and the sweep direction is decreasing, then the channel and its DAC
    // are disabled.
    if (p_APU->m_PulseChannel1.m_VolumeEnvelope.m_InitialVolume == 0 &&
        p_APU->m_PulseChannel1.m_VolumeEnvelope.m_Direction == GABLE_ESD_DECREASE)
    {
        p_APU->m_PulseChannel1.m_DACEnabled = false;
        p_APU->m_MasterControl.m_PC1Enable = false;
    }
    else
    {
        p_APU->m_PulseChannel1.m_DACEnabled = true;
    }
}

void GABLE_WriteNR13 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel1.m_PeriodLow.m_PeriodLow = p_Value;
    p_APU->m_PulseChannel1.m_CurrentPeriod =
        (p_APU->m_PulseChannel1.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_PulseChannel1.m_PeriodLow.m_PeriodLow;
    p_APU->m_PulseChannel1.m_PeriodDivider = p_APU->m_PulseChannel1.m_CurrentPeriod;
}

void GABLE_WriteNR14 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel1.m_PeriodHighControl.m_Register = p_Value;
    p_APU->m_PulseChannel1.m_CurrentPeriod =
        (p_APU->m_PulseChannel1.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_PulseChannel1.m_PeriodLow.m_PeriodLow;
    p_APU->m_PulseChannel1.m_PeriodDivider = p_APU->m_PulseChannel1.m_CurrentPeriod;

    // If the trigger bit is set, then trigger the channel.
    if (p_APU->m_PulseChannel1.m_PeriodHighControl.m_Trigger)
    {
        GABLE_TriggerChannelInternal(p_APU, GABLE_AC_PULSE_1);
    }
}

void GABLE_WriteNR21 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel2.m_LengthDuty.m_Register = p_Value;
}

void GABLE_WriteNR22 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel2.m_VolumeEnvelope.m_Register = p_Value;

    // If the volume is zero, and the sweep direction is decreasing, then the channel and its DAC
    // are disabled.
    if (p_APU->m_PulseChannel2.m_VolumeEnvelope.m_InitialVolume == 0 &&
        p_APU->m_PulseChannel2.m_VolumeEnvelope.m_Direction == GABLE_ESD_DECREASE)
    {
        p_APU->m_PulseChannel2.m_DACEnabled = false;
        p_APU->m_MasterControl.m_PC2Enable = false;
    }
    else
    {
        p_APU->m_PulseChannel2.m_DACEnabled = true;
    }
}

void GABLE_WriteNR23 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel2.m_PeriodLow.m_PeriodLow = p_Value;
    p_APU->m_PulseChannel2.m_CurrentPeriod =
        (p_APU->m_PulseChannel2.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_PulseChannel2.m_PeriodLow.m_PeriodLow;
    p_APU->m_PulseChannel2.m_PeriodDivider = p_APU->m_PulseChannel2.m_CurrentPeriod;
}

void GABLE_WriteNR24 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel2.m_PeriodHighControl.m_Register = p_Value;
    p_APU->m_PulseChannel2.m_CurrentPeriod =
        (p_APU->m_PulseChannel2.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_PulseChannel2.m_PeriodLow.m_PeriodLow;
    p_APU->m_PulseChannel2.m_PeriodDivider = p_APU->m_PulseChannel2.m_CurrentPeriod;

    // If the trigger bit is set, then trigger the channel.
    if (p_APU->m_PulseChannel2.m_PeriodHighControl.m_Trigger)
    {
        GABLE_TriggerChannelInternal(p_APU, GABLE_AC_PULSE_2);
    }
}

void GABLE_WriteNR30 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_WaveChannel.m_DACEnable.m_Register = p_Value;

    // If the DAC is disabled, then the channel is disabled.
    if (p_APU->m_WaveChannel.m_DACEnable.m_DACPower == false)
    {
        p_APU->m_MasterControl.m_WCEnable = false;
    }
}

void GABLE_WriteNR31 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_WaveChannel.m_LengthTimer.m_Register = p_Value;
}

void GABLE_WriteNR32 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_WaveChannel.m_OutputLevel.m_Register = p_Value;
}

void GABLE_WriteNR33 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_WaveChannel.m_PeriodLow.m_PeriodLow = p_Value;
    p_APU->m_WaveChannel.m_CurrentPeriod =
        (p_APU->m_WaveChannel.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_WaveChannel.m_PeriodLow.m_PeriodLow;
    p_APU->m_WaveChannel.m_PeriodDivider = p_APU->m_WaveChannel.m_CurrentPeriod;
}

void GABLE_WriteNR34 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_WaveChannel.m_PeriodHighControl.m_Register = p_Value;
    p_APU->m_WaveChannel.m_CurrentPeriod =
        (p_APU->m_WaveChannel.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_WaveChannel.m_PeriodLow.m_PeriodLow;
    p_APU->m_WaveChannel.m_PeriodDivider = p_APU->m_WaveChannel.m_CurrentPeriod;

    // If the trigger bit is set, then trigger the channel.
    if (p_APU->m_WaveChannel.m_PeriodHighControl.m_Trigger)
    {
        GABLE_TriggerChannelInternal(p_APU, GABLE_AC_WAVE);
    }
}

void GABLE_WriteNR41 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_NoiseChannel.m_LengthTimer.m_Register = p_Value;
}

void GABLE_WriteNR42 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_NoiseChannel.m_VolumeEnvelope.m_Register = p_Value;

    // If the volume is zero, and the sweep direction is decreasing, then the channel and its DAC
    // are disabled.
    if (p_APU->m_NoiseChannel.m_VolumeEnvelope.m_InitialVolume == 0 &&
        p_APU->m_NoiseChannel.m_VolumeEnvelope.m_Direction == GABLE_ESD_DECREASE)
    {
        p_APU->m_NoiseChannel.m_DACEnabled = false;
        p_APU->m_MasterControl.m_NCEnable = false;
    }
    else
    {
        p_APU->m_NoiseChannel.m_DACEnabled = true;
    }
}

void GABLE_WriteNR43 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_NoiseChannel.m_FrequencyRandomness.m_Register = p_Value;

    // Update the noise channel's clock frequency.
    if (p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockDivider == 0)
    {
        p_APU->m_NoiseChannel.m_CurrentClockFrequency = 
            262144 / (0.5 * pow(2, p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockShift));
    }
    else
    {
        p_APU->m_NoiseChannel.m_CurrentClockFrequency = 
            262144 / (p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockDivider 
                << p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockShift);
    }
}

void GABLE_WriteNR44 (GABLE_APU* p_APU, Uint8 p_Value)
{
    GABLE_expect(p_APU != NULL, "APU context is NULL!");

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_NoiseChannel.m_Control.m_Register = p_Value;

    // If the trigger bit is set, then trigger the channel.
    if (p_APU->m_NoiseChannel.m_Control.m_Trigger)
    {
        GABLE_TriggerChannelInternal(p_APU, GABLE_AC_NOISE);
    }
}

// Public Functions - High-Level Functions /////////////////////////////////////////////////////////

void GABLE_SetAudioMixCallback (GABLE_Engine* p_Engine, GABLE_AudioMixCallback p_Callback)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");
    GABLE_APU* l_APU = GABLE_GetAPU(p_Engine);
    l_APU->m_MixCallback = p_Callback;
}

const GABLE_AudioSample* GABLE_GetLatestAudioSample (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");
    GABLE_APU* l_APU = GABLE_GetAPU(p_Engine);
    return &l_APU->m_AudioSample;
}
