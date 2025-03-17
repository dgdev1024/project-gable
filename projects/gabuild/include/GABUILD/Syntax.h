/**
 * @file     GABUILD/Syntax.h
 * @brief    Contains structures and functions for parsing tokens into an abstract syntax tree.
 */

#pragma once
#include <GABUILD/Token.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define GABUILD_LITERAL_STRLEN 256

// Syntax Type Enumeration /////////////////////////////////////////////////////////////////////////

typedef enum GABUILD_SyntaxType
{
    GABUILD_SYNTAX_UNKNOWN = 0,

    // Statements
    GABUILD_SYNTAX_BLOCK,                   // Syntax Block (A list of statements enclosed in braces)
    GABUILD_SYNTAX_INCLUDE,                 // Include Statement (eg. `include "file.gab"`)
    GABUILD_SYNTAX_INCBIN,                  // Include Binary Statement (eg. `incbin "file.bin"`)
    GABUILD_SYNTAX_ASM,                     // Assembly Statement (eg. `asm { ... }`)
    GABUILD_SYNTAX_DATA,                    // Data Placement (eg. `db 0x52, 0x53, 0x54`, `word 16384`, `string "Hello, World!"`; requires an `asm` statement)
    GABUILD_SYNTAX_LABEL,                   // Label Definition (eg. `loop:`)
    GABUILD_SYNTAX_LET,                     // Variable Declaration (eg. `let x = 10`)
    GABUILD_SYNTAX_CONST,                   // Constant Declaration (eg. `const PI = 3.14159`)
    GABUILD_SYNTAX_IF,                      // If Statement (eg. `if (x == 10) {}`)
    GABUILD_SYNTAX_ELSE,                    // Else Statement (eg. `else {}`; requires an `if` statement)
    GABUILD_SYNTAX_FOR,                     // For Loop (eg. `for (let i = 0; i < 10; i++) {}`)
    GABUILD_SYNTAX_WHILE,                   // While Loop (eg. `while (x < 10) {}`)
    GABUILD_SYNTAX_DO,                      // Do-While Loop (eg. `do {} while (x < 10)`)
    GABUILD_SYNTAX_REPEAT,                  // Repeat Loop (eg. `repeat (10) {}`)
    GABUILD_SYNTAX_FUNCTION,                // Function Declaration (eg. `function main () {}`)
    GABUILD_SYNTAX_SHIFT,                   // Shift Statement (eg. `shift 1`; requires a `function` statement)
    GABUILD_SYNTAX_RETURN,                  // Return Statement (eg. `return 0`)
    GABUILD_SYNTAX_BREAK,                   // Break Statement (eg. `break`)
    GABUILD_SYNTAX_CONTINUE,                // Continue Statement (eg. `continue`)

    // Expressions
    GABUILD_SYNTAX_BINARY,                  // Binary Expression (eg. `x + 10`, `(x + 10) * 2`)
    GABUILD_SYNTAX_UNARY,                   // Unary Expression (eg. `-x`, `!x`)
    GABUILD_SYNTAX_TERNARY,                 // Ternary Expression (eg. `x == 10 ? 1 : 0`)
    GABUILD_SYNTAX_ASSIGNMENT,              // Assignment Expression (eg. `x = 10`)
    GABUILD_SYNTAX_CALL,                    // Function Call (eg. `main()`, `printf("Hello, World!")`)
    GABUILD_SYNTAX_INDEX,                   // Array Indexing (eg. `array[0]`, `matrix[1][2]`)
    GABUILD_SYNTAX_MEMBER,                  // Structure Member Access (eg. `point.x`, `point.y`)
    GABUILD_SYNTAX_IDENTIFIER,              // Identifier (eg. `x`, `y`)
    GABUILD_SYNTAX_STRING,                  // String Literal (eg. `"Hello, World!"`)
    GABUILD_SYNTAX_NUMBER,                  // Number Literal (eg. `10`, `3.14159`)
    GABUILD_SYNTAX_BINARY_NUMBER,           // Binary Number Literal (eg. `0b1010`)
    GABUILD_SYNTAX_OCTAL_NUMBER,            // Octal Number Literal (eg. `0o755`)
    GABUILD_SYNTAX_HEXADECIMAL_NUMBER,      // Hexadecimal Number Literal (eg. `0x7FFF`)
    GABUILD_SYNTAX_CHARACTER,               // Character Literal (eg. `'A'`, `'\n'`)
    GABUILD_SYNTAX_ARRAY,                   // Array Literal (eg. `[1, 2, 3]`)
    GABUILD_SYNTAX_STRUCTURE,               // Structure Literal (eg. `{ x: 10, y: 20 }`)
} GABUILD_SyntaxType;

// Syntax Structure ////////////////////////////////////////////////////////////////////////////////

typedef struct GABUILD_Syntax
{
    GABUILD_SyntaxType           m_Type;             ///< @brief Syntax Type
    const GABUILD_Token*         m_LeadToken;        ///< @brief The Syntax's Leading Token

    // Some Syntax Nodes are literals, and may have either a string, number, or character value:
    // - `GABUILD_SYNTAX_STRING` contains a string literal.
    // - `GABUILD_SYNTAX_NUMBER` contains a number literal.
    // - `GABUILD_SYNTAX_BINARY_NUMBER` contains a binary number literal.
    // - `GABUILD_SYNTAX_OCTAL_NUMBER` contains an octal number literal.
    // - `GABUILD_SYNTAX_HEXADECIMAL_NUMBER` contains a hexadecimal number literal.
    // - `GABUILD_SYNTAX_CHARACTER` contains a character literal.
    Char*                        m_String;           ///< @brief String Literal
    Float64                      m_Number;           ///< @brief Number Literal

    // Some Syntax Nodes may have a body of statements or expressions:
    // - `GABUILD_SYNTAX_BLOCK` contains a list of statements.
    // - `GABUILD_SYNTAX_ASM` contains a list of data placement (and instruction, if desired) statements.
    // - `GABUILD_SYNTAX_FUNCTION` contains a list of statements.
    struct GABUILD_Syntax**      m_Body;             ///< @brief Syntax Body (Statements or Expressions)
    Size                         m_BodySize;         ///< @brief Number of Syntax Nodes in the Body
    Size                         m_BodyCapacity;     ///< @brief Capacity of the Body

    // Some Syntax Nodes may have arguments:
    // - `GABUILD_SYNTAX_FUNCTION` contains a list of parameters.
    // - `GABUILD_SYNTAX_CALL` contains a list of arguments.
    struct GABUILD_Syntax**      m_Arguments;        ///< @brief Function Arguments
    Size                         m_ArgumentSize;     ///< @brief Number of Arguments
    Size                         m_ArgumentCapacity; ///< @brief Capacity of the Arguments

    // `GABUILD_SYNTAX_FUNCTION` may have a return value:
    struct GABUILD_Syntax*       m_ReturnValue;      ///< @brief Function Return Value

    // Some Syntax Nodes may have a condition, and/or one or two branches:
    // - `GABUILD_SYNTAX_IF` contains a condition and two branches (if and else).
    // - `GABUILD_SYNTAX_TERANRY` contains a condition and two branches (true and false).
    struct GABUILD_Syntax*       m_Condition;        ///< @brief Condition Expression
    struct GABUILD_Syntax*       m_TrueBranch;       ///< @brief True Branch (if, ternary `?`)
    struct GABUILD_Syntax*       m_FalseBranch;      ///< @brief False Branch (else, ternary `:`)

    // Some Syntax Nodes are loops and may have an initialization, condition, and/or increment, in
    // addition to a branch:
    // - `GABUILD_SYNTAX_FOR` contains an initialization, condition, increment, and a branch.
    // - `GABUILD_SYNTAX_WHILE` contains a condition and a branch.
    // - `GABUILD_SYNTAX_DO` contains a branch and a condition.
    // - `GABUILD_SYNTAX_REPEAT` contains a count and a branch.
    struct GABUILD_Syntax*       m_Initialization;   ///< @brief Loop Initialization (eg. `for (let i = 0;...)`)
    struct GABUILD_Syntax*       m_Increment;        ///< @brief Loop Increment (eg. `for (...; ...; i++)`)
    struct GABUILD_Syntax*       m_Branch;           ///< @brief Loop Branch (eg. `for (...; ...; ...) {}`)
    struct GABUILD_Syntax*       m_Count;            ///< @brief Loop Count (eg. `repeat (10) {}`)

    // Some Syntax Nodes may have a variable or constant declaration:
    // - `GABUILD_SYNTAX_LET` contains a variable declaration.
    // - `GABUILD_SYNTAX_CONST` contains a constant declaration.
    struct GABUILD_Syntax*       m_VariableValue;    ///< @brief Variable or Constant Value
    Bool                         m_IsConstant;       ///< @brief Is Constant Declaration

    // Some Syntax Nodes may have a lefthand and righthand side, and maybe an operator:
    // - `GABUILD_SYNTAX_BINARY` contains a lefthand and righthand side, and an operator.
    // - `GABUILD_SYNTAX_UNARY` contains a lefthand or righthand side (but not both) and an operator.
    // - `GABUILD_SYNTAX_ASSIGNMENT` contains a lefthand and righthand side, and an operator.
    // - `GABUILD_SYNTAX_INDEX` contains a lefthand (array) and righthand (index) side.
    // - `GABUILD_SYNTAX_MEMBER` contains a lefthand (structure) and righthand (member) side.
    struct GABUILD_Syntax*       m_Left;             ///< @brief Lefthand Side
    struct GABUILD_Syntax*       m_Right;            ///< @brief Righthand Side
    GABUILD_TokenType            m_Operator;         ///< @brief Operator Token
    
    // Function Call Syntax Nodes have a callee syntax:
    // - `GABUILD_SYNTAX_CALL` contains a callee syntax.
    struct GABUILD_Syntax*       m_Callee;           ///< @brief Function Call Callee

} GABUILD_Syntax;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABUILD_Syntax* GABUILD_CreateSyntax (GABUILD_SyntaxType p_Type, const GABUILD_Token* p_Token);
void GABUILD_DestroySyntax (GABUILD_Syntax* p_Syntax);
void GABUILD_ResizeSyntax (GABUILD_Syntax* p_Syntax);
void GABUILD_PushToBody (GABUILD_Syntax* p_Syntax, GABUILD_Syntax* p_Node);
void GABUILD_PushToArguments (GABUILD_Syntax* p_Syntax, GABUILD_Syntax* p_Argument);
