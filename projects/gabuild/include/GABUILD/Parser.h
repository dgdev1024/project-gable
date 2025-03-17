/**
 * @file     GABUILD/Parser.h
 * @brief    Contains functions for parsing tokens into a syntax tree.
 */

#pragma once
#include <GABUILD/Syntax.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void GABUILD_InitParser ();
void GABUILD_ShutdownParser ();
Bool GABUILD_ParseTokens (GABUILD_Syntax* p_Syntax);
