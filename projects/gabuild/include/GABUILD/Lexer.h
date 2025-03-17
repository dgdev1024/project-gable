/**
 * @file     GABUILD/Lexer.h
 * @brief    Contains functions for extracting tokens from a source file.
 */

#pragma once
#include <GABUILD/Token.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void GABUILD_InitLexer ();
void GABUILD_ShutdownLexer ();
Bool GABUILD_LexFile (const Char* p_FilePath);
Bool GABUILD_HasMoreTokens ();
const GABUILD_Token* GABUILD_AdvanceToken ();
const GABUILD_Token* GABUILD_AdvanceTokenIfType (GABUILD_TokenType p_Type);
const GABUILD_Token* GABUILD_AdvanceTokenIfKeyword (GABUILD_KeywordType p_Type);
const GABUILD_Token* GABUILD_PeekToken (Index p_Offset);
void GABUILD_PrintTokens ();
void GABUILD_ResetLexer ();
