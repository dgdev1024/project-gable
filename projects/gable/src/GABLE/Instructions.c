/**
 * @file  GABLE/Instructions.c
 */

#include <GABLE/Engine.h>
#include <GABLE/InterruptContext.h>
#include <GABLE/Instructions.h>

// Static Functions ////////////////////////////////////////////////////////////////////////////////

Bool GABLE_CheckCondition (GABLE_Engine* p_Engine, GABLE_ConditionType p_Condition)
{
    switch (p_Condition)
    {
        case GABLE_CT_NONE: return true;
        case GABLE_CT_NZ: return !GABLE_GetFlag(p_Engine, GABLE_FT_Z);
        case GABLE_CT_Z:  return  GABLE_GetFlag(p_Engine, GABLE_FT_Z);
        case GABLE_CT_NC: return !GABLE_GetFlag(p_Engine, GABLE_FT_C);
        case GABLE_CT_C:  return  GABLE_GetFlag(p_Engine, GABLE_FT_C);
        default: return false;
    }
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

Bool G_ADC_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Bool   l_Carry     = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Uint16 l_Result    = l_A + l_Src + l_Carry;
    Uint8  l_HalfCarry = (l_A & 0x0F) + (l_Src & 0x0F) + l_Carry;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, (l_Result & 0xFF) == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry > 0x0F);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result > 0xFF);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_ADC_A_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Bool   l_Carry     = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Uint16 l_Result    = l_A + l_Src + l_Carry;
    Uint8  l_HalfCarry = (l_A & 0x0F) + (l_Src & 0x0F) + l_Carry;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, (l_Result & 0xFF) == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry > 0x0F);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result > 0xFF);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_ADC_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Bool   l_Carry     = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Uint16 l_Result    = l_A + p_Src + l_Carry;
    Uint8  l_HalfCarry = (l_A & 0x0F) + (p_Src & 0x0F) + l_Carry;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, (l_Result & 0xFF) == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry > 0x0F);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result > 0xFF);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_ADD_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint16 l_Result    = l_A + l_Src;
    Uint8  l_HalfCarry = (l_A & 0x0F) + (l_Src & 0x0F);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry > 0x0F);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result > 0xFF);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_ADD_A_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint16 l_Result    = l_A + l_Src;
    Uint8  l_HalfCarry = (l_A & 0x0F) + (l_Src & 0x0F);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry > 0x0F);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result > 0xFF);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_ADD_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint16 l_Result    = l_A + p_Src;
    Uint8  l_HalfCarry = (l_A & 0x0F) + (p_Src & 0x0F);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry > 0x0F);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result > 0xFF);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_ADD_HL_R16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Src = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint16 l_HL = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_HL), "Failed to read register HL.");

    Uint32 l_Result    = l_HL + l_Src;
    Uint8  l_HalfCarry = (l_HL & 0x0FFF) + (l_Src & 0x0FFF) > 0x0FFF;
    Uint8  l_Carry     = l_Result > 0xFFFF;

    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Carry);

    return 
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_HL, l_Result & 0xFFFF) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_ADD_HL_SP (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_SP = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_SP, &l_SP), "Failed to read register SP.");

    Uint16 l_HL = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_HL), "Failed to read register HL.");

    Uint32 l_Result    = l_HL + l_SP;
    Uint8  l_HalfCarry = (l_HL & 0x0FFF) + (l_SP & 0x0FFF) > 0x0FFF;
    Uint8  l_Carry     = l_Result > 0xFFFF;

    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Carry);

    return 
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_HL, l_Result & 0xFFFF) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_ADD_SP_E8 (GABLE_Engine* p_Engine, Int8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_SP = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_SP, &l_SP), "Failed to read register SP.");

    Uint32 l_Result    = l_SP + p_Src;
    Uint8  l_HalfCarry = (l_SP & 0x0F) + (p_Src & 0x0F) > 0x0F;
    Uint8  l_Carry     = l_Result > 0xFFFF;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Carry);

    return 
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_SP, l_Result & 0xFFFF) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_AND_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Result = l_A & l_Src;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, false);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_AND_A_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Result = l_A & l_Src;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, false);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_AND_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Result = l_A & p_Src;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, false);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_BIT_U3_R8 (GABLE_Engine* p_Engine, Uint8 p_Bit, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    if (p_Bit > 7)
    {
        GABLE_error("Invalid bit index: %d.", p_Bit);
        return false;
    }

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Bool l_Result = GABLE_bit(l_Src, p_Bit);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, !l_Result);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, true);

    return GABLE_CycleEngine(p_Engine, 2);
}

Bool G_BIT_U3_HL (GABLE_Engine* p_Engine, Uint8 p_Bit)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    if (p_Bit > 7)
    {
        GABLE_error("Invalid bit index: %d.", p_Bit);
        return false;
    }

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    Bool l_Result = GABLE_bit(l_Src, p_Bit);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, !l_Result);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, true);

    return GABLE_CycleEngine(p_Engine, 3);
}

Bool G_CALL (GABLE_Engine* p_Engine, GABLE_ConditionType p_Cond)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // The simulation of this instruction does not do anything except for cycle the engine.
    // This function will return false if the condition is not met, even though the instruction
    // itself does not fail.
    //
    // The intended use of this function is to call it, then call a function in your program if
    // the condition is met. This function will cycle the engine and return the condition result.
    //
    // ```
    // if (G_CALL(p_Engine, GABLE_CT_NZ) == true)
    // {
    //     MyFunction();
    // }
    // ```

    Bool l_Condition = GABLE_CheckCondition(p_Engine, p_Cond);
    return GABLE_CycleEngine(p_Engine, l_Condition ? 6 : 3) && l_Condition;
}

Bool G_CCF (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Bool l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);

    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, !l_Carry);

    return GABLE_CycleEngine(p_Engine, 1);
}

Bool G_CP_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Int32 l_Result = l_A - l_Src;
    Int32 l_HalfCarry = (l_A & 0x0F) - (l_Src & 0x0F);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry < 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result < 0);

    return GABLE_CycleEngine(p_Engine, 1);
}

Bool G_CP_A_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Int32 l_Result = l_A - l_Src;
    Int32 l_HalfCarry = (l_A & 0x0F) - (l_Src & 0x0F);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry < 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result < 0);

    return GABLE_CycleEngine(p_Engine, 2);
}

Bool G_CP_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Int32 l_Result = l_A - p_Src;
    Int32 l_HalfCarry = (l_A & 0x0F) - (p_Src & 0x0F);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry < 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result < 0);

    return GABLE_CycleEngine(p_Engine, 2);
}

Bool G_CPL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    l_A = ~l_A;

    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, true);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_A) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_DAA (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Uint8 l_HalfCarry = GABLE_GetFlag(p_Engine, GABLE_FT_H);

    if (l_HalfCarry || (l_A & 0x0F) > 9)
    {
        l_A += 0x06;
    }

    if (l_Carry || l_A > 0x9F)
    {
        l_A += 0x60;
        GABLE_SetFlag(p_Engine, GABLE_FT_C, true);
    }

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_A == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_A) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_DEC_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    l_Dst--;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, (l_Dst & 0x0F) == 0x0F);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_DEC_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    l_Dst--;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, (l_Dst & 0x0F) == 0x0F);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 3);
}

Bool G_DEC_R16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Dst = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    l_Dst--;

    return 
        GABLE_WriteWordRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_DEC_SP (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_SP = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_SP, &l_SP), "Failed to read register SP.");

    l_SP--;

    return 
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_SP, l_SP) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_DI (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    GABLE_SetInterruptMasterEnable(p_Engine, false);

    return GABLE_CycleEngine(p_Engine, 1);
}

Bool G_EI (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    GABLE_SetInterruptMasterEnable(p_Engine, true);

    return GABLE_CycleEngine(p_Engine, 1);
}

Bool G_HALT (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");
    
    // This instruction does nothing.
    return true;
}

Bool G_INC_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    l_Dst++;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, (l_Dst & 0x0F) == 0);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_INC_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    l_Dst++;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, (l_Dst & 0x0F) == 0);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 3);
}

Bool G_INC_R16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Dst = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    l_Dst++;

    return 
        GABLE_WriteWordRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_INC_SP (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_SP = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_SP, &l_SP), "Failed to read register SP.");

    l_SP++;

    return 
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_SP, l_SP) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_JP (GABLE_Engine* p_Engine, GABLE_ConditionType p_Cond)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // The simulation of this instruction does not do anything except for cycle the engine.
    // This function will return false if the condition is not met, even though the instruction
    // itself does not fail.
    //
    // The intended use of this function is to call it just before a loop or goto statement in your
    // program. This function will cycle the engine and return the condition result.
    //
    // ```
    // if (G_JP(p_Engine, GABLE_CT_NZ) == true)
    // {
    //     goto MyLabel;
    // }
    // ```

    Bool l_Condition = GABLE_CheckCondition(p_Engine, p_Cond);
    return GABLE_CycleEngine(p_Engine, l_Condition ? 4 : 3) && l_Condition;
}

Bool G_JR (GABLE_Engine* p_Engine, GABLE_ConditionType p_Cond)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // The simulation of this instruction does not do anything except for cycle the engine.
    // This function will return false if the condition is not met, even though the instruction
    // itself does not fail.
    //
    // The intended use of this function is to call it just before a loop or goto statement in your
    // program. This function will cycle the engine and return the condition result.
    //
    // ```
    // if (G_JR(p_Engine, GABLE_CT_NZ) == true)
    // {
    //     goto MyLabel;
    // }
    // ```

    Bool l_Condition = GABLE_CheckCondition(p_Engine, p_Cond);
    return GABLE_CycleEngine(p_Engine, l_Condition ? 3 : 2) && l_Condition;
}

Bool G_LD_R8_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Src) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_LD_R8_N8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, p_Src) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_R16_N16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst, Uint16 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    return 
        GABLE_WriteWordRegister(p_Engine, p_Dst, p_Src) &&
        GABLE_CycleEngine(p_Engine, 3);
}

Bool G_LD_HL_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Src) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_HL_N8 (GABLE_Engine* p_Engine, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    return 
        GABLE_WriteByte(p_Engine, l_Address, p_Src) &&
        GABLE_CycleEngine(p_Engine, 3);
}

Bool G_LD_R8_HL (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Src) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_RP16_A (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Dst = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    return 
        GABLE_WriteByte(p_Engine, l_Dst, l_A) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_A16_A (GABLE_Engine* p_Engine, Uint16 p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    return 
        GABLE_WriteByte(p_Engine, p_Dst, l_A) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_LDH_A8_A (GABLE_Engine* p_Engine, Uint8 p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    return 
        GABLE_WriteByte(p_Engine, 0xFF00 + p_Dst, l_A) &&
        GABLE_CycleEngine(p_Engine, 3);
}

Bool G_LDH_C_A (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_C = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_C, &l_C), "Failed to read register C.");

    return 
        GABLE_WriteByte(p_Engine, 0xFF00 + l_C, l_A) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_A_RP16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Src = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Src, &l_A), "Failed to read memory at address $%04X.", l_Src);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_A) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_A_A16 (GABLE_Engine* p_Engine, Uint16 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, p_Src, &l_A), "Failed to read memory at address $%04X.", p_Src);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_A) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_LDH_A_A8 (GABLE_Engine* p_Engine, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, 0xFF00 + p_Src, &l_A), "Failed to read memory at address $FF%02X.", p_Src);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_A) &&
        GABLE_CycleEngine(p_Engine, 3);
}

Bool G_LDH_A_C (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_C = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_C, &l_C), "Failed to read register C.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, 0xFF00 + l_C, &l_A), "Failed to read memory at address $FF%02X.", l_C);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_A) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_HLI_A (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_A) &&
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_HL, l_Address + 1) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_HLD_A (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_A) &&
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_HL, l_Address - 1) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_A_HLI (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Src) &&
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_HL, l_Address + 1) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_A_HLD (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Src) &&
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_HL, l_Address - 1) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_LD_SP_N16 (GABLE_Engine* p_Engine, Uint16 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    return 
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_SP, p_Src) &&
        GABLE_CycleEngine(p_Engine, 3);
}

Bool G_LD_A16_SP (GABLE_Engine* p_Engine, Uint16 p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_SP = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_SP, &l_SP), "Failed to read register SP.");

    return 
        GABLE_WriteByte(p_Engine, p_Dst, l_SP & 0xFF) &&
        GABLE_WriteByte(p_Engine, p_Dst + 1, l_SP >> 8) &&
        GABLE_CycleEngine(p_Engine, 5);
}

Bool G_LD_HL_SP_E8 (GABLE_Engine* p_Engine, Int8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_SP = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_SP, &l_SP), "Failed to read register SP.");

    Uint32 l_Result = l_SP + p_Src;
    Uint8  l_HalfCarry = (l_SP & 0x0F) + (p_Src & 0x0F) > 0x0F;
    Uint8  l_Carry = l_Result > 0xFFFF;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Carry);

    return 
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_HL, l_Result & 0xFFFF) &&
        GABLE_CycleEngine(p_Engine, 3);
}

Bool G_LD_SP_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_HL = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_HL), "Failed to read register HL.");

    return 
        GABLE_WriteWordRegister(p_Engine, GABLE_RT_SP, l_HL) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_NOP (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // This instruction does nothing.
    return GABLE_CycleEngine(p_Engine, 1);
}

Bool G_OR_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Result = l_A | l_Src;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, false);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_OR_A_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Result = l_A | l_Src;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, false);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_OR_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Result = l_A | p_Src;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, false);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_POP_R16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Popped = 0;
    GABLE_expect(GABLE_PopWord(p_Engine, &l_Popped), "Failed to pop word from stack.");

    return 
        GABLE_WriteWordRegister(p_Engine, p_Dst, l_Popped) &&
        GABLE_CycleEngine(p_Engine, 3);
}

Bool G_PUSH_R16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Src = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    return 
        GABLE_PushWord(p_Engine, l_Src) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_RES_U3_R8 (GABLE_Engine* p_Engine, Uint8 p_Bit, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    if (p_Bit > 7)
    {
        GABLE_error("Invalid bit index: %d.", p_Bit);
        return false;
    }

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    l_Dst &= ~(1 << p_Bit);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_RES_U3_HL (GABLE_Engine* p_Engine, Uint8 p_Bit)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    if (p_Bit > 7)
    {
        GABLE_error("Invalid bit index: %d.", p_Bit);
        return false;
    }

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    l_Dst &= ~(1 << p_Bit);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_RET (GABLE_Engine* p_Engine, GABLE_ConditionType p_Cond)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // The simulation of this instruction does not do anything except for cycle the engine.
    // This function will return false if the condition is not met, even though the instruction
    // itself does not fail.
    //
    // The intended use of this function is to return from a function with this function's
    // return value if the condition is met.
    //
    // ```
    // Bool l_Condition = G_RET(p_Engine, GABLE_CT_NZ);
    // if (l_Condition == true)
    // {
    //     return l_Condition;
    // }
    // ```
    
    if (p_Cond == GABLE_CT_NONE)
    {
        return GABLE_CycleEngine(p_Engine, 4);
    }

    return GABLE_CheckCondition(p_Engine, p_Cond) ?
        GABLE_CycleEngine(p_Engine, 5) :
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_RETI (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    return GABLE_ReturnFromInterrupt(p_Engine) && GABLE_CycleEngine(p_Engine, 4);
}

Bool G_RL_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    Uint8 l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Bool l_NewCarry = (l_Dst & 0x80) != 0;

    l_Dst = (l_Dst << 1) | l_Carry;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_RL_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    Uint8 l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Bool l_NewCarry = (l_Dst & 0x80) != 0;

    l_Dst = (l_Dst << 1) | l_Carry;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_RLA (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Bool l_NewCarry = (l_A & 0x80) != 0;

    l_A = (l_A << 1) | l_Carry;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_A == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_A) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_RLC_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    Bool l_NewCarry = (l_Dst & 0x80) != 0;

    l_Dst = (l_Dst << 1) | (l_Dst >> 7);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_RLC_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    Bool l_NewCarry = (l_Dst & 0x80) != 0;

    l_Dst = (l_Dst << 1) | (l_Dst >> 7);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_RLCA (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Bool l_NewCarry = (l_A & 0x80) != 0;

    l_A = (l_A << 1) | (l_A >> 7);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_A == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_A) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_RR_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    Uint8 l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Bool l_NewCarry = (l_Dst & 0x01) != 0;

    l_Dst = (l_Dst >> 1) | (l_Carry << 7);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_RR_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    Uint8 l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Bool l_NewCarry = (l_Dst & 0x01) != 0;

    l_Dst = (l_Dst >> 1) | (l_Carry << 7);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_RRA (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Bool l_NewCarry = (l_A & 0x01) != 0;

    l_A = (l_A >> 1) | (l_Carry << 7);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_A == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_A) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_RRC_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    Bool l_NewCarry = (l_Dst & 0x01) != 0;

    l_Dst = (l_Dst >> 1) | (l_Dst << 7);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_RRC_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    Bool l_NewCarry = (l_Dst & 0x01) != 0;

    l_Dst = (l_Dst >> 1) | (l_Dst << 7);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_RRCA (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Bool l_NewCarry = (l_A & 0x01) != 0;

    l_A = (l_A >> 1) | (l_A << 7);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_A == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_A) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_RST_U3 (GABLE_Engine* p_Engine, Uint8 p_Vector)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    return
        GABLE_CallRestartVector(p_Engine, p_Vector) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_SBC_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Int32 l_Result = l_A - l_Src - l_Carry;
    Int32 l_HalfCarry = (l_A & 0x0F) - (l_Src & 0x0F) - l_Carry;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, (l_Result & 0xFF) == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry < 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result < 0);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_SBC_A_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Int32 l_Result = l_A - l_Src - l_Carry;
    Int32 l_HalfCarry = (l_A & 0x0F) - (l_Src & 0x0F) - l_Carry;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, (l_Result & 0xFF) == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry < 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result < 0);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_SBC_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Carry = GABLE_GetFlag(p_Engine, GABLE_FT_C);
    Int32 l_Result = l_A - p_Src - l_Carry;
    Int32 l_HalfCarry = (l_A & 0x0F) - (p_Src & 0x0F) - l_Carry;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, (l_Result & 0xFF) == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry < 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result < 0);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_SCF (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, true);

    return GABLE_CycleEngine(p_Engine, 1);
}

Bool G_SET_U3_R8 (GABLE_Engine* p_Engine, Uint8 p_Bit, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    if (p_Bit > 7)
    {
        GABLE_error("Invalid bit index: %d.", p_Bit);
        return false;
    }

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    l_Dst |= (1 << p_Bit);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_SET_U3_HL (GABLE_Engine* p_Engine, Uint8 p_Bit)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    if (p_Bit > 7)
    {
        GABLE_error("Invalid bit index: %d.", p_Bit);
        return false;
    }

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    l_Dst |= (1 << p_Bit);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_SLA_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    Bool l_NewCarry = (l_Dst & 0x80) != 0;

    l_Dst <<= 1;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_SLA_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    Bool l_NewCarry = (l_Dst & 0x80) != 0;

    l_Dst <<= 1;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_SRA_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    Bool l_NewCarry = (l_Dst & 0x01) != 0;

    l_Dst = (l_Dst & 0x80) | (l_Dst >> 1);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_SRA_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    Bool l_NewCarry = (l_Dst & 0x01) != 0;

    l_Dst = (l_Dst & 0x80) | (l_Dst >> 1);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_SRL_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Dst, &l_Dst), "Failed to read destination register.");

    Bool l_NewCarry = (l_Dst & 0x01) != 0;

    l_Dst >>= 1;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByteRegister(p_Engine, p_Dst, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_SRL_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Dst = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Dst), "Failed to read memory at address $%04X.", l_Address);

    Bool l_NewCarry = (l_Dst & 0x01) != 0;

    l_Dst >>= 1;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Dst == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_NewCarry);

    return 
        GABLE_WriteByte(p_Engine, l_Address, l_Dst) &&
        GABLE_CycleEngine(p_Engine, 4);
}

Bool G_STOP (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // This instruction does nothing.
    return true;
}

Bool G_SUB_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Int32 l_Result = l_A - l_Src;
    Int32 l_HalfCarry = (l_A & 0x0F) - (l_Src & 0x0F);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, (l_Result & 0xFF) == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry < 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result < 0);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_SUB_A_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Int32 l_Result = l_A - l_Src;
    Int32 l_HalfCarry = (l_A & 0x0F) - (l_Src & 0x0F);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, (l_Result & 0xFF) == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry < 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result < 0);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_SUB_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Int32 l_Result = l_A - p_Src;
    Int32 l_HalfCarry = (l_A & 0x0F) - (p_Src & 0x0F);

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, (l_Result & 0xFF) == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, true);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, l_HalfCarry < 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, l_Result < 0);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result & 0xFF) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_XOR_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, p_Src, &l_Src), "Failed to read source register.");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Result = l_A ^ l_Src;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, false);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result) &&
        GABLE_CycleEngine(p_Engine, 1);
}

Bool G_XOR_A_HL (GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint16 l_Address = 0;
    GABLE_expect(GABLE_ReadWordRegister(p_Engine, GABLE_RT_HL, &l_Address), "Failed to read register HL.");

    Uint8 l_Src = 0;
    GABLE_expect(GABLE_ReadByte(p_Engine, l_Address, &l_Src), "Failed to read memory at address $%04X.", l_Address);

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Result = l_A ^ l_Src;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, false);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result) &&
        GABLE_CycleEngine(p_Engine, 2);
}

Bool G_XOR_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src)
{
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    Uint8 l_A = 0;
    GABLE_expect(GABLE_ReadByteRegister(p_Engine, GABLE_RT_A, &l_A), "Failed to read register A.");

    Uint8 l_Result = l_A ^ p_Src;

    GABLE_SetFlag(p_Engine, GABLE_FT_Z, l_Result == 0);
    GABLE_SetFlag(p_Engine, GABLE_FT_N, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_H, false);
    GABLE_SetFlag(p_Engine, GABLE_FT_C, false);

    return 
        GABLE_WriteByteRegister(p_Engine, GABLE_RT_A, l_Result) &&
        GABLE_CycleEngine(p_Engine, 2);
}
