/**
 * @file  GABUILD/Value.h
 * @brief Contains functions for managing runtime values.
 */

#pragma once
#include <GABLE/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define GABUILD_STRING_INITIAL_CAPACITY 80

// Value Type Enumeration //////////////////////////////////////////////////////////////////////////

typedef enum GABUILD_ValueType
{
    GABUILD_VT_VOID,
    GABUILD_VT_NUMBER,
    GABUILD_VT_STRING
} GABUILD_ValueType;

// Value Structure /////////////////////////////////////////////////////////////////////////////////

typedef struct GABUILD_Value
{
    GABUILD_ValueType m_Type;
    union
    {
        struct
        {
            Float64 m_Number;
            Uint64 m_IntegerPart;
            Uint64 m_FractionalPart;
        };

        Char* m_String;
    };
} GABUILD_Value;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABUILD_Value* GABUILD_CreateVoidValue ();
GABUILD_Value* GABUILD_CreateNumberValue (Float64 p_Number);
GABUILD_Value* GABUILD_CreateStringValue (const Char* p_String);
GABUILD_Value* GABUILD_CopyValue (const GABUILD_Value* p_Value);
void GABUILD_DestroyValue (GABUILD_Value* p_Value);
void GABUILD_PrintValue (const GABUILD_Value* p_Value);
void GABUILD_SetNumberValue (GABUILD_Value* p_Value, Float64 p_Number);
void GABUILD_SetStringValue (GABUILD_Value* p_Value, const Char* p_String);
GABUILD_Value* GABUILD_ConcatenateStringValues (const GABUILD_Value* p_LeftValue, const GABUILD_Value* p_RightValue);
