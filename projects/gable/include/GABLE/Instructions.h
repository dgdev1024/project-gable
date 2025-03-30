/**
 * @file     GABLE/Instructions.h
 * @brief    Contains functions which simulate the execution of Game Boy CPU instructions.
 */

#pragma once
#include <GABLE/Common.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @group Instruction Simulation Functions
 * 
 * The functions below simulate the execution of the instructions found in the instruction set of
 * the Sharp LR35902 CPU, the CPU which powers the Game Boy.
 * 
 * All of these functions require a pointer to a GABLE Engine structure, and some may require
 * additional arguments, like registers, memory addresses or immediate values. All of these
 * functions return a boolean value, which is true if the instruction was executed successfully, and
 * false if an error occurred. This boolean value should be the return value of the `GABLE_CycleEngine`
 * function.
 * 
 * Not all instructions can be simulated perfectly. Because the GABLE Engine is not an emulator, it
 * does not simulate the exact behavior of the Sharp LR35902 CPU. Instead, it simulates the behavior
 * of the CPU in a way that is sufficient for creating "Gameboy-like" games. Since the GABLE Engine
 * does not have a program counter, the `JP` and `CALL` instructions require a function pointer to
 * simulate the jump or call to a different address, the `JR` and `JP HL` instructions are not
 * included in this list, and the `RET` and `RETI` instructions only cycle the engine (`RETI` still
 * enables interrupts, however).
 */

Bool G_ADC_A_R8 (GABLE_RegisterType p_Src);
Bool G_ADC_A_HL ();
Bool G_ADC_A_N8 (Uint8 p_Src);
Bool G_ADD_A_R8 (GABLE_RegisterType p_Src);
Bool G_ADD_A_HL ();
Bool G_ADD_A_N8 (Uint8 p_Src);
Bool G_ADD_HL_R16 (GABLE_RegisterType p_Src);
Bool G_ADD_HL_SP ();
Bool G_ADD_SP_E8 (Int8 p_Src);
Bool G_AND_A_R8 (GABLE_RegisterType p_Src);
Bool G_AND_A_HL ();
Bool G_AND_A_N8 (Uint8 p_Src);
Bool G_BIT_U3_R8 (Uint8 p_Bit, GABLE_RegisterType p_Src);
Bool G_BIT_U3_HL (Uint8 p_Bit);
Bool G_CALL (GABLE_ConditionType p_Condition);
Bool G_CCF ();
Bool G_CP_A_R8 (GABLE_RegisterType p_Src);
Bool G_CP_A_HL ();
Bool G_CP_A_N8 (Uint8 p_Src);
Bool G_CPL ();
Bool G_DAA ();
Bool G_DEC_R8 (GABLE_RegisterType p_Dst);
Bool G_DEC_HL ();
Bool G_DEC_R16 (GABLE_RegisterType p_Dst);
Bool G_DEC_SP ();
Bool G_DI ();
Bool G_EI ();
Bool G_HALT ();
Bool G_INC_R8 (GABLE_RegisterType p_Dst);
Bool G_INC_HL ();
Bool G_INC_R16 (GABLE_RegisterType p_Dst);
Bool G_INC_SP ();
Bool G_JP (GABLE_ConditionType p_Condition);
Bool G_JP_HL (Uint16* p_HL);
Bool G_JR (GABLE_ConditionType p_Condition);
Bool G_LD_R8_R8 (GABLE_RegisterType p_Dst, GABLE_RegisterType p_Src);
Bool G_LD_R8_N8 (GABLE_RegisterType p_Dst, Uint8 p_Src);
Bool G_LD_R16_N16 (GABLE_RegisterType p_Dst, Uint16 p_Src);
Bool G_LD_HL_R8 (GABLE_RegisterType p_Src);
Bool G_LD_HL_N8 (Uint8 p_Src);
Bool G_LD_R8_HL (GABLE_RegisterType p_Dst);
Bool G_LD_RP16_A (GABLE_RegisterType p_Dst);
Bool G_LD_A16_A (Uint16 p_Dst);
Bool G_LDH_A8_A (Uint8 p_Dst);
Bool G_LDH_C_A ();
Bool G_LD_A_RP16 (GABLE_RegisterType p_Src);
Bool G_LD_A_A16 (Uint16 p_Src);
Bool G_LDH_A_A8 (Uint8 p_Src);
Bool G_LDH_A_C ();
Bool G_LD_HLI_A ();
Bool G_LD_HLD_A ();
Bool G_LD_A_HLI ();
Bool G_LD_A_HLD ();
Bool G_LD_SP_N16 (Uint16 p_Src);
Bool G_LD_A16_SP (Uint16 p_Dst);
Bool G_LD_HL_SP_E8 (Int8 p_Src);
Bool G_LD_SP_HL ();
Bool G_NOP ();
Bool G_OR_A_R8 (GABLE_RegisterType p_Src);
Bool G_OR_A_HL ();
Bool G_OR_A_N8 (Uint8 p_Src);
Bool G_POP_R16 (GABLE_RegisterType p_Dst);
Bool G_PUSH_R16 (GABLE_RegisterType p_Src);
Bool G_RES_U3_R8 (Uint8 p_Bit, GABLE_RegisterType p_Dst);
Bool G_RES_U3_HL (Uint8 p_Bit);
Bool G_RET (GABLE_ConditionType p_Condition);
Bool G_RETI ();
Bool G_RL_R8 (GABLE_RegisterType p_Dst);
Bool G_RL_HL ();
Bool G_RLA ();
Bool G_RLC_R8 (GABLE_RegisterType p_Dst);
Bool G_RLC_HL ();
Bool G_RLCA ();
Bool G_RR_R8 (GABLE_RegisterType p_Dst);
Bool G_RR_HL ();
Bool G_RRA ();
Bool G_RRC_R8 (GABLE_RegisterType p_Dst);
Bool G_RRC_HL ();
Bool G_RRCA ();
Bool G_RST_U3 (Uint8 p_Vector);
Bool G_SBC_A_R8 (GABLE_RegisterType p_Src);
Bool G_SBC_A_HL ();
Bool G_SBC_A_N8 (Uint8 p_Src);
Bool G_SCF ();
Bool G_SET_U3_R8 (Uint8 p_Bit, GABLE_RegisterType p_Dst);
Bool G_SET_U3_HL (Uint8 p_Bit);
Bool G_SLA_R8 (GABLE_RegisterType p_Dst);
Bool G_SLA_HL ();
Bool G_SRA_R8 (GABLE_RegisterType p_Dst);
Bool G_SRA_HL ();
Bool G_SRL_R8 (GABLE_RegisterType p_Dst);
Bool G_SRL_HL ();
Bool G_STOP ();
Bool G_SUB_A_R8 (GABLE_RegisterType p_Src);
Bool G_SUB_A_HL ();
Bool G_SUB_A_N8 (Uint8 p_Src);
Bool G_SWAP_R8 (GABLE_RegisterType p_Dst);
Bool G_SWAP_HL ();
Bool G_XOR_A_R8 (GABLE_RegisterType p_Src);
Bool G_XOR_A_HL ();
Bool G_XOR_A_N8 (Uint8 p_Src);

/**
 * @endgroup
 */

#define G_JP_GOTO(C, L) if (G_JP(C)) { goto L; }
#define G_JR_GOTO(C, L) if (G_JR(C)) { goto L; }
#define G_CALL_FUNC(C, F) if (G_CALL(C)) { F; }
#define G_JUMPTABLE(...) \
    Uint16 l_HL = 0; \
    G_JP_HL(&l_HL); \
    switch (l_HL) { \
        __VA_ARGS__ \
    }
