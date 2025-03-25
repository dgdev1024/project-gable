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
    GABUILD_KT_NONE = 0,                ///< @brief No keyword type.
    GABUILD_KT_DB,                      ///< @brief "Define Byte" keyword (eg. `db 0x00`).
    GABUILD_KT_DW,                      ///< @brief "Define Word" keyword (eg. `dw 0x0000`).
    GABUILD_KT_DL,                      ///< @brief "Define Long" keyword (eg. `dl 0x00000000`).
    GABUILD_KT_DS,                      ///< @brief "Define Sequence" keyword (eg. `ds 3, 0x00, 0x11`, `ds 5, 0x44`).
    GABUILD_KT_INCLUDE,                 ///< @brief "Include" keyword (eg. `include "file.asm"`).
    GABUILD_KT_INCBIN,                  ///< @brief "Include Binary" keyword (eg. `incbin "file.bin"`).
    GABUILD_KT_DEF,                     ///< @brief "Define" keyword (eg. `def x = 0x00`).
    GABUILD_KT_MACRO,                   ///< @brief "Macro" keyword (eg. `macro name`).
    GABUILD_KT_ENDM,                    ///< @brief "End Macro" keyword (eg. `endm`).
    GABUILD_KT_NARG,                    ///< @brief "Number of Arguments" keyword (eg. `narg`).
    GABUILD_KT_SHIFT,                   ///< @brief "Shift" keyword (eg. `shift 2`).
    GABUILD_KT_REPEAT,                  ///< @brief "Repeat" keyword (eg. `repeat 3`, `rept 5`).
    GABUILD_KT_FOR,                     ///< @brief "For" keyword (eg. `for n, 256`, `for x, 0, 20, 4`).
    GABUILD_KT_IF,                      ///< @brief "If" keyword (eg. `if n == 0`).
    GABUILD_KT_ELIF,                    ///< @brief "Else If" keyword (eg. `elif n == 1`).
    GABUILD_KT_ELSE,                    ///< @brief "Else" keyword (eg. `else`).
    GABUILD_KT_ENDR,                    ///< @brief "End Repeat", "End For" keyword (eg. `endr`).
    GABUILD_KT_ENDC,                    ///< @brief "End If" keyword (eg. `endc`).
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
