/**
 * @file     GABUILD/Keyword.c
 */

#include <GABUILD/Keyword.h>

// Keyword Lookup Table ////////////////////////////////////////////////////////////////////////////

static const GABUILD_Keyword GABUILD_KEYWORD_TABLE[] = {
    { "DB",         GABUILD_KT_DB,         0 },
    { "DW",         GABUILD_KT_DW,         0 },
    { "DL",         GABUILD_KT_DL,         0 },
    { "DS",         GABUILD_KT_DS,         0 },
    { "INCLUDE",    GABUILD_KT_INCLUDE,    0 },
    { "INCBIN",     GABUILD_KT_INCBIN,     0 },
    { "DEF",        GABUILD_KT_DEF,        0 },
    { "MACRO",      GABUILD_KT_MACRO,      0 },
    { "ENDM",       GABUILD_KT_ENDM,       0 },
    { "SHIFT",      GABUILD_KT_SHIFT,      0 },
    { "REPEAT",     GABUILD_KT_REPEAT,     0 },
    { "REPT",       GABUILD_KT_REPEAT,     0 },
    { "FOR",        GABUILD_KT_FOR,        0 },
    { "IF",         GABUILD_KT_IF,         0 },
    { "ELIF",       GABUILD_KT_ELIF,       0 },
    { "ELSE",       GABUILD_KT_ELSE,       0 },
    { "ENDR",       GABUILD_KT_ENDR,       0 },
    { "ENDC",       GABUILD_KT_ENDC,       0 },
    { "",           GABUILD_KT_NONE,       0 }
};

// Public Functions ////////////////////////////////////////////////////////////////////////////////

const GABUILD_Keyword* GABUILD_LookupKeyword (const Char* p_Name)
{
    for (Int32 i = 0; ; ++i)
    {
        if (
            GABUILD_KEYWORD_TABLE[i].m_Type == GABUILD_KT_NONE ||
            strncmp(GABUILD_KEYWORD_TABLE[i].m_Name, p_Name, GABUILD_KEYWORD_STRLEN) == 0
        )
        {
            return &GABUILD_KEYWORD_TABLE[i];
        }
    }
}

const Char* GABUILD_StringifyKeywordType (GABUILD_KeywordType p_Type)
{
    switch (p_Type)
    {
        default:                    return "NONE";
    }
}
