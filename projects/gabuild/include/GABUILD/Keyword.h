/**
 * @file     GABUILD/Keyword.h
 * @brief    Contains functions for looking up reserved keywords.
 */

#pragma once

#include <GABLE/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define GABUILD_KEYWORD_STRLEN 16

// Keyword Type Enumeration ////////////////////////////////////////////////////////////////////////

typedef enum GABUILD_KeywordType
{
    GABUILD_KT_NONE = 0,
    
    // Data Statements
    GABUILD_KT_BYTE,            ///< @brief Define Byte Data
    GABUILD_KT_WORD,            ///< @brief Define Word Data
    GABUILD_KT_DWORD,           ///< @brief Define Double Word Data
    GABUILD_KT_QWORD,           ///< @brief Define Quad Word Data
    GABUILD_KT_FLOAT,           ///< @brief Define Single Precision Floating Point Data
    GABUILD_KT_DOUBLE,          ///< @brief Define Double Precision Floating Point Data
    GABUILD_KT_STRING,          ///< @brief Define String Data

    // Variable Statements
    GABUILD_KT_LET,             ///< @brief Variable Assignment
    GABUILD_KT_CONST,           ///< @brief Constant Declaration

    // Control Statements
    GABUILD_KT_IF,              ///< @brief Conditional Statement (If)
    GABUILD_KT_ELSE,            ///< @brief Conditional Statement (Else)
    GABUILD_KT_FOR,             ///< @brief Loop Statement (For)
    GABUILD_KT_WHILE,           ///< @brief Loop Statement (While)
    GABUILD_KT_DO,              ///< @brief Loop Statement (Do-While)
    GABUILD_KT_REPEAT,          ///< @brief Loop Statement (Repeat)
    GABUILD_KT_BREAK,           ///< @brief Loop Control Statement (Break)
    GABUILD_KT_CONTINUE,        ///< @brief Loop Control Statement (Continue)
    GABUILD_KT_FUNCTION,        ///< @brief Function Definition
    GABUILD_KT_SHIFT,           ///< @brief Function Argument Control Statement (Shift)
    GABUILD_KT_RETURN,          ///< @brief Function Return Statement
    GABUILD_KT_ASM,             ///< @brief Assembly Block
    GABUILD_KT_INCBIN,          ///< @brief Include Binary File
    GABUILD_KT_INCLUDE,         ///< @brief Include Source File

} GABUILD_KeywordType;

// Keyword Structure ///////////////////////////////////////////////////////////////////////////////

typedef struct GABUILD_Keyword
{
    Char                        m_Name[GABUILD_KEYWORD_STRLEN]; ///< @brief Keyword Name
    GABUILD_KeywordType         m_Type;                         ///< @brief Keyword Type
    Int32                       m_Param;                        ///< @brief Optional Parameter
} GABUILD_Keyword;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

const GABUILD_Keyword* GABUILD_LookupKeyword (const Char* p_Name);
const Char* GABUILD_StringifyKeywordType (GABUILD_KeywordType p_Type);
