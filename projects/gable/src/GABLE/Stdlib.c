/**
 * @file GABLE/Stdlib.c
 */

#include <GABLE/Engine.h>
#include <GABLE/Stdlib.h>
#include <GABLE/Instructions.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void G_WaitVBlank ()
{
    G_WaitVBlank__Loop:
        G_LDH_A_A8(G_LY);
        G_CP_A_N8(G_SCRN_Y);
        G_JR_GOTO(G_COND_NZ, G_WaitVBlank__Loop);
}

void G_WaitAfterVBlank ()
{
    G_WaitAfterVBlank__Loop:
        G_LDH_A_A8(G_LY);
        G_CP_A_N8(G_SCRN_Y);
        G_JR_GOTO(G_COND_Z, G_WaitAfterVBlank__Loop);
}

void G_CopyBytes ()
{
    G_CopyBytes__Loop:
        G_LD_A_RP16(G_DE);
        G_LD_HLI_A();
        G_INC_R16(G_DE);
        G_DEC_R16(G_BC);
        G_LD_R8_R8(G_A, G_B);
        G_OR_A_R8(G_C);
        G_JP_GOTO(G_COND_NZ, G_CopyBytes__Loop);
}

void G_ClearOAM ()
{
    G_XOR_A_R8(G_A);
    G_LD_R8_N8(G_B, 160);

    G_ClearOAM__Loop:
        G_LD_HLI_A();
        G_DEC_R8(G_B);
        G_JR_GOTO(G_COND_NZ, G_ClearOAM__Loop);
}

void G_UpdateKeys (const Uint16 p_CurKeys, const Uint16 p_NewKeys)
{
    // Poll the button half of the controller
    G_LD_R8_N8(G_A, G_P1F_GET_BTN);
    G_LDH_A8_A(G_P1);                          // Switch the key matrix
    G_LDH_A_A8(G_P1);                          // Read the key matrix
    G_OR_A_N8(0xF0);                           // A7-4 = 1; A3-0 = unpressed keys
    G_LD_R8_R8(G_B, G_A);                      // B7-4 = 1; B3-0 = unpressed buttons

    // Poll the direction half of the controller
    G_LD_R8_N8(G_A, G_P1F_GET_DPAD);
    G_LDH_A8_A(G_P1);                          // Switch the key matrix
    G_LDH_A_A8(G_P1);                          // Read the key matrix
    G_OR_A_N8(0xF0);                           // A7-4 = 1; A3-0 = unpressed keys
    G_SWAP_R8(G_A);                            // A7-4 = unpressed directions; A3-0 = 1
    G_XOR_A_R8(G_B);                           // A = pressed buttons + directions
    G_LD_R8_R8(G_B, G_A);                      // B = pressed buttons + directions

    // Release the controller.
    G_LD_R8_N8(G_A, G_P1F_GET_NONE);
    G_LDH_A8_A(G_P1);

    // Combine with previous `p_CurKeys` to make `p_NewKeys`.
    G_LD_A_A16(p_CurKeys);
    G_XOR_A_R8(G_B);                          // A = keys that changed state
    G_AND_A_R8(G_B);                          // A = keys that changed to pressed
    G_LD_A16_A(p_NewKeys);
    G_LD_R8_R8(G_A, G_B);
    G_LD_A16_A(p_CurKeys);
}
