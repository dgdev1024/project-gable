/**
 * @file      GABUILD/Token.c
 */

#include <GABUILD/Token.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

const Char* GABUILD_StringifyTokenType (GABUILD_TokenType p_Type)
{
    switch (p_Type)
    {
        // Keywords and Identifiers
        case GABUILD_TOKEN_KEYWORD: return "Keyword";
        case GABUILD_TOKEN_IDENTIFIER: return "Identifier";

        // String, Number, and Character Literals
        case GABUILD_TOKEN_STRING: return "String";
        case GABUILD_TOKEN_NUMBER: return "Number";
        case GABUILD_TOKEN_BINARY: return "Binary";
        case GABUILD_TOKEN_OCTAL: return "Octal";
        case GABUILD_TOKEN_HEXADECIMAL: return "Hexadecimal";
        case GABUILD_TOKEN_CHARACTER: return "Character";

        // Arithmetic Operators
        case GABUILD_TOKEN_PLUS: return "Plus";
        case GABUILD_TOKEN_MINUS: return "Minus";
        case GABUILD_TOKEN_MULTIPLY: return "Multiply";
        case GABUILD_TOKEN_EXPONENT: return "Exponent";
        case GABUILD_TOKEN_DIVIDE: return "Divide";
        case GABUILD_TOKEN_MODULO: return "Modulo";
        case GABUILD_TOKEN_INCREMENT: return "Increment";
        case GABUILD_TOKEN_DECREMENT: return "Decrement";

        // Bitwise Operators
        case GABUILD_TOKEN_BITWISE_AND: return "Bitwise And";
        case GABUILD_TOKEN_BITWISE_OR: return "Bitwise Or";
        case GABUILD_TOKEN_BITWISE_XOR: return "Bitwise Xor";
        case GABUILD_TOKEN_BITWISE_NOT: return "Bitwise Not";
        case GABUILD_TOKEN_BITWISE_SHIFT_LEFT: return "Bitwise Shift Left";
        case GABUILD_TOKEN_BITWISE_SHIFT_RIGHT: return "Bitwise Shift Right";

        // Comparison Operators
        case GABUILD_TOKEN_COMPARE_EQUAL: return "Compare Equal";
        case GABUILD_TOKEN_COMPARE_NOT_EQUAL: return "Compare Not Equal";
        case GABUILD_TOKEN_COMPARE_LESS: return "Compare Less";
        case GABUILD_TOKEN_COMPARE_LESS_EQUAL: return "Compare Less Equal";
        case GABUILD_TOKEN_COMPARE_GREATER: return "Compare Greater";
        case GABUILD_TOKEN_COMPARE_GREATER_EQUAL: return "Compare Greater Equal";

        // Logical Operators
        case GABUILD_TOKEN_LOGICAL_AND: return "Logical And";
        case GABUILD_TOKEN_LOGICAL_OR: return "Logical Or";
        case GABUILD_TOKEN_LOGICAL_NOT: return "Logical Not";

        // Assignment Operators
        case GABUILD_TOKEN_ASSIGN_EQUAL: return "Assign Equal";
        case GABUILD_TOKEN_ASSIGN_PLUS: return "Assign Plus";
        case GABUILD_TOKEN_ASSIGN_MINUS: return "Assign Minus";
        case GABUILD_TOKEN_ASSIGN_MULTIPLY: return "Assign Multiply";
        case GABUILD_TOKEN_ASSIGN_EXPONENT: return "Assign Exponent";
        case GABUILD_TOKEN_ASSIGN_DIVIDE: return "Assign Divide";
        case GABUILD_TOKEN_ASSIGN_MODULO: return "Assign Modulo";
        case GABUILD_TOKEN_ASSIGN_BITWISE_AND: return "Assign Bitwise And";
        case GABUILD_TOKEN_ASSIGN_BITWISE_OR: return "Assign Bitwise Or";
        case GABUILD_TOKEN_ASSIGN_BITWISE_XOR: return "Assign Bitwise Xor";
        case GABUILD_TOKEN_ASSIGN_BITWISE_SHIFT_LEFT: return "Assign Bitwise Shift Left";
        case GABUILD_TOKEN_ASSIGN_BITWISE_SHIFT_RIGHT: return "Assign Bitwise Shift Right";

        // Grouping Operators
        case GABUILD_TOKEN_PARENTHESIS_OPEN: return "Parenthesis Open";
        case GABUILD_TOKEN_PARENTHESIS_CLOSE: return "Parenthesis Close";
        case GABUILD_TOKEN_BRACKET_OPEN: return "Bracket Open";
        case GABUILD_TOKEN_BRACKET_CLOSE: return "Bracket Close";
        case GABUILD_TOKEN_BRACE_OPEN: return "Brace Open";
        case GABUILD_TOKEN_BRACE_CLOSE: return "Brace Close";

        // Punctuation
        case GABUILD_TOKEN_COMMA: return "Comma";
        case GABUILD_TOKEN_SEMICOLON: return "Semicolon";
        case GABUILD_TOKEN_COLON: return "Colon";
        case GABUILD_TOKEN_PERIOD: return "Period";
        case GABUILD_TOKEN_QUESTION: return "Question Mark";

        case GABUILD_TOKEN_EOF: return "End of File";

        default: return "Unknown";
    }
}

const Char* GABUILD_StringifyToken (const GABUILD_Token* p_Token)
{
    return (p_Token != NULL) ? GABUILD_StringifyTokenType(p_Token->m_Type) : "Null";
}

void GABUILD_PrintToken (const GABUILD_Token* p_Token)
{
    printf("  Token '%s'", GABUILD_StringifyToken(p_Token));
    if (p_Token != NULL && p_Token->m_Lexeme[0] != '\0')
    {
        printf(" = '%s'", p_Token->m_Lexeme);
    }
    printf("\n");
}
