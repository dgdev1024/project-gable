/**
 * @file     GABLE/Instructions.h
 * @brief    Contains functions which simulate the execution of Game Boy CPU instructions.
 */

#pragma once
#include <GABLE/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward-declaration of the GABLE Engine structure.
 */
typedef struct GABLE_Engine GABLE_Engine;

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

Bool G_ADC_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_ADC_A_HL (GABLE_Engine* p_Engine);
Bool G_ADC_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src);
Bool G_ADD_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_ADD_A_HL (GABLE_Engine* p_Engine);
Bool G_ADD_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src);
Bool G_ADD_HL_R16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_ADD_HL_SP (GABLE_Engine* p_Engine);
Bool G_ADD_SP_E8 (GABLE_Engine* p_Engine, Int8 p_Src);
Bool G_AND_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_AND_A_HL (GABLE_Engine* p_Engine);
Bool G_AND_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src);
Bool G_BIT_U3_R8 (GABLE_Engine* p_Engine, Uint8 p_Bit, GABLE_RegisterType p_Src);
Bool G_BIT_U3_HL (GABLE_Engine* p_Engine, Uint8 p_Bit);
Bool G_CALL (GABLE_Engine* p_Engine, GABLE_ConditionType p_Condition);
Bool G_CCF (GABLE_Engine* p_Engine);
Bool G_CP_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_CP_A_HL (GABLE_Engine* p_Engine);
Bool G_CP_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src);
Bool G_CPL (GABLE_Engine* p_Engine);
Bool G_DAA (GABLE_Engine* p_Engine);
Bool G_DEC_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_DEC_HL (GABLE_Engine* p_Engine);
Bool G_DEC_R16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_DEC_SP (GABLE_Engine* p_Engine);
Bool G_DI (GABLE_Engine* p_Engine);
Bool G_EI (GABLE_Engine* p_Engine);
Bool G_HALT (GABLE_Engine* p_Engine);
Bool G_INC_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_INC_HL (GABLE_Engine* p_Engine);
Bool G_INC_R16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_INC_SP (GABLE_Engine* p_Engine);
Bool G_JP (GABLE_Engine* p_Engine, GABLE_ConditionType p_Condition);
Bool G_JR (GABLE_Engine* p_Engine, GABLE_ConditionType p_Condition);
Bool G_LD_R8_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst, GABLE_RegisterType p_Src);
Bool G_LD_R8_N8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst, Uint8 p_Src);
Bool G_LD_R16_N16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst, Uint16 p_Src);
Bool G_LD_HL_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_LD_HL_N8 (GABLE_Engine* p_Engine, Uint8 p_Src);
Bool G_LD_R8_HL (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_LD_RP16_A (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_LD_A16_A (GABLE_Engine* p_Engine, Uint16 p_Dst);
Bool G_LDH_A8_A (GABLE_Engine* p_Engine, Uint8 p_Dst);
Bool G_LDH_C_A (GABLE_Engine* p_Engine);
Bool G_LD_A_RP16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_LD_A_A16 (GABLE_Engine* p_Engine, Uint16 p_Src);
Bool G_LDH_A_A8 (GABLE_Engine* p_Engine, Uint8 p_Src);
Bool G_LDH_A_C (GABLE_Engine* p_Engine);
Bool G_LD_HLI_A (GABLE_Engine* p_Engine);
Bool G_LD_HLD_A (GABLE_Engine* p_Engine);
Bool G_LD_A_HLI (GABLE_Engine* p_Engine);
Bool G_LD_A_HLD (GABLE_Engine* p_Engine);
Bool G_LD_SP_N16 (GABLE_Engine* p_Engine, Uint16 p_Src);
Bool G_LD_A16_SP (GABLE_Engine* p_Engine, Uint16 p_Dst);
Bool G_LD_HL_SP_E8 (GABLE_Engine* p_Engine, Int8 p_Src);
Bool G_LD_SP_HL (GABLE_Engine* p_Engine);
Bool G_NOP (GABLE_Engine* p_Engine);
Bool G_OR_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_OR_A_HL (GABLE_Engine* p_Engine);
Bool G_OR_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src);
Bool G_POP_R16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_PUSH_R16 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_RES_U3_R8 (GABLE_Engine* p_Engine, Uint8 p_Bit, GABLE_RegisterType p_Dst);
Bool G_RES_U3_HL (GABLE_Engine* p_Engine, Uint8 p_Bit);
Bool G_RET (GABLE_Engine* p_Engine, GABLE_ConditionType p_Condition);
Bool G_RETI (GABLE_Engine* p_Engine);
Bool G_RL_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_RL_HL (GABLE_Engine* p_Engine);
Bool G_RLA (GABLE_Engine* p_Engine);
Bool G_RLC_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_RLC_HL (GABLE_Engine* p_Engine);
Bool G_RLCA (GABLE_Engine* p_Engine);
Bool G_RR_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_RR_HL (GABLE_Engine* p_Engine);
Bool G_RRA (GABLE_Engine* p_Engine);
Bool G_RRC_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_RRC_HL (GABLE_Engine* p_Engine);
Bool G_RRCA (GABLE_Engine* p_Engine);
Bool G_RST_U3 (GABLE_Engine* p_Engine, Uint8 p_Vector);
Bool G_SBC_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_SBC_A_HL (GABLE_Engine* p_Engine);
Bool G_SBC_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src);
Bool G_SCF (GABLE_Engine* p_Engine);
Bool G_SET_U3_R8 (GABLE_Engine* p_Engine, Uint8 p_Bit, GABLE_RegisterType p_Dst);
Bool G_SET_U3_HL (GABLE_Engine* p_Engine, Uint8 p_Bit);
Bool G_SLA_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_SLA_HL (GABLE_Engine* p_Engine);
Bool G_SRA_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_SRA_HL (GABLE_Engine* p_Engine);
Bool G_SRL_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_SRL_HL (GABLE_Engine* p_Engine);
Bool G_STOP (GABLE_Engine* p_Engine);
Bool G_SUB_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_SUB_A_HL (GABLE_Engine* p_Engine);
Bool G_SUB_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src);
Bool G_SWAP_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Dst);
Bool G_SWAP_HL (GABLE_Engine* p_Engine);
Bool G_XOR_A_R8 (GABLE_Engine* p_Engine, GABLE_RegisterType p_Src);
Bool G_XOR_A_HL (GABLE_Engine* p_Engine);
Bool G_XOR_A_N8 (GABLE_Engine* p_Engine, Uint8 p_Src);

/**
 * @endgroup
 */

#define G_JP_GOTO(E, C, L) if (G_JP(E, C)) { goto L; }
#define G_JR_GOTO(E, C, L) if (G_JR(E, C)) { goto L; }
#define G_CALL_FUNC(E, C, F) if (G_CALL(E, C)) { F; }
