/**
 * @file     GABUILD/Token.h
 * @brief    Contains the token structure and the token type enumeration.
 */

#pragma once
#include <GABUILD/Keyword.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define GABUILD_TOKEN_MAX_LENGTH 80

// Token Type Enumeration //////////////////////////////////////////////////////////////////////////

typedef enum GABUILD_TokenType
{
    GABUILD_TOKEN_UNKNOWN = 0,

    // Keywords and Identifiers
    GABUILD_TOKEN_KEYWORD,
    GABUILD_TOKEN_IDENTIFIER,

    // String, Number, and Character Literals
    GABUILD_TOKEN_STRING,
    GABUILD_TOKEN_NUMBER,
    GABUILD_TOKEN_BINARY,
    GABUILD_TOKEN_OCTAL,
    GABUILD_TOKEN_HEXADECIMAL,
    GABUILD_TOKEN_CHARACTER,
    GABUILD_TOKEN_ARGUMENT,

    // Arithmetic Operators
    GABUILD_TOKEN_PLUS,
    GABUILD_TOKEN_MINUS,
    GABUILD_TOKEN_MULTIPLY,
    GABUILD_TOKEN_EXPONENT,
    GABUILD_TOKEN_DIVIDE,
    GABUILD_TOKEN_MODULO,
    GABUILD_TOKEN_INCREMENT,
    GABUILD_TOKEN_DECREMENT,

    // Bitwise Operators
    GABUILD_TOKEN_BITWISE_AND,
    GABUILD_TOKEN_BITWISE_OR,
    GABUILD_TOKEN_BITWISE_XOR,
    GABUILD_TOKEN_BITWISE_NOT,
    GABUILD_TOKEN_BITWISE_SHIFT_LEFT,
    GABUILD_TOKEN_BITWISE_SHIFT_RIGHT,

    // Comparison Operators
    GABUILD_TOKEN_COMPARE_EQUAL,
    GABUILD_TOKEN_COMPARE_NOT_EQUAL,
    GABUILD_TOKEN_COMPARE_LESS,
    GABUILD_TOKEN_COMPARE_LESS_EQUAL,
    GABUILD_TOKEN_COMPARE_GREATER,
    GABUILD_TOKEN_COMPARE_GREATER_EQUAL,

    // Logical Operators
    GABUILD_TOKEN_LOGICAL_AND,
    GABUILD_TOKEN_LOGICAL_OR,
    GABUILD_TOKEN_LOGICAL_NOT,

    // Assignment Operators
    GABUILD_TOKEN_ASSIGN_EQUAL,
    GABUILD_TOKEN_ASSIGN_PLUS,
    GABUILD_TOKEN_ASSIGN_MINUS,
    GABUILD_TOKEN_ASSIGN_MULTIPLY,
    GABUILD_TOKEN_ASSIGN_EXPONENT,
    GABUILD_TOKEN_ASSIGN_DIVIDE,
    GABUILD_TOKEN_ASSIGN_MODULO,
    GABUILD_TOKEN_ASSIGN_BITWISE_AND,
    GABUILD_TOKEN_ASSIGN_BITWISE_OR,
    GABUILD_TOKEN_ASSIGN_BITWISE_XOR,
    GABUILD_TOKEN_ASSIGN_BITWISE_SHIFT_LEFT,
    GABUILD_TOKEN_ASSIGN_BITWISE_SHIFT_RIGHT,

    // Grouping Operators
    GABUILD_TOKEN_PARENTHESIS_OPEN,
    GABUILD_TOKEN_PARENTHESIS_CLOSE,
    GABUILD_TOKEN_BRACKET_OPEN,
    GABUILD_TOKEN_BRACKET_CLOSE,
    GABUILD_TOKEN_BRACE_OPEN,
    GABUILD_TOKEN_BRACE_CLOSE,

    // Punctuation
    GABUILD_TOKEN_COMMA,
    GABUILD_TOKEN_COLON,
    GABUILD_TOKEN_PERIOD,
    GABUILD_TOKEN_QUESTION,
    GABUILD_TOKEN_POUND,

    GABUILD_TOKEN_NEWLINE,
    GABUILD_TOKEN_EOF
} GABUILD_TokenType;

// Token Structure /////////////////////////////////////////////////////////////////////////////////

typedef struct GABUILD_Token
{
    Char*                       m_Lexeme;
    GABUILD_TokenType           m_Type;
    const GABUILD_Keyword*      m_Keyword;
    const Char*                 m_SourceFile;
    Index                       m_Line;
    Index                       m_Column;
} GABUILD_Token;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

const Char* GABUILD_StringifyTokenType (GABUILD_TokenType p_Type);
const Char* GABUILD_StringifyToken (const GABUILD_Token* p_Token);
void GABUILD_PrintToken (const GABUILD_Token* p_Token);
Bool GABUILD_IsUnaryOperator (GABUILD_TokenType p_Type);
Bool GABUILD_IsMultiplicativeOperator (GABUILD_TokenType p_Type);
Bool GABUILD_IsAdditiveOperator (GABUILD_TokenType p_Type);
Bool GABUILD_IsShiftOperator (GABUILD_TokenType p_Type);
Bool GABUILD_IsComparisonOperator (GABUILD_TokenType p_Type);
Bool GABUILD_IsAssignmentOperator (GABUILD_TokenType p_Type);
