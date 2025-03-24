/**
 * @file  GABUILD/Builder.h
 * @brief Contains functions for building the output file from the syntax tree.
 */

#pragma once
#include <GABUILD/Syntax.h>
#include <GABUILD/Value.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void GABUILD_InitBuilder ();
void GABUILD_ShutdownBuilder ();
Bool GABUILD_Build (const GABUILD_Syntax* p_SyntaxNode);
Bool GABUILD_SaveBinary (const char* p_OutputPath);
