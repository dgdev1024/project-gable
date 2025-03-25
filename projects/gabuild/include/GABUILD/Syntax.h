/**
 * @file     GABUILD/Syntax.h
 * @brief    Contains enumerations and structures for syntax nodes extracted from the lexer.
 */

#pragma once
#include <GABUILD/Token.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define GABUILD_SYNTAX_BODY_INITIAL_CAPACITY 8
#define GABUILD_STRING_CAPACITY 80

// Syntax Type Enumeration /////////////////////////////////////////////////////////////////////////

typedef enum GABUILD_SyntaxType
{
    GABUILD_ST_BLOCK,                   ///< @brief A block of syntax nodes, including the root node.

    // Statement Nodes
    GABUILD_ST_LABEL,                   ///< @brief Label Statement (eg. `label:`).
    GABUILD_ST_DATA,                    ///< @brief Data Statement (eg. `db 0x00`, `db "Hello, World!"`, `dw 0x0000`).
    GABUILD_ST_DEF,                     ///< @brief Define Statement (eg. `def x = 0x00`).
    GABUILD_ST_MACRO,                   ///< @brief Macro Statement (eg. `macro x ... endm`).
    GABUILD_ST_MACRO_CALL,              ///< @brief Macro Call Statement (eg. `x`, `place_byte $42`).
    GABUILD_ST_SHIFT,                   ///< @brief Shift Statement (eg. `shift 2`).
    GABUILD_ST_REPEAT,                  ///< @brief Repeat Statement (eg. `repeat 3`, `rept 5`).
    GABUILD_ST_IF,                      ///< @brief If Statement (eg. `if x == 0`).
    GABUILD_ST_INCLUDE,                 ///< @brief Include Statement (eg. `include "file.asm"`).
    GABUILD_ST_INCBIN,                  ///< @brief Include Binary Statement (eg. `incbin "file.bin"`).

    // Expression Nodes
    GABUILD_ST_BINARY_EXP,              ///< @brief Binary Expression (eg. `1 + 2`, `3 * 4`).
    GABUILD_ST_UNARY_EXP,               ///< @brief Unary Expression (eg. `-1`, `~2`).
    GABUILD_ST_NARG,                    ///< @brief Number of Arguments Expression (eg. `_narg`).
    GABUILD_ST_IDENTIFIER,              ///< @brief Identifier (eg. `x`, `y`).
    GABUILD_ST_NUMBER,                  ///< @brief Number (eg. `0`, `1`, `2`).
    GABUILD_ST_ARGUMENT,                ///< @brief Argument Placeholder (eg. `@0`, `@1`).
    GABUILD_ST_STRING,                  ///< @brief String (eg. `"Hello, World!"`).

} GABUILD_SyntaxType;

// Syntax Node Structure ///////////////////////////////////////////////////////////////////////////

typedef struct GABUILD_Syntax
{
    GABUILD_SyntaxType           m_Type;         ///< @brief Syntax Node Type
    GABUILD_Token                m_Token;        ///< @brief Token Associated with the Syntax Node

    // Syntax Node Specific Data ///////////////////////////////////////////////////////////////////

    // Some nodes keep a string of text.
    // - `GABUILD_ST_LABEL` nodes have a string of text to hold the label name.
    // - `GABUILD_ST_DEF` nodes have a string of text to hold the variable name.
    // - `GABUILD_ST_MACRO` nodes have a string of text to hold the macro name.
    // - `GABUILD_ST_MACRO_CALL` nodes have a string of text to hold the macro name.
    // - `GABUILD_ST_IDENTIFIER` nodes have a string of text to hold the symbol name.
    // - `GABUILD_ST_STRING` nodes have a string of text to hold the string.
    // - `GABUILD_ST_NUMBER` nodes have a string of text to hold the number in string form.
    Char*                        m_String;       ///< @brief String of Text

    // Some nodes hold a number.
    // - `GABUILD_ST_MACRO_CALL` nodes have a number value to hold the argument count.
    // - `GABUILD_ST_ARGUMENT` nodes have a number value to hold the argument index.
    // - `GABUILD_ST_NARG` nodes have a number value to hold the number of arguments passed to a macro.
    // - `GABUILD_ST_NUMBER` nodes have a number value.
    Float64                      m_Number;       ///< @brief Number Value

    // Some nodes may need to keep track of the keyword type of its lead token.
    // - `GABUILD_ST_DATA` nodes have a keyword type to hold the data type.
    GABUILD_KeywordType          m_KeywordType;  ///< @brief Keyword Type

    // Some nodes have a body of child nodes, such as a block of statements, or a macro definition.
    // - `GABUILD_ST_BLOCK` nodes have a body of child nodes.
    // - `GABUILD_ST_DATA` nodes have a body of child nodes to hold parameters passed to the data statement.
    // - `GABUILD_ST_MACRO_CALL` nodes have a body of child nodes to hold arguments passed to the macro.
    struct GABUILD_Syntax**      m_Body;         ///< @brief Array of Child Syntax Nodes
    Size                         m_BodySize;     ///< @brief Number of Child Syntax Nodes
    Size                         m_BodyCapacity; ///< @brief Capacity of Child Syntax Nodes

    // Some nodes keep track of a count.
    // - `GABUILD_ST_DATA` nodes have a count of the number of parameters.
    // - `GABUILD_ST_SHIFT` nodes have a count of the number of arguments to shift.
    // - `GABUILD_ST_REPEAT` nodes have a count of the number of times to repeat the block.
    struct GABUILD_Syntax*       m_CountExpr;    ///< @brief Count Expression

    // Some nodes have a conditional expression.
    // - `GABUILD_ST_IF` nodes have a conditional expression.
    struct GABUILD_Syntax*       m_CondExpr;     ///< @brief Conditional Expression

    // Some nodes are unary and binary expression nodes, with left and/or right child nodes 
    // and an operator token.
    // - `GABUILD_ST_MACRO` nodes contain their body in the left child node.
    // - `GABUILD_ST_REPEAT` nodes contain their body in the left child node.
    // - `GABUILD_ST_IF` nodes contain their true block in the left child node and their false block in the right child node.
    // - `GABUILD_ST_BINARY_EXP` nodes have a left and right child node.
    // - `GABUILD_ST_UNARY_EXP` nodes have a right child node.
    struct GABUILD_Syntax*       m_LeftExpr;     ///< @brief Left Expression
    struct GABUILD_Syntax*       m_RightExpr;    ///< @brief Right Expression
    GABUILD_TokenType            m_Operator;     ///< @brief Operator Token Type

} GABUILD_Syntax;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABUILD_Syntax* GABUILD_CreateSyntax (GABUILD_SyntaxType p_Type, const GABUILD_Token* p_Token);
GABUILD_Syntax* GABUILD_CopySyntax (const GABUILD_Syntax* p_Syntax);
void GABUILD_DestroySyntax (GABUILD_Syntax* p_Syntax);
void GABUILD_PushToSyntaxBody (GABUILD_Syntax* p_Parent, GABUILD_Syntax* p_Child);
