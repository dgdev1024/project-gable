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

// Quick Instruction Macros ////////////////////////////////////////////////////////////////////////

/* 0x00 */ #define gNOP                 G_NOP()
/* 0x01 */ #define gLD_BC_I(I)          G_LD_R16_N16(G_BC, I)
/* 0x02 */ #define gLD_pBC_A            G_LD_RP16_A(G_BC)
/* 0x03 */ #define gINC_BC              G_INC_R16(G_BC)
/* 0x04 */ #define gINC_B               G_INC_R8(G_B)
/* 0x05 */ #define gDEC_B               G_DEC_R8(G_B)
/* 0x06 */ #define gLD_B_I(I)           G_LD_R8_N8(G_B, I)
/* 0x07 */ #define gRLCA                G_RLCA()
/* 0x08 */ #define gLD_A16_SP_I(I)      G_LD_A16_SP(I)
/* 0x09 */ #define gADD_HL_BC           G_ADD_HL_R16(G_BC)
/* 0x0A */ #define gLD_A_pBC            G_LD_A_RP16(G_BC)
/* 0x0B */ #define gDEC_BC              G_DEC_R16(G_BC)
/* 0x0C */ #define gINC_C               G_INC_R8(G_C)
/* 0x0D */ #define gDEC_C               G_DEC_R8(G_C)
/* 0x0E */ #define gLD_C_I(I)           G_LD_R8_N8(G_C, I)
/* 0x0F */ #define gRRCA                G_RRCA()

/* 0x10 */ #define gSTOP                G_STOP()
/* 0x11 */ #define gLD_DE_I(I)          G_LD_R16_N16(G_DE, I)
/* 0x12 */ #define gLD_pDE_A            G_LD_RP16_A(G_DE)
/* 0x13 */ #define gINC_DE              G_INC_R16(G_DE)
/* 0x14 */ #define gINC_D               G_INC_R8(G_D)
/* 0x15 */ #define gDEC_D               G_DEC_R8(G_D)
/* 0x16 */ #define gLD_D_I(I)           G_LD_R8_N8(G_D, I)
/* 0x17 */ #define gRLA                 G_RLA()
/* 0x18 */ #define gJR                  G_JR(G_NOCOND)
           #define gJR_(L)              G_JR_GOTO(G_NOCOND, L)
/* 0x19 */ #define gADD_HL_DE           G_ADD_HL_R16(G_DE)
/* 0x1A */ #define gLD_A_pDE            G_LD_A_RP16(G_DE)
/* 0x1B */ #define gDEC_DE              G_DEC_R16(G_DE)
/* 0x1C */ #define gINC_E               G_INC_R8(G_E)
/* 0x1D */ #define gDEC_E               G_DEC_R8(G_E)
/* 0x1E */ #define gLD_E_I(I)           G_LD_R8_N8(G_E, I)
/* 0x1F */ #define gRRA                 G_RRA()

/* 0x20 */ #define gJR_NZ               G_JR(G_COND_NZ)
           #define gJR_NZ_(L)           G_JR_GOTO(G_COND_NZ, L)
/* 0x21 */ #define gLD_HL_I(I)          G_LD_R16_N16(G_HL, I)
/* 0x22 */ #define gLD_pHLI_A           G_LD_HLI_A()
/* 0x23 */ #define gINC_HL              G_INC_R16(G_HL)
/* 0x24 */ #define gINC_H               G_INC_R8(G_H)
/* 0x25 */ #define gDEC_H               G_DEC_R8(G_H)
/* 0x26 */ #define gLD_H_I(I)           G_LD_R8_N8(G_H, I)
/* 0x27 */ #define gDAA                 G_DAA()
/* 0x28 */ #define gJR_Z                G_JR(G_COND_Z)
           #define gJR_Z_(L)            G_JR_GOTO(G_COND_Z, L)
/* 0x29 */ #define gADD_HL_HL           G_ADD_HL_R16(G_HL)
/* 0x2A */ #define gLD_A_pHLI           G_LD_A_HLI()
/* 0x2B */ #define gDEC_HL              G_DEC_R16(G_HL)
/* 0x2C */ #define gINC_L               G_INC_R8(G_L)
/* 0x2D */ #define gDEC_L               G_DEC_R8(G_L)
/* 0x2E */ #define gLD_L_I(I)           G_LD_R8_N8(G_L, I)
/* 0x2F */ #define gCPL                 G_CPL()

/* 0x30 */ #define gJR_NC               G_JR(G_COND_NC)
           #define gJR_NC_(L)           G_JR_GOTO(G_COND_NC, L)
/* 0x31 */ #define gLD_SP_I(I)          G_LD_R16_N16(G_SP, I)
/* 0x32 */ #define gLD_pHLD_A           G_LD_HLD_A()
/* 0x33 */ #define gINC_SP              G_INC_R16(G_SP)
/* 0x34 */ #define gINC_pHL             G_INC_HL()
/* 0x35 */ #define gDEC_pHL             G_DEC_HL()
/* 0x36 */ #define gLD_pHL_I(I)         G_LD_HL_N8(I)
/* 0x37 */ #define gSCF                 G_SCF()
/* 0x38 */ #define gJR_C                G_JR(G_COND_C)
           #define gJR_C_(L)            G_JR_GOTO(G_COND_C, L)
/* 0x39 */ #define gADD_HL_SP           G_ADD_HL_SP()
/* 0x3A */ #define gLD_A_pHLD           G_LD_A_HLD()
/* 0x3B */ #define gDEC_SP              G_DEC_R16(G_SP)
/* 0x3C */ #define gINC_A               G_INC_R8(G_A)
/* 0x3D */ #define gDEC_A               G_DEC_R8(G_A)
/* 0x3E */ #define gLD_A_I(I)           G_LD_R8_N8(G_A, I)
/* 0x3F */ #define gCCF                 G_CCF()

/* 0x40 */ #define gLD_B_B             G_LD_R8_R8(G_B, G_B)
/* 0x41 */ #define gLD_B_C             G_LD_R8_R8(G_B, G_C)
/* 0x42 */ #define gLD_B_D             G_LD_R8_R8(G_B, G_D)
/* 0x43 */ #define gLD_B_E             G_LD_R8_R8(G_B, G_E)
/* 0x44 */ #define gLD_B_H             G_LD_R8_R8(G_B, G_H)
/* 0x45 */ #define gLD_B_L             G_LD_R8_R8(G_B, G_L)
/* 0x46 */ #define gLD_B_pHL           G_LD_R8_HL(G_B)
/* 0x47 */ #define gLD_B_A             G_LD_R8_R8(G_B, G_A)
/* 0x48 */ #define gLD_C_B             G_LD_R8_R8(G_C, G_B)
/* 0x49 */ #define gLD_C_C             G_LD_R8_R8(G_C, G_C)
/* 0x4A */ #define gLD_C_D             G_LD_R8_R8(G_C, G_D)
/* 0x4B */ #define gLD_C_E             G_LD_R8_R8(G_C, G_E)
/* 0x4C */ #define gLD_C_H             G_LD_R8_R8(G_C, G_H)
/* 0x4D */ #define gLD_C_L             G_LD_R8_R8(G_C, G_L)
/* 0x4E */ #define gLD_C_pHL           G_LD_R8_HL(G_C)
/* 0x4F */ #define gLD_C_A             G_LD_R8_R8(G_C, G_A)
/* 0x50 */ #define gLD_D_B             G_LD_R8_R8(G_D, G_B)
/* 0x51 */ #define gLD_D_C             G_LD_R8_R8(G_D, G_C)
/* 0x52 */ #define gLD_D_D             G_LD_R8_R8(G_D, G_D)
/* 0x53 */ #define gLD_D_E             G_LD_R8_R8(G_D, G_E) 
/* 0x54 */ #define gLD_D_H             G_LD_R8_R8(G_D, G_H)
/* 0x55 */ #define gLD_D_L             G_LD_R8_R8(G_D, G_L)
/* 0x56 */ #define gLD_D_pHL           G_LD_R8_HL(G_D)
/* 0x57 */ #define gLD_D_A             G_LD_R8_R8(G_D, G_A)
/* 0x58 */ #define gLD_E_B             G_LD_R8_R8(G_E, G_B)
/* 0x59 */ #define gLD_E_C             G_LD_R8_R8(G_E, G_C)
/* 0x5A */ #define gLD_E_D             G_LD_R8_R8(G_E, G_D)
/* 0x5B */ #define gLD_E_E             G_LD_R8_R8(G_E, G_E)
/* 0x5C */ #define gLD_E_H             G_LD_R8_R8(G_E, G_H)
/* 0x5D */ #define gLD_E_L             G_LD_R8_R8(G_E, G_L)
/* 0x5E */ #define gLD_E_pHL           G_LD_R8_HL(G_E)
/* 0x5F */ #define gLD_E_A             G_LD_R8_R8(G_E, G_A)
/* 0x60 */ #define gLD_H_B             G_LD_R8_R8(G_H, G_B)
/* 0x61 */ #define gLD_H_C             G_LD_R8_R8(G_H, G_C)
/* 0x62 */ #define gLD_H_D             G_LD_R8_R8(G_H, G_D)
/* 0x63 */ #define gLD_H_E             G_LD_R8_R8(G_H, G_E)
/* 0x64 */ #define gLD_H_H             G_LD_R8_R8(G_H, G_H)
/* 0x65 */ #define gLD_H_L             G_LD_R8_R8(G_H, G_L)
/* 0x66 */ #define gLD_H_pHL           G_LD_R8_HL(G_H)
/* 0x67 */ #define gLD_H_A             G_LD_R8_R8(G_H, G_A)
/* 0x68 */ #define gLD_L_B             G_LD_R8_R8(G_L, G_B)
/* 0x69 */ #define gLD_L_C             G_LD_R8_R8(G_L, G_C)
/* 0x6A */ #define gLD_L_D             G_LD_R8_R8(G_L, G_D)
/* 0x6B */ #define gLD_L_E             G_LD_R8_R8(G_L, G_E)
/* 0x6C */ #define gLD_L_H             G_LD_R8_R8(G_L, G_H)
/* 0x6D */ #define gLD_L_L             G_LD_R8_R8(G_L, G_L)
/* 0x6E */ #define gLD_L_pHL           G_LD_R8_HL(G_L)
/* 0x6F */ #define gLD_L_A             G_LD_R8_R8(G_L, G_A)
/* 0x70 */ #define gLD_pHL_B           G_LD_HL_R8(G_B)
/* 0x71 */ #define gLD_pHL_C           G_LD_HL_R8(G_C)
/* 0x72 */ #define gLD_pHL_D           G_LD_HL_R8(G_D)
/* 0x73 */ #define gLD_pHL_E           G_LD_HL_R8(G_E)
/* 0x74 */ #define gLD_pHL_H           G_LD_HL_R8(G_H)
/* 0x75 */ #define gLD_pHL_L           G_LD_HL_R8(G_L)
/* 0x76 */ #define gHALT               G_HALT()
/* 0x77 */ #define gLD_pHL_A           G_LD_HL_R8(G_A)
/* 0x78 */ #define gLD_A_B             G_LD_R8_R8(G_A, G_B)
/* 0x79 */ #define gLD_A_C             G_LD_R8_R8(G_A, G_C)
/* 0x7A */ #define gLD_A_D             G_LD_R8_R8(G_A, G_D)
/* 0x7B */ #define gLD_A_E             G_LD_R8_R8(G_A, G_E)
/* 0x7C */ #define gLD_A_H             G_LD_R8_R8(G_A, G_H)
/* 0x7D */ #define gLD_A_L             G_LD_R8_R8(G_A, G_L)
/* 0x7E */ #define gLD_A_pHL           G_LD_R8_HL(G_A)
/* 0x7F */ #define gLD_A_A             G_LD_R8_R8(G_A, G_A)

/* 0x80 */ #define gADD_B            G_ADD_A_R8(G_B)
/* 0x81 */ #define gADD_C            G_ADD_A_R8(G_C)
/* 0x82 */ #define gADD_D            G_ADD_A_R8(G_D)
/* 0x83 */ #define gADD_E            G_ADD_A_R8(G_E)
/* 0x84 */ #define gADD_H            G_ADD_A_R8(G_H)
/* 0x85 */ #define gADD_L            G_ADD_A_R8(G_L)
/* 0x86 */ #define gADD_pHL          G_ADD_A_HL()
/* 0x87 */ #define gADD_A            G_ADD_A_R8(G_A)
/* 0x88 */ #define gADC_B            G_ADC_A_R8(G_B)
/* 0x89 */ #define gADC_C            G_ADC_A_R8(G_C)
/* 0x8A */ #define gADC_D            G_ADC_A_R8(G_D)
/* 0x8B */ #define gADC_E            G_ADC_A_R8(G_E)
/* 0x8C */ #define gADC_H            G_ADC_A_R8(G_H)
/* 0x8D */ #define gADC_L            G_ADC_A_R8(G_L)
/* 0x8E */ #define gADC_pHL          G_ADC_A_HL()
/* 0x8F */ #define gADC_A            G_ADC_A_R8(G_A)

/* 0x90 */ #define gSUB_B              G_SUB_A_R8(G_B)
/* 0x91 */ #define gSUB_C              G_SUB_A_R8(G_C)
/* 0x92 */ #define gSUB_D              G_SUB_A_R8(G_D)
/* 0x93 */ #define gSUB_E              G_SUB_A_R8(G_E)
/* 0x94 */ #define gSUB_H              G_SUB_A_R8(G_H)
/* 0x95 */ #define gSUB_L              G_SUB_A_R8(G_L)
/* 0x96 */ #define gSUB_pHL            G_SUB_A_HL()
/* 0x97 */ #define gSUB_A              G_SUB_A_R8(G_A)
/* 0x98 */ #define gSBC_B              G_SBC_A_R8(G_B)
/* 0x99 */ #define gSBC_C              G_SBC_A_R8(G_C)
/* 0x9A */ #define gSBC_D              G_SBC_A_R8(G_D)
/* 0x9B */ #define gSBC_E              G_SBC_A_R8(G_E)
/* 0x9C */ #define gSBC_H              G_SBC_A_R8(G_H)
/* 0x9D */ #define gSBC_L              G_SBC_A_R8(G_L)
/* 0x9E */ #define gSBC_pHL            G_SBC_A_HL()
/* 0x9F */ #define gSBC_A              G_SBC_A_R8(G_A)

/* 0xA0 */ #define gAND_B              G_AND_A_R8(G_B)
/* 0xA1 */ #define gAND_C              G_AND_A_R8(G_C)
/* 0xA2 */ #define gAND_D              G_AND_A_R8(G_D)
/* 0xA3 */ #define gAND_E              G_AND_A_R8(G_E)
/* 0xA4 */ #define gAND_H              G_AND_A_R8(G_H)
/* 0xA5 */ #define gAND_L              G_AND_A_R8(G_L)
/* 0xA6 */ #define gAND_pHL            G_AND_A_HL()
/* 0xA7 */ #define gAND_A              G_AND_A_R8(G_A)
/* 0xA8 */ #define gXOR_B              G_XOR_A_R8(G_B)
/* 0xA9 */ #define gXOR_C              G_XOR_A_R8(G_C)
/* 0xAA */ #define gXOR_D              G_XOR_A_R8(G_D)
/* 0xAB */ #define gXOR_E              G_XOR_A_R8(G_E)
/* 0xAC */ #define gXOR_H              G_XOR_A_R8(G_H)
/* 0xAD */ #define gXOR_L              G_XOR_A_R8(G_L)
/* 0xAE */ #define gXOR_pHL            G_XOR_A_HL()
/* 0xAF */ #define gXOR_A              G_XOR_A_R8(G_A)

/* 0xB0 */ #define gOR_B               G_OR_A_R8(G_B)
/* 0xB1 */ #define gOR_C               G_OR_A_R8(G_C)
/* 0xB2 */ #define gOR_D               G_OR_A_R8(G_D)
/* 0xB3 */ #define gOR_E               G_OR_A_R8(G_E)
/* 0xB4 */ #define gOR_H               G_OR_A_R8(G_H)
/* 0xB5 */ #define gOR_L               G_OR_A_R8(G_L)
/* 0xB6 */ #define gOR_pHL             G_OR_A_HL()
/* 0xB7 */ #define gOR_A               G_OR_A_R8(G_A)
/* 0xB8 */ #define gCP_B               G_CP_A_R8(G_B)
/* 0xB9 */ #define gCP_C               G_CP_A_R8(G_C)
/* 0xBA */ #define gCP_D               G_CP_A_R8(G_D)
/* 0xBB */ #define gCP_E               G_CP_A_R8(G_E)
/* 0xBC */ #define gCP_H               G_CP_A_R8(G_H)
/* 0xBD */ #define gCP_L               G_CP_A_R8(G_L)
/* 0xBE */ #define gCP_pHL             G_CP_A_HL()
/* 0xBF */ #define gCP_A               G_CP_A_R8(G_A)

/* 0xC0 */ #define gRET_NZ             if (G_RET(G_COND_NZ)) { return; }
           #define gRET_NZ_(V)         if (G_RET(G_COND_NZ)) { return V; }
/* 0xC1 */ #define gPOP_BC             G_POP_R16(G_BC)
/* 0xC2 */ #define gJP_NZ              G_JP(G_COND_NZ)
           #define gJP_NZ_(L)          G_JP_GOTO(G_COND_NZ, L)
/* 0xC3 */ #define gJP                 G_JP(G_NOCOND)
           #define gJP_(L)             G_JP_GOTO(G_NOCOND, L)
/* 0xC4 */ #define gCALL_NZ            G_CALL(G_COND_NZ)
           #define gCALL_NZ_(F)        G_CALL_FUNC(G_COND_NZ, F)
/* 0xC5 */ #define gPUSH_BC            G_PUSH_R16(G_BC)
/* 0xC6 */ #define gADD_A_I(I)         G_ADD_A_N8(I)
           #define gADD_I(I)           G_ADD_A_N8(I)
/* 0xC7 */ #define gRST0               G_RST_U3(0)
/* 0xC8 */ #define gRET_Z              if (G_RET(G_COND_Z)) { return; }
           #define gRET_Z_(V)          if (G_RET(G_COND_Z)) { return V; }
/* 0xC9 */ #define gRET                if (G_RET(G_NOCOND)) { return; }
           #define gRET_(V)            if (G_RET(G_NOCOND)) { return V; }
/* 0xCA */ #define gJP_Z               G_JP(G_COND_Z)
           #define gJP_Z_(L)           G_JP_GOTO(G_COND_Z, L)
/* 0xCB */
/* 0xCC */ #define gCALL_Z             G_CALL(G_COND_Z)
           #define gCALL_Z_(F)         G_CALL_FUNC(G_COND_Z, F)
/* 0xCD */ #define gCALL               G_CALL(G_NOCOND)
           #define gCALL_(F)           G_CALL_FUNC(G_NOCOND, F)
/* 0xCE */ #define gADC_A_I(I)         G_ADC_A_N8(I)
           #define gADC_I(I)           G_ADC_A_N8(I)
/* 0xCF */ #define gRST8               G_RST_U3(1)
/* 0xD0 */ #define gRET_NC             if (G_RET(G_COND_NC)) { return; }
           #define gRET_NC_(V)         if (G_RET(G_COND_NC)) { return V; }
/* 0xD1 */ #define gPOP_DE             G_POP_R16(G_DE)
/* 0xD2 */ #define gJP_NC              G_JP(G_COND_NC)
           #define gJP_NC_(L)          G_JP_GOTO(G_COND_NC, L)
/* 0xD3 */
/* 0xD4 */ #define gCALL_NC            G_CALL(G_COND_NC)
           #define gCALL_NC_(F)        G_CALL_FUNC(G_COND_NC, F)
/* 0xD5 */ #define gPUSH_DE            G_PUSH_R16(G_DE)
/* 0xD6 */ #define gSUB_I(I)           G_SUB_A_N8(I)
           #define gSUB_A_I(I)         G_SUB_A_N8(I)
/* 0xD7 */ #define gRST10              G_RST_U3(2)
/* 0xD8 */ #define gRET_C              if (G_RET(G_COND_C)) { return; }
           #define gRET_C_(V)          if (G_RET(G_COND_C)) { return V; }
/* 0xD9 */ #define gRETI               if (G_RETI()) { return; }
           #define gRETI_(V)           if (G_RETI()) { return V; }
/* 0xDA */ #define gJP_C               G_JP(G_COND_C)
           #define gJP_C_(L)           G_JP_GOTO(G_COND_C, L)
/* 0xDB */
/* 0xDC */ #define gCALL_C             G_CALL(G_COND_C)
           #define gCALL_C_(F)         G_CALL_FUNC(G_COND_C, F)
/* 0xDD */
/* 0xDE */ #define gSBC_A_I(I)         G_SBC_A_N8(I)
           #define gSBC_I(I)           G_SBC_A_N8(I)
/* 0xDF */ #define gRST18              G_RST_U3(3)
/* 0xE0 */ #define gLDH_P_A(P)         G_LDH_A8_A(P)
/* 0xE1 */ #define gPOP_HL             G_POP_R16(G_HL)
/* 0xE2 */ #define gLDH_pC_A           G_LDH_C_A()
/* 0xE3 */
/* 0xE4 */ 
/* 0xE5 */ #define gPUSH_HL            G_PUSH_R16(G_HL)
/* 0xE6 */ #define gAND_I(I)           G_AND_A_N8(I)
           #define gAND_A_I(I)         G_AND_A_N8(I)
/* 0xE7 */ #define gRST20              G_RST_U3(4)
/* 0xE8 */ #define gADD_SP_I(I)        G_ADD_SP_E8(I)
           #define gADD_SP_E8(I)       G_ADD_SP_E8(I)
/* 0xE9 */ #define gJP_HL              G_JUMPTABLE
/* 0xEA */ #define gLD_P_A(P)          G_LD_A16_A(P)
/* 0xEB */
/* 0xEC */
/* 0xED */
/* 0xEE */ #define gXOR_I(I)           G_XOR_A_N8(I)
           #define gXOR_A_I(I)         G_XOR_A_N8(I)
/* 0xEF */ #define gRST28              G_RST_U3(5)
/* 0xF0 */ #define gLDH_A_P(P)         G_LDH_A_A8(P)
/* 0xF1 */ #define gPOP_AF             G_POP_R16(G_AF)
/* 0xF2 */ #define gLDH_A_pC           G_LDH_A_C()
/* 0xF3 */ #define gDI                 G_DI()
/* 0xF4 */
/* 0xF5 */ #define gPUSH_AF            G_PUSH_R16(G_AF)
/* 0xF6 */ #define gOR_I(I)            G_OR_A_N8(I)
           #define gOR_A_I(I)          G_OR_A_N8(I)
/* 0xF7 */ #define gRST30              G_RST_U3(6)
/* 0xF8 */ #define gLD_HL_SP_I(I)      G_LD_HL_SP_E8(I)
           #define gLD_HL_SP_E8(I)     G_LD_HL_SP_E8(I)
/* 0xF9 */ #define gLD_SP_HL           G_LD_SP_HL()
/* 0xFA */ #define gLD_A_P(P)          G_LD_A_A16(P)
/* 0xFB */ #define gEI                 G_EI()
/* 0xFC */
/* 0xFD */
/* 0xFE */ #define gCP_I(I)            G_CP_A_N8(I)
           #define gCP_A_I(I)          G_CP_A_N8(I)
/* 0xFF */ #define gRST38              G_RST_U3(7)

// Quick Instruction Macros (0xCB) /////////////////////////////////////////////////////////////////

/* 0xCB00 */ #define gRLC_B              G_RLC_R8(G_B)
/* 0xCB01 */ #define gRLC_C              G_RLC_R8(G_C)
/* 0xCB02 */ #define gRLC_D              G_RLC_R8(G_D)
/* 0xCB03 */ #define gRLC_E              G_RLC_R8(G_E)
/* 0xCB04 */ #define gRLC_H              G_RLC_R8(G_H)
/* 0xCB05 */ #define gRLC_L              G_RLC_R8(G_L)
/* 0xCB06 */ #define gRLC_pHL            G_RLC_HL()
/* 0xCB07 */ #define gRLC_A              G_RLC_R8(G_A)
/* 0xCB08 */ #define gRRC_B              G_RRC_R8(G_B)
/* 0xCB09 */ #define gRRC_C              G_RRC_R8(G_C)
/* 0xCB0A */ #define gRRC_D              G_RRC_R8(G_D)
/* 0xCB0B */ #define gRRC_E              G_RRC_R8(G_E)
/* 0xCB0C */ #define gRRC_H              G_RRC_R8(G_H)
/* 0xCB0D */ #define gRRC_L              G_RRC_R8(G_L)
/* 0xCB0E */ #define gRRC_pHL            G_RRC_HL()
/* 0xCB0F */ #define gRRC_A              G_RRC_R8(G_A)

/* 0xCB10 */ #define gRL_B               G_RL_R8(G_B)
/* 0xCB11 */ #define gRL_C               G_RL_R8(G_C)
/* 0xCB12 */ #define gRL_D               G_RL_R8(G_D)
/* 0xCB13 */ #define gRL_E               G_RL_R8(G_E)
/* 0xCB14 */ #define gRL_H               G_RL_R8(G_H)
/* 0xCB15 */ #define gRL_L               G_RL_R8(G_L)
/* 0xCB16 */ #define gRL_pHL             G_RL_HL()
/* 0xCB17 */ #define gRL_A               G_RL_R8(G_A)
/* 0xCB18 */ #define gRR_B               G_RR_R8(G_B)
/* 0xCB19 */ #define gRR_C               G_RR_R8(G_C)
/* 0xCB1A */ #define gRR_D               G_RR_R8(G_D)
/* 0xCB1B */ #define gRR_E               G_RR_R8(G_E)
/* 0xCB1C */ #define gRR_H               G_RR_R8(G_H)
/* 0xCB1D */ #define gRR_L               G_RR_R8(G_L)
/* 0xCB1E */ #define gRR_pHL             G_RR_HL()
/* 0xCB1F */ #define gRR_A               G_RR_R8(G_A)

/* 0xCB20 */ #define gSLA_B              G_SLA_R8(G_B)
/* 0xCB21 */ #define gSLA_C              G_SLA_R8(G_C)
/* 0xCB22 */ #define gSLA_D              G_SLA_R8(G_D)
/* 0xCB23 */ #define gSLA_E              G_SLA_R8(G_E)
/* 0xCB24 */ #define gSLA_H              G_SLA_R8(G_H)
/* 0xCB25 */ #define gSLA_L              G_SLA_R8(G_L)
/* 0xCB26 */ #define gSLA_pHL            G_SLA_HL()
/* 0xCB27 */ #define gSLA_A              G_SLA_R8(G_A)
/* 0xCB28 */ #define gSRA_B              G_SRA_R8(G_B)
/* 0xCB29 */ #define gSRA_C              G_SRA_R8(G_C)
/* 0xCB2A */ #define gSRA_D              G_SRA_R8(G_D)  
/* 0xCB2B */ #define gSRA_E              G_SRA_R8(G_E)
/* 0xCB2C */ #define gSRA_H              G_SRA_R8(G_H)
/* 0xCB2D */ #define gSRA_L              G_SRA_R8(G_L)
/* 0xCB2E */ #define gSRA_pHL            G_SRA_HL()
/* 0xCB2F */ #define gSRA_A              G_SRA_R8(G_A)

/* 0xCB30 */ #define gSWAP_B             G_SWAP_R8(G_B)
/* 0xCB31 */ #define gSWAP_C             G_SWAP_R8(G_C)
/* 0xCB32 */ #define gSWAP_D             G_SWAP_R8(G_D)
/* 0xCB33 */ #define gSWAP_E             G_SWAP_R8(G_E)
/* 0xCB34 */ #define gSWAP_H             G_SWAP_R8(G_H)
/* 0xCB35 */ #define gSWAP_L             G_SWAP_R8(G_L)
/* 0xCB36 */ #define gSWAP_pHL           G_SWAP_HL()
/* 0xCB37 */ #define gSWAP_A             G_SWAP_R8(G_A)
/* 0xCB38 */ #define gSRL_B              G_SRL_R8(G_B)
/* 0xCB39 */ #define gSRL_C              G_SRL_R8(G_C)
/* 0xCB3A */ #define gSRL_D              G_SRL_R8(G_D)
/* 0xCB3B */ #define gSRL_E              G_SRL_R8(G_E)
/* 0xCB3C */ #define gSRL_H              G_SRL_R8(G_H)
/* 0xCB3D */ #define gSRL_L              G_SRL_R8(G_L)
/* 0xCB3E */ #define gSRL_pHL            G_SRL_HL()
/* 0xCB3F */ #define gSRL_A              G_SRL_R8(G_A)

/* 0xCB40 */ #define gBIT0_B             G_BIT_U3_R8(0, G_B)
/* 0xCB41 */ #define gBIT0_C             G_BIT_U3_R8(0, G_C)
/* 0xCB42 */ #define gBIT0_D             G_BIT_U3_R8(0, G_D)
/* 0xCB43 */ #define gBIT0_E             G_BIT_U3_R8(0, G_E)
/* 0xCB44 */ #define gBIT0_H             G_BIT_U3_R8(0, G_H)
/* 0xCB45 */ #define gBIT0_L             G_BIT_U3_R8(0, G_L)
/* 0xCB46 */ #define gBIT0_pHL           G_BIT_U3_HL(0)
/* 0xCB47 */ #define gBIT0_A             G_BIT_U3_R8(0, G_A)
/* 0xCB48 */ #define gBIT1_B             G_BIT_U3_R8(1, G_B)
/* 0xCB49 */ #define gBIT1_C             G_BIT_U3_R8(1, G_C)
/* 0xCB4A */ #define gBIT1_D             G_BIT_U3_R8(1, G_D)
/* 0xCB4B */ #define gBIT1_E             G_BIT_U3_R8(1, G_E)
/* 0xCB4C */ #define gBIT1_H             G_BIT_U3_R8(1, G_H)
/* 0xCB4D */ #define gBIT1_L             G_BIT_U3_R8(1, G_L)
/* 0xCB4E */ #define gBIT1_pHL           G_BIT_U3_HL(1)
/* 0xCB4F */ #define gBIT1_A             G_BIT_U3_R8(1, G_A)

/* 0xCB50 */ #define gBIT2_B             G_BIT_U3_R8(2, G_B)
/* 0xCB51 */ #define gBIT2_C             G_BIT_U3_R8(2, G_C)
/* 0xCB52 */ #define gBIT2_D             G_BIT_U3_R8(2, G_D)
/* 0xCB53 */ #define gBIT2_E             G_BIT_U3_R8(2, G_E)
/* 0xCB54 */ #define gBIT2_H             G_BIT_U3_R8(2, G_H)
/* 0xCB55 */ #define gBIT2_L             G_BIT_U3_R8(2, G_L)
/* 0xCB56 */ #define gBIT2_pHL           G_BIT_U3_HL(2)
/* 0xCB57 */ #define gBIT2_A             G_BIT_U3_R8(2, G_A)
/* 0xCB58 */ #define gBIT3_B             G_BIT_U3_R8(3, G_B)
/* 0xCB59 */ #define gBIT3_C             G_BIT_U3_R8(3, G_C)
/* 0xCB5A */ #define gBIT3_D             G_BIT_U3_R8(3, G_D)
/* 0xCB5B */ #define gBIT3_E             G_BIT_U3_R8(3, G_E)
/* 0xCB5C */ #define gBIT3_H             G_BIT_U3_R8(3, G_H)
/* 0xCB5D */ #define gBIT3_L             G_BIT_U3_R8(3, G_L)
/* 0xCB5E */ #define gBIT3_pHL           G_BIT_U3_HL(3)
/* 0xCB5F */ #define gBIT3_A             G_BIT_U3_R8(3, G_A)

/* 0xCB60 */ #define gBIT4_B             G_BIT_U3_R8(4, G_B)
/* 0xCB61 */ #define gBIT4_C             G_BIT_U3_R8(4, G_C)
/* 0xCB62 */ #define gBIT4_D             G_BIT_U3_R8(4, G_D)
/* 0xCB63 */ #define gBIT4_E             G_BIT_U3_R8(4, G_E)
/* 0xCB64 */ #define gBIT4_H             G_BIT_U3_R8(4, G_H)
/* 0xCB65 */ #define gBIT4_L             G_BIT_U3_R8(4, G_L)
/* 0xCB66 */ #define gBIT4_pHL           G_BIT_U3_HL(4)
/* 0xCB67 */ #define gBIT4_A             G_BIT_U3_R8(4, G_A)
/* 0xCB68 */ #define gBIT5_B             G_BIT_U3_R8(5, G_B)
/* 0xCB69 */ #define gBIT5_C             G_BIT_U3_R8(5, G_C)
/* 0xCB6A */ #define gBIT5_D             G_BIT_U3_R8(5, G_D)
/* 0xCB6B */ #define gBIT5_E             G_BIT_U3_R8(5, G_E)
/* 0xCB6C */ #define gBIT5_H             G_BIT_U3_R8(5, G_H)
/* 0xCB6D */ #define gBIT5_L             G_BIT_U3_R8(5, G_L)
/* 0xCB6E */ #define gBIT5_pHL           G_BIT_U3_HL(5)
/* 0xCB6F */ #define gBIT5_A             G_BIT_U3_R8(5, G_A)

/* 0xCB70 */ #define gBIT6_B             G_BIT_U3_R8(6, G_B)
/* 0xCB71 */ #define gBIT6_C             G_BIT_U3_R8(6, G_C)
/* 0xCB72 */ #define gBIT6_D             G_BIT_U3_R8(6, G_D)
/* 0xCB73 */ #define gBIT6_E             G_BIT_U3_R8(6, G_E)
/* 0xCB74 */ #define gBIT6_H             G_BIT_U3_R8(6, G_H)
/* 0xCB75 */ #define gBIT6_L             G_BIT_U3_R8(6, G_L)
/* 0xCB76 */ #define gBIT6_pHL           G_BIT_U3_HL(6)
/* 0xCB77 */ #define gBIT6_A             G_BIT_U3_R8(6, G_A)
/* 0xCB78 */ #define gBIT7_B             G_BIT_U3_R8(7, G_B)
/* 0xCB79 */ #define gBIT7_C             G_BIT_U3_R8(7, G_C)
/* 0xCB7A */ #define gBIT7_D             G_BIT_U3_R8(7, G_D)
/* 0xCB7B */ #define gBIT7_E             G_BIT_U3_R8(7, G_E)
/* 0xCB7C */ #define gBIT7_H             G_BIT_U3_R8(7, G_H)
/* 0xCB7D */ #define gBIT7_L             G_BIT_U3_R8(7, G_L)
/* 0xCB7E */ #define gBIT7_pHL           G_BIT_U3_HL(7)
/* 0xCB7F */ #define gBIT7_A             G_BIT_U3_R8(7, G_A)    

/* 0xCB80 */ #define gRES0_B             G_RES_U3_R8(0, G_B)
/* 0xCB81 */ #define gRES0_C             G_RES_U3_R8(0, G_C)
/* 0xCB82 */ #define gRES0_D             G_RES_U3_R8(0, G_D)
/* 0xCB83 */ #define gRES0_E             G_RES_U3_R8(0, G_E)
/* 0xCB84 */ #define gRES0_H             G_RES_U3_R8(0, G_H)
/* 0xCB85 */ #define gRES0_L             G_RES_U3_R8(0, G_L)
/* 0xCB86 */ #define gRES0_pHL           G_RES_U3_HL(0)
/* 0xCB87 */ #define gRES0_A             G_RES_U3_R8(0, G_A)
/* 0xCB88 */ #define gRES1_B             G_RES_U3_R8(1, G_B)
/* 0xCB89 */ #define gRES1_C             G_RES_U3_R8(1, G_C)
/* 0xCB8A */ #define gRES1_D             G_RES_U3_R8(1, G_D)
/* 0xCB8B */ #define gRES1_E             G_RES_U3_R8(1, G_E)
/* 0xCB8C */ #define gRES1_H             G_RES_U3_R8(1, G_H)
/* 0xCB8D */ #define gRES1_L             G_RES_U3_R8(1, G_L)
/* 0xCB8E */ #define gRES1_pHL           G_RES_U3_HL(1)
/* 0xCB8F */ #define gRES1_A             G_RES_U3_R8(1, G_A)

/* 0xCB90 */ #define gRES2_B             G_RES_U3_R8(2, G_B)
/* 0xCB91 */ #define gRES2_C             G_RES_U3_R8(2, G_C)
/* 0xCB92 */ #define gRES2_D             G_RES_U3_R8(2, G_D)
/* 0xCB93 */ #define gRES2_E             G_RES_U3_R8(2, G_E)
/* 0xCB94 */ #define gRES2_H             G_RES_U3_R8(2, G_H)
/* 0xCB95 */ #define gRES2_L             G_RES_U3_R8(2, G_L)
/* 0xCB96 */ #define gRES2_pHL           G_RES_U3_HL(2)
/* 0xCB97 */ #define gRES2_A             G_RES_U3_R8(2, G_A)
/* 0xCB98 */ #define gRES3_B             G_RES_U3_R8(3, G_B)
/* 0xCB99 */ #define gRES3_C             G_RES_U3_R8(3, G_C)
/* 0xCB9A */ #define gRES3_D             G_RES_U3_R8(3, G_D)
/* 0xCB9B */ #define gRES3_E             G_RES_U3_R8(3, G_E)
/* 0xCB9C */ #define gRES3_H             G_RES_U3_R8(3, G_H)
/* 0xCB9D */ #define gRES3_L             G_RES_U3_R8(3, G_L)
/* 0xCB9E */ #define gRES3_pHL           G_RES_U3_HL(3)
/* 0xCB9F */ #define gRES3_A             G_RES_U3_R8(3, G_A)

/* 0xCBA0 */ #define gRES4_B             G_RES_U3_R8(4, G_B)
/* 0xCBA1 */ #define gRES4_C             G_RES_U3_R8(4, G_C)
/* 0xCBA2 */ #define gRES4_D             G_RES_U3_R8(4, G_D)
/* 0xCBA3 */ #define gRES4_E             G_RES_U3_R8(4, G_E)
/* 0xCBA4 */ #define gRES4_H             G_RES_U3_R8(4, G_H)
/* 0xCBA5 */ #define gRES4_L             G_RES_U3_R8(4, G_L)
/* 0xCBA6 */ #define gRES4_pHL           G_RES_U3_HL(4)
/* 0xCBA7 */ #define gRES4_A             G_RES_U3_R8(4, G_A)
/* 0xCBA8 */ #define gRES5_B             G_RES_U3_R8(5, G_B)
/* 0xCBA9 */ #define gRES5_C             G_RES_U3_R8(5, G_C)
/* 0xCBAA */ #define gRES5_D             G_RES_U3_R8(5, G_D)
/* 0xCBAB */ #define gRES5_E             G_RES_U3_R8(5, G_E)
/* 0xCBAC */ #define gRES5_H             G_RES_U3_R8(5, G_H)
/* 0xCBAD */ #define gRES5_L             G_RES_U3_R8(5, G_L)
/* 0xCBAE */ #define gRES5_pHL           G_RES_U3_HL(5)
/* 0xCBAF */ #define gRES5_A             G_RES_U3_R8(5, G_A)

/* 0xCBB0 */ #define gRES6_B             G_RES_U3_R8(6, G_B)
/* 0xCBB1 */ #define gRES6_C             G_RES_U3_R8(6, G_C)
/* 0xCBB2 */ #define gRES6_D             G_RES_U3_R8(6, G_D)
/* 0xCBB3 */ #define gRES6_E             G_RES_U3_R8(6, G_E)
/* 0xCBB4 */ #define gRES6_H             G_RES_U3_R8(6, G_H)
/* 0xCBB5 */ #define gRES6_L             G_RES_U3_R8(6, G_L)
/* 0xCBB6 */ #define gRES6_pHL           G_RES_U3_HL(6)
/* 0xCBB7 */ #define gRES6_A             G_RES_U3_R8(6, G_A)
/* 0xCBB8 */ #define gRES7_B             G_RES_U3_R8(7, G_B)
/* 0xCBB9 */ #define gRES7_C             G_RES_U3_R8(7, G_C)
/* 0xCBBA */ #define gRES7_D             G_RES_U3_R8(7, G_D)
/* 0xCBBB */ #define gRES7_E             G_RES_U3_R8(7, G_E)
/* 0xCBBC */ #define gRES7_H             G_RES_U3_R8(7, G_H)
/* 0xCBBD */ #define gRES7_L             G_RES_U3_R8(7, G_L)
/* 0xCBBE */ #define gRES7_pHL           G_RES_U3_HL(7)
/* 0xCBBF */ #define gRES7_A             G_RES_U3_R8(7, G_A)    

/* 0xCBC0 */ #define gSET0_B             G_SET_U3_R8(0, G_B)
/* 0xCBC1 */ #define gSET0_C             G_SET_U3_R8(0, G_C)
/* 0xCBC2 */ #define gSET0_D             G_SET_U3_R8(0, G_D)
/* 0xCBC3 */ #define gSET0_E             G_SET_U3_R8(0, G_E)
/* 0xCBC4 */ #define gSET0_H             G_SET_U3_R8(0, G_H)
/* 0xCBC5 */ #define gSET0_L             G_SET_U3_R8(0, G_L)
/* 0xCBC6 */ #define gSET0_pHL           G_SET_U3_HL(0)
/* 0xCBC7 */ #define gSET0_A             G_SET_U3_R8(0, G_A)
/* 0xCBC8 */ #define gSET1_B             G_SET_U3_R8(1, G_B)
/* 0xCBC9 */ #define gSET1_C             G_SET_U3_R8(1, G_C)
/* 0xCBCA */ #define gSET1_D             G_SET_U3_R8(1, G_D)
/* 0xCBCB */ #define gSET1_E             G_SET_U3_R8(1, G_E)
/* 0xCBCC */ #define gSET1_H             G_SET_U3_R8(1, G_H)
/* 0xCBCD */ #define gSET1_L             G_SET_U3_R8(1, G_L)
/* 0xCBCE */ #define gSET1_pHL           G_SET_U3_HL(1)
/* 0xCBCF */ #define gSET1_A             G_SET_U3_R8(1, G_A)

/* 0xCBD0 */ #define gSET2_B             G_SET_U3_R8(2, G_B)
/* 0xCBD1 */ #define gSET2_C             G_SET_U3_R8(2, G_C)
/* 0xCBD2 */ #define gSET2_D             G_SET_U3_R8(2, G_D)
/* 0xCBD3 */ #define gSET2_E             G_SET_U3_R8(2, G_E)
/* 0xCBD4 */ #define gSET2_H             G_SET_U3_R8(2, G_H)
/* 0xCBD5 */ #define gSET2_L             G_SET_U3_R8(2, G_L)
/* 0xCBD6 */ #define gSET2_pHL           G_SET_U3_HL(2)
/* 0xCBD7 */ #define gSET2_A             G_SET_U3_R8(2, G_A)
/* 0xCBD8 */ #define gSET3_B             G_SET_U3_R8(3, G_B)
/* 0xCBD9 */ #define gSET3_C             G_SET_U3_R8(3, G_C)
/* 0xCBDA */ #define gSET3_D             G_SET_U3_R8(3, G_D)
/* 0xCBDB */ #define gSET3_E             G_SET_U3_R8(3, G_E)
/* 0xCBDC */ #define gSET3_H             G_SET_U3_R8(3, G_H)
/* 0xCBDD */ #define gSET3_L             G_SET_U3_R8(3, G_L)
/* 0xCBDE */ #define gSET3_pHL           G_SET_U3_HL(3)
/* 0xCBDF */ #define gSET3_A             G_SET_U3_R8(3, G_A)

/* 0xCBE0 */ #define gSET4_B             G_SET_U3_R8(4, G_B)
/* 0xCBE1 */ #define gSET4_C             G_SET_U3_R8(4, G_C)
/* 0xCBE2 */ #define gSET4_D             G_SET_U3_R8(4, G_D)
/* 0xCBE3 */ #define gSET4_E             G_SET_U3_R8(4, G_E)
/* 0xCBE4 */ #define gSET4_H             G_SET_U3_R8(4, G_H)
/* 0xCBE5 */ #define gSET4_L             G_SET_U3_R8(4, G_L)
/* 0xCBE6 */ #define gSET4_pHL           G_SET_U3_HL(4)
/* 0xCBE7 */ #define gSET4_A             G_SET_U3_R8(4, G_A)
/* 0xCBE8 */ #define gSET5_B             G_SET_U3_R8(5, G_B)
/* 0xCBE9 */ #define gSET5_C             G_SET_U3_R8(5, G_C)
/* 0xCBEA */ #define gSET5_D             G_SET_U3_R8(5, G_D)
/* 0xCBEB */ #define gSET5_E             G_SET_U3_R8(5, G_E)
/* 0xCBEC */ #define gSET5_H             G_SET_U3_R8(5, G_H)
/* 0xCBED */ #define gSET5_L             G_SET_U3_R8(5, G_L)
/* 0xCBEE */ #define gSET5_pHL           G_SET_U3_HL(5)
/* 0xCBEF */ #define gSET5_A             G_SET_U3_R8(5, G_A)

/* 0xCBF0 */ #define gSET6_B             G_SET_U3_R8(6, G_B)
/* 0xCBF1 */ #define gSET6_C             G_SET_U3_R8(6, G_C)
/* 0xCBF2 */ #define gSET6_D             G_SET_U3_R8(6, G_D)
/* 0xCBF3 */ #define gSET6_E             G_SET_U3_R8(6, G_E)
/* 0xCBF4 */ #define gSET6_H             G_SET_U3_R8(6, G_H)
/* 0xCBF5 */ #define gSET6_L             G_SET_U3_R8(6, G_L)
/* 0xCBF6 */ #define gSET6_pHL           G_SET_U3_HL(6)
/* 0xCBF7 */ #define gSET6_A             G_SET_U3_R8(6, G_A)
/* 0xCBF8 */ #define gSET7_B             G_SET_U3_R8(7, G_B)
/* 0xCBF9 */ #define gSET7_C             G_SET_U3_R8(7, G_C)
/* 0xCBFA */ #define gSET7_D             G_SET_U3_R8(7, G_D)
/* 0xCBFB */ #define gSET7_E             G_SET_U3_R8(7, G_E)
/* 0xCBFC */ #define gSET7_H             G_SET_U3_R8(7, G_H)
/* 0xCBFD */ #define gSET7_L             G_SET_U3_R8(7, G_L)
/* 0xCBFE */ #define gSET7_pHL           G_SET_U3_HL(7)
/* 0xCBFF */ #define gSET7_A             G_SET_U3_R8(7, G_A)    
