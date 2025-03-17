/**
 * @file     GABUILD/Keyword.c
 */

#include <GABUILD/Keyword.h>

// Keyword Lookup Table ////////////////////////////////////////////////////////////////////////////

static const GABUILD_Keyword GABUILD_KEYWORD_TABLE[] = {
    { "BYTE",       GABUILD_KT_BYTE,       0 },
    { "DB",         GABUILD_KT_BYTE,       0 },
    { "WORD",       GABUILD_KT_WORD,       0 },
    { "DW",         GABUILD_KT_WORD,       0 },
    { "DWORD",      GABUILD_KT_DWORD,      0 },
    { "DD",         GABUILD_KT_DWORD,      0 },
    { "QWORD",      GABUILD_KT_QWORD,      0 },
    { "DQ",         GABUILD_KT_QWORD,      0 },
    { "FLOAT",      GABUILD_KT_FLOAT,      0 },
    { "DOUBLE",     GABUILD_KT_DOUBLE,     0 },
    { "STRING",     GABUILD_KT_STRING,     0 },
    { "LET",        GABUILD_KT_LET,        0 },
    { "CONST",      GABUILD_KT_CONST,      0 },
    { "IF",         GABUILD_KT_IF,         0 },
    { "ELSE",       GABUILD_KT_ELSE,       0 },
    { "FOR",        GABUILD_KT_FOR,        0 },
    { "WHILE",      GABUILD_KT_WHILE,      0 },
    { "DO",         GABUILD_KT_DO,         0 },
    { "REPEAT",     GABUILD_KT_REPEAT,     0 },
    { "BREAK",      GABUILD_KT_BREAK,      0 },
    { "CONTINUE",   GABUILD_KT_CONTINUE,   0 },
    { "FUNCTION",   GABUILD_KT_FUNCTION,   0 },
    { "SHIFT",      GABUILD_KT_SHIFT,      0 },
    { "RETURN",     GABUILD_KT_RETURN,     0 },
    { "ASM",        GABUILD_KT_ASM,        0 },
    { "INCBIN",     GABUILD_KT_INCBIN,     0 },
    { "INCLUDE",    GABUILD_KT_INCLUDE,    0 },

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
        case GABUILD_KT_BYTE:       return "BYTE";
        case GABUILD_KT_WORD:       return "WORD";
        case GABUILD_KT_DWORD:      return "DWORD";
        case GABUILD_KT_QWORD:      return "QWORD";
        case GABUILD_KT_FLOAT:      return "FLOAT";
        case GABUILD_KT_DOUBLE:     return "DOUBLE";
        case GABUILD_KT_STRING:     return "STRING";
        case GABUILD_KT_LET:        return "LET";
        case GABUILD_KT_CONST:      return "CONST";
        case GABUILD_KT_IF:         return "IF";
        case GABUILD_KT_ELSE:       return "ELSE";
        case GABUILD_KT_FOR:        return "FOR";
        case GABUILD_KT_WHILE:      return "WHILE";
        case GABUILD_KT_REPEAT:     return "REPEAT";
        case GABUILD_KT_BREAK:      return "BREAK";
        case GABUILD_KT_CONTINUE:   return "CONTINUE";
        case GABUILD_KT_FUNCTION:   return "FUNCTION";
        case GABUILD_KT_SHIFT:      return "SHIFT";
        case GABUILD_KT_RETURN:     return "RETURN";
        default:                    return "NONE";
    }
}
