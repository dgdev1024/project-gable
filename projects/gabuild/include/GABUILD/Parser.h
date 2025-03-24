/**
 * @file  GABUILD/Parser.h
 * @brief Contains functions for parsing tokens to produce syntax nodes.
 */

#pragma once
#include <GABUILD/Syntax.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void GABUILD_InitParser ();
void GABUILD_ShutdownParser ();
Bool GABUILD_Parse (GABUILD_Syntax* p_SyntaxBlock);
const GABUILD_Syntax* GABUILD_GetRootSyntax ();
