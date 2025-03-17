/**
 * @file     GABUILD/Arguments.h
 * @brief    Provides functions used for processing and parsing command line arguments.
 */

#pragma once

#include <GABLE/Common.h>

void GABUILD_CaptureArguments (Int32 p_Argc, Char** p_Argv);
void GABUILD_ReleaseArguments ();
Bool GABUILD_HasArgument (const Char* p_Longform, const Char p_Shortform);
const Char* GABUILD_GetArgumentValue (const Char* p_Longform, const Char p_Shortform);