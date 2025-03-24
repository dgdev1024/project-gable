/**
 * @file  GABUILD/Parser.c
 */

#include <GABUILD/Lexer.h>
#include <GABUILD/Parser.h>

// Parser Context Structure ////////////////////////////////////////////////////////////////////////

static struct
{
    GABUILD_Syntax*             m_RootBlock;
    const GABUILD_Token*        m_LeadToken;
} s_Parser = {
    .m_RootBlock = NULL,
    .m_LeadToken = NULL
};

// Static Function Prototypes //////////////////////////////////////////////////////////////////////

static GABUILD_Syntax* GABUILD_ParsePrimaryExpression ();
static GABUILD_Syntax* GABUILD_ParseExpression ();
static GABUILD_Syntax* GABUILD_ParseStatement ();

// Static Functions - Expression Parsing ///////////////////////////////////////////////////////////

// Most assembly languages have a simple syntax for expressions, which are composed of an order
// of operator precedence. The order of precedence is as follows:
//
// 1. Primary Expressions and Parentheses (highest)
// 2. Unary Operators
// 3. Exponentiation
// 4. Multiplication and Division
// 5. Addition and Subtraction
// 6. Bitwise Shifts
// 7. Bitwise AND
// 8. Bitwise XOR
// 9. Bitwise OR
// 10. Comparison Operators
// 11. Logical AND
// 12. Logical OR (lowest)

// 1. Primary Expressions and Parentheses
//
// The primary expression is the most basic form of an expression, and can be a number, string,
// identifier, or a parenthesized expression. Parentheses are used to group expressions together,
// and are evaluated first.
//
// The syntax of a primary expression is as follows:
//
//    `10`                              // Number
//    `4.1`                             // Number (floating-point)
//    `0b1010`                          // Binary Number
//    `0o777`                           // Octal Number
//    `0xFF`                            // Hexadecimal Number
//    `@3`                              // Macro Argument Placeholder
//    `'a'`, `'\n'`                     // Characters (single quotes, can be escaped)
//    `"Hello, World!"`                 // String
//    `label`, `x`                      // Identifiers
//    `(10 + 20)`                       // Parenthesized Expression
//
GABUILD_Syntax* GABUILD_ParsePrimaryExpression ()
{
    // Store and consume the lead token of the expression.
    const GABUILD_Token* l_LeadToken = GABUILD_AdvanceToken();

    // Check the type of the token.
    switch (l_LeadToken->m_Type)
    {
        case GABUILD_TOKEN_NUMBER:
        {
            GABUILD_Syntax* l_NumberSyntax = GABUILD_CreateSyntax(GABUILD_ST_NUMBER, l_LeadToken);
            l_NumberSyntax->m_Number = strtod(l_LeadToken->m_Lexeme, NULL);
            return l_NumberSyntax;
        }

        case GABUILD_TOKEN_BINARY:
        {
            GABUILD_Syntax* l_BinarySyntax = GABUILD_CreateSyntax(GABUILD_ST_NUMBER, l_LeadToken);
            l_BinarySyntax->m_Number = (Float64) strtoul(l_LeadToken->m_Lexeme, NULL, 2);
            return l_BinarySyntax;
        }
        

        case GABUILD_TOKEN_OCTAL:
        {
            GABUILD_Syntax* l_OctalSyntax = GABUILD_CreateSyntax(GABUILD_ST_NUMBER, l_LeadToken);
            l_OctalSyntax->m_Number = (Float64) strtoul(l_LeadToken->m_Lexeme, NULL, 8);
            return l_OctalSyntax;
        }

        case GABUILD_TOKEN_HEXADECIMAL:
        {
            GABUILD_Syntax* l_HexadecimalSyntax = GABUILD_CreateSyntax(GABUILD_ST_NUMBER, l_LeadToken);
            l_HexadecimalSyntax->m_Number = (Float64) strtoul(l_LeadToken->m_Lexeme, NULL, 16);
            return l_HexadecimalSyntax;
        }

        case GABUILD_TOKEN_CHARACTER:
        {
            GABUILD_Syntax* l_CharacterSyntax = GABUILD_CreateSyntax(GABUILD_ST_NUMBER, l_LeadToken);
            l_CharacterSyntax->m_Number = (Float64) (Uint64) l_LeadToken->m_Lexeme[0];
            return l_CharacterSyntax;
        }

        case GABUILD_TOKEN_STRING:
        {
            GABUILD_Syntax* l_StringSyntax = GABUILD_CreateSyntax(GABUILD_ST_STRING, l_LeadToken);
            strncpy(l_StringSyntax->m_String, l_LeadToken->m_Lexeme, GABUILD_STRING_CAPACITY);
            return l_StringSyntax;
        }

        case GABUILD_TOKEN_IDENTIFIER:
        {
            GABUILD_Syntax* l_IdentifierSyntax = GABUILD_CreateSyntax(GABUILD_ST_IDENTIFIER, l_LeadToken);
            strncpy(l_IdentifierSyntax->m_String, l_LeadToken->m_Lexeme, GABUILD_STRING_CAPACITY);
            return l_IdentifierSyntax;
        }

        case GABUILD_TOKEN_PARENTHESIS_OPEN:
        {
            GABUILD_Syntax* l_ExpressionSyntax = GABUILD_ParseExpression();
            if (l_ExpressionSyntax == NULL)
            {
                return NULL;
            }

            if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_PARENTHESIS_CLOSE) == NULL)
            {
                GABLE_error("Expected a closing parenthesis after an expression.");
                GABUILD_DestroySyntax(l_ExpressionSyntax);
                return NULL;
            }

            return l_ExpressionSyntax;
        }

        default:
            GABLE_error("Unexpected '%s' token = '%s'.", 
                GABUILD_StringifyTokenType(l_LeadToken->m_Type), l_LeadToken->m_Lexeme);
            return NULL;
    }
}

// 2. Unary Operators
//
// Unary operators are operators that operate on a single operand. The unary operators in the
// language are the logical NOT operator `!`, the bitwise NOT operator `~`, and the unary minus
// operator `-`.
//
// The syntax of a unary expression is as follows:
//
//    `+4`                              // Unary Plus
//    `-10`                             // Unary Minus
//    `!flag`                           // Logical NOT
//    `~0xFF`                           // Bitwise NOT
//
GABUILD_Syntax* GABUILD_ParseUnaryExpression ()
{
    // Peek the next token. Is it a unary operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (GABUILD_IsUnaryOperator(l_Token->m_Type) == true)
    {
        
        // Store and consume the unary operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();
        
        // Create the unary syntax node and set the operator type.
        GABUILD_Syntax* l_UnaryExpr = GABUILD_CreateSyntax(GABUILD_ST_UNARY_EXP, l_OperatorToken);
        l_UnaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the operand of the unary expression.
        GABUILD_Syntax* l_OperandExpr = GABUILD_ParseUnaryExpression();
        if (l_OperandExpr == NULL)
        {
            GABUILD_DestroySyntax(l_UnaryExpr);
            return NULL;
        }

        // Set the righthand side of the unary expression to the operand.
        l_UnaryExpr->m_RightExpr = l_OperandExpr;
        return l_UnaryExpr;
    }

    return GABUILD_ParsePrimaryExpression();
}

// 3. Exponentiation
//
// Exponentiation is a binary operation which raises a left-hand operand to the power of a
// right-hand operand. The exponentiation operator in the language is the caret `**`.
//
// The syntax of an exponentiation expression is as follows:
//
//    `2 ** 3`                          // 2 raised to the power of 3
//
GABUILD_Syntax* GABUILD_ParseExponentiationExpression ()
{
    // Parse the left-hand side of the expression.
    GABUILD_Syntax* l_LeftExpr = GABUILD_ParseUnaryExpression();
    if (l_LeftExpr == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it an exponentiation operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (l_Token->m_Type == GABUILD_TOKEN_EXPONENT)
    {
        // Store and consume the exponentiation operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();

        // Create the binary syntax node and set the operator type.
        GABUILD_Syntax* l_BinaryExpr = GABUILD_CreateSyntax(GABUILD_ST_BINARY_EXP, l_OperatorToken);
        l_BinaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the right-hand side of the expression.
        GABUILD_Syntax* l_RightExpr = GABUILD_ParseExponentiationExpression();
        if (l_RightExpr == NULL)
        {
            GABUILD_DestroySyntax(l_BinaryExpr);
            return NULL;
        }

        // Set the lefthand and righthand sides of the binary expression.
        l_BinaryExpr->m_LeftExpr = l_LeftExpr;
        l_BinaryExpr->m_RightExpr = l_RightExpr;
        return l_BinaryExpr;
    }

    return l_LeftExpr;
}

// 4. Multiplication, Division, and Modulo (Multiplicative Operators)
//
// Multiplication, division, and modulo are binary operations that multiply, divide, and find the
// remainder of the division of two operands, respectively. The multiplicative operators in the
// language are the asterisk `*` for multiplication, the forward slash `/` for division, and the
// percent sign `%` for modulo.
//
// The syntax of a multiplicative expression is as follows:
//
//    `10 * 20`                         // 10 multiplied by 20
//    `10 / 20`                         // 10 divided by 20
//    `10 % 20`                         // 10 modulo 20 (remainder of 10 divided by 20)
//
GABUILD_Syntax* GABUILD_ParseMultiplicativeExpression ()
{
    // Parse the left-hand side of the expression.
    GABUILD_Syntax* l_LeftExpr = GABUILD_ParseExponentiationExpression();
    if (l_LeftExpr == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a multiplicative operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (GABUILD_IsMultiplicativeOperator(l_Token->m_Type) == true)
    {
        // Store and consume the multiplicative operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();

        // Create the binary syntax node and set the operator type.
        GABUILD_Syntax* l_BinaryExpr = GABUILD_CreateSyntax(GABUILD_ST_BINARY_EXP, l_OperatorToken);
        l_BinaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the right-hand side of the expression.
        GABUILD_Syntax* l_RightExpr = GABUILD_ParseMultiplicativeExpression();
        if (l_RightExpr == NULL)
        {
            GABUILD_DestroySyntax(l_BinaryExpr);
            return NULL;
        }

        // Set the lefthand and righthand sides of the binary expression.
        l_BinaryExpr->m_LeftExpr = l_LeftExpr;
        l_BinaryExpr->m_RightExpr = l_RightExpr;
        return l_BinaryExpr;
    }

    return l_LeftExpr;
}

// 5. Addition and Subtraction (Additive Operators)
//
// Addition and subtraction are binary operations that add and subtract two operands, respectively.
// The additive operators in the language are the plus `+` and minus `-` operators.
//
// The syntax of an additive expression is as follows:
//
//    `10 + 20`                         // 10 added to 20
//    `10 - 20`                         // 10 subtracted by 20
//
GABUILD_Syntax* GABUILD_ParseAdditiveExpression ()
{
    // Parse the left-hand side of the expression.
    GABUILD_Syntax* l_LeftExpr = GABUILD_ParseMultiplicativeExpression();
    if (l_LeftExpr == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it an additive operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (GABUILD_IsAdditiveOperator(l_Token->m_Type) == true)
    {
        // Store and consume the additive operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();

        // Create the binary syntax node and set the operator type.
        GABUILD_Syntax* l_BinaryExpr = GABUILD_CreateSyntax(GABUILD_ST_BINARY_EXP, l_OperatorToken);
        l_BinaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the right-hand side of the expression.
        GABUILD_Syntax* l_RightExpr = GABUILD_ParseAdditiveExpression();
        if (l_RightExpr == NULL)
        {
            GABUILD_DestroySyntax(l_BinaryExpr);
            return NULL;
        }

        // Set the lefthand and righthand sides of the binary expression.
        l_BinaryExpr->m_LeftExpr = l_LeftExpr;
        l_BinaryExpr->m_RightExpr = l_RightExpr;
        return l_BinaryExpr;
    }

    return l_LeftExpr;
}

// 6. Bitwise Shifts
//
// Bitwise shifts are binary operations that shift the bits of a number to the left or right by a
// specified number of positions. The bitwise shift operators in the language are the left shift `<<`
// and right shift `>>` operators.
//
// The syntax of a bitwise shift expression is as follows:
//
//    `10 << 2`                         // 10 shifted left by 2 bits
//    `10 >> 2`                         // 10 shifted right by 2 bits
//
GABUILD_Syntax* GABUILD_ParseBitwiseShiftExpression ()
{
    // Parse the left-hand side of the expression.
    GABUILD_Syntax* l_LeftExpr = GABUILD_ParseAdditiveExpression();
    if (l_LeftExpr == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a bitwise shift operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (GABUILD_IsShiftOperator(l_Token->m_Type) == true)
    {
        // Store and consume the bitwise shift operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();

        // Create the binary syntax node and set the operator type.
        GABUILD_Syntax* l_BinaryExpr = GABUILD_CreateSyntax(GABUILD_ST_BINARY_EXP, l_OperatorToken);
        l_BinaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the right-hand side of the expression.
        GABUILD_Syntax* l_RightExpr = GABUILD_ParseBitwiseShiftExpression();
        if (l_RightExpr == NULL)
        {
            GABUILD_DestroySyntax(l_BinaryExpr);
            return NULL;
        }

        // Set the lefthand and righthand sides of the binary expression.
        l_BinaryExpr->m_LeftExpr = l_LeftExpr;
        l_BinaryExpr->m_RightExpr = l_RightExpr;
        return l_BinaryExpr;
    }

    return l_LeftExpr;
}

// 7. Bitwise AND
//
// Bitwise AND is a binary operation that performs a bitwise AND operation on two numbers. The
// bitwise AND operator in the language is the ampersand `&` operator.
//
// The syntax of a bitwise AND expression is as follows:
//
//    `10 & 20`                         // 10 bitwise AND 20
//
GABUILD_Syntax* GABUILD_ParseBitwiseAndExpression ()
{
    // Parse the left-hand side of the expression.
    GABUILD_Syntax* l_LeftExpr = GABUILD_ParseBitwiseShiftExpression();
    if (l_LeftExpr == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a bitwise AND operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (l_Token->m_Type == GABUILD_TOKEN_BITWISE_AND)
    {
        // Store and consume the bitwise AND operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();

        // Create the binary syntax node and set the operator type.
        GABUILD_Syntax* l_BinaryExpr = GABUILD_CreateSyntax(GABUILD_ST_BINARY_EXP, l_OperatorToken);
        l_BinaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the right-hand side of the expression.
        GABUILD_Syntax* l_RightExpr = GABUILD_ParseBitwiseAndExpression();
        if (l_RightExpr == NULL)
        {
            GABUILD_DestroySyntax(l_BinaryExpr);
            return NULL;
        }

        // Set the lefthand and righthand sides of the binary expression.
        l_BinaryExpr->m_LeftExpr = l_LeftExpr;
        l_BinaryExpr->m_RightExpr = l_RightExpr;
        return l_BinaryExpr;
    }

    return l_LeftExpr;
}

// 8. Bitwise XOR
//
// Bitwise XOR is a binary operation that performs a bitwise XOR operation on two numbers. The
// bitwise XOR operator in the language is the caret `^` operator.
//
// The syntax of a bitwise XOR expression is as follows:
//
//    `10 ^ 20`                         // 10 bitwise XOR 20
//
GABUILD_Syntax* GABUILD_ParseBitwiseXorExpression ()
{
    // Parse the left-hand side of the expression.
    GABUILD_Syntax* l_LeftExpr = GABUILD_ParseBitwiseAndExpression();
    if (l_LeftExpr == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a bitwise XOR operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (l_Token->m_Type == GABUILD_TOKEN_BITWISE_XOR)
    {
        // Store and consume the bitwise XOR operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();

        // Create the binary syntax node and set the operator type.
        GABUILD_Syntax* l_BinaryExpr = GABUILD_CreateSyntax(GABUILD_ST_BINARY_EXP, l_OperatorToken);
        l_BinaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the right-hand side of the expression.
        GABUILD_Syntax* l_RightExpr = GABUILD_ParseBitwiseXorExpression();
        if (l_RightExpr == NULL)
        {
            GABUILD_DestroySyntax(l_BinaryExpr);
            return NULL;
        }

        // Set the lefthand and righthand sides of the binary expression.
        l_BinaryExpr->m_LeftExpr = l_LeftExpr;
        l_BinaryExpr->m_RightExpr = l_RightExpr;
        return l_BinaryExpr;
    }

    return l_LeftExpr;
}

// 9. Bitwise OR
//
// Bitwise OR is a binary operation that performs a bitwise OR operation on two numbers. The
// bitwise OR operator in the language is the pipe `|` operator.
//
// The syntax of a bitwise OR expression is as follows:
//
//    `10 | 20`                         // 10 bitwise OR 20
//
GABUILD_Syntax* GABUILD_ParseBitwiseOrExpression ()
{
    // Parse the left-hand side of the expression.
    GABUILD_Syntax* l_LeftExpr = GABUILD_ParseBitwiseXorExpression();
    if (l_LeftExpr == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a bitwise OR operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (l_Token->m_Type == GABUILD_TOKEN_BITWISE_OR)
    {
        // Store and consume the bitwise OR operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();

        // Create the binary syntax node and set the operator type.
        GABUILD_Syntax* l_BinaryExpr = GABUILD_CreateSyntax(GABUILD_ST_BINARY_EXP, l_OperatorToken);
        l_BinaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the right-hand side of the expression.
        GABUILD_Syntax* l_RightExpr = GABUILD_ParseBitwiseOrExpression();
        if (l_RightExpr == NULL)
        {
            GABUILD_DestroySyntax(l_BinaryExpr);
            return NULL;
        }

        // Set the lefthand and righthand sides of the binary expression.
        l_BinaryExpr->m_LeftExpr = l_LeftExpr;
        l_BinaryExpr->m_RightExpr = l_RightExpr;
        return l_BinaryExpr;
    }

    return l_LeftExpr;
}

// 10. Comparison Operators
//
// Comparison operators are binary operations that compare two operands and return a boolean value.
// The comparison operators in the language are the equality `==`, inequality `!=`, less than `<`,
// less than or equal `<=`, greater than `>`, and greater than or equal `>=` operators.
//
// The syntax of a comparison expression is as follows:
//
//    `10 == 20`                        // 10 is equal to 20
//    `10 != 20`                        // 10 is not equal to 20
//    `10 < 20`                         // 10 is less than 20
//    `10 <= 20`                        // 10 is less than or equal to 20
//    `10 > 20`                         // 10 is greater than 20
//    `10 >= 20`                        // 10 is greater than or equal to 20
//
GABUILD_Syntax* GABUILD_ParseComparisonExpression ()
{
    // Parse the left-hand side of the expression.
    GABUILD_Syntax* l_LeftExpr = GABUILD_ParseBitwiseOrExpression();
    if (l_LeftExpr == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a comparison operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (GABUILD_IsComparisonOperator(l_Token->m_Type) == true)
    {
        // Store and consume the comparison operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();

        // Create the binary syntax node and set the operator type.
        GABUILD_Syntax* l_BinaryExpr = GABUILD_CreateSyntax(GABUILD_ST_BINARY_EXP, l_OperatorToken);
        l_BinaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the right-hand side of the expression.
        GABUILD_Syntax* l_RightExpr = GABUILD_ParseComparisonExpression();
        if (l_RightExpr == NULL)
        {
            GABUILD_DestroySyntax(l_BinaryExpr);
            return NULL;
        }

        // Set the lefthand and righthand sides of the binary expression.
        l_BinaryExpr->m_LeftExpr = l_LeftExpr;
        l_BinaryExpr->m_RightExpr = l_RightExpr;
        return l_BinaryExpr;
    }

    return l_LeftExpr;
}

// 11. Logical AND
//
// Logical AND is a binary operation that performs a logical AND operation on two boolean operands.
// The logical AND operator in the language is the double ampersand `&&` operator.
//
// The syntax of a logical AND expression is as follows:
//
//    `flag1 && flag2`                  // flag1 AND flag2
//
GABUILD_Syntax* GABUILD_ParseLogicalAndExpression ()
{
    // Parse the left-hand side of the expression.
    GABUILD_Syntax* l_LeftExpr = GABUILD_ParseComparisonExpression();
    if (l_LeftExpr == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a logical AND operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (l_Token->m_Type == GABUILD_TOKEN_LOGICAL_AND)
    {
        // Store and consume the logical AND operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();

        // Create the binary syntax node and set the operator type.
        GABUILD_Syntax* l_BinaryExpr = GABUILD_CreateSyntax(GABUILD_ST_BINARY_EXP, l_OperatorToken);
        l_BinaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the right-hand side of the expression.
        GABUILD_Syntax* l_RightExpr = GABUILD_ParseLogicalAndExpression();
        if (l_RightExpr == NULL)
        {
            GABUILD_DestroySyntax(l_BinaryExpr);
            return NULL;
        }

        // Set the lefthand and righthand sides of the binary expression.
        l_BinaryExpr->m_LeftExpr = l_LeftExpr;
        l_BinaryExpr->m_RightExpr = l_RightExpr;
        return l_BinaryExpr;
    }

    return l_LeftExpr;
}

// 12. Logical OR
//
// Logical OR is a binary operation that performs a logical OR operation on two boolean operands.
// The logical OR operator in the language is the double pipe `||` operator. This is the lowest
// precedence operator in the language.
//
// The syntax of a logical OR expression is as follows:
//
//    `flag1 || flag2`                  // flag1 OR flag2
//
GABUILD_Syntax* GABUILD_ParseLogicalOrExpression ()
{
    // Parse the left-hand side of the expression.
    GABUILD_Syntax* l_LeftExpr = GABUILD_ParseLogicalAndExpression();
    if (l_LeftExpr == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a logical OR operator?
    const GABUILD_Token* l_Token = GABUILD_PeekToken(0);
    if (l_Token->m_Type == GABUILD_TOKEN_LOGICAL_OR)
    {
        // Store and consume the logical OR operator token.
        const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();

        // Create the binary syntax node and set the operator type.
        GABUILD_Syntax* l_BinaryExpr = GABUILD_CreateSyntax(GABUILD_ST_BINARY_EXP, l_OperatorToken);
        l_BinaryExpr->m_Operator = l_OperatorToken->m_Type;

        // Parse the right-hand side of the expression.
        GABUILD_Syntax* l_RightExpr = GABUILD_ParseLogicalOrExpression();
        if (l_RightExpr == NULL)
        {
            GABUILD_DestroySyntax(l_BinaryExpr);
            return NULL;
        }

        // Set the lefthand and righthand sides of the binary expression.
        l_BinaryExpr->m_LeftExpr = l_LeftExpr;
        l_BinaryExpr->m_RightExpr = l_RightExpr;
        return l_BinaryExpr;
    }

    return l_LeftExpr;
}

GABUILD_Syntax* GABUILD_ParseExpression ()
{
    return GABUILD_ParseLogicalOrExpression();
}

// Static Functions - Statement Parsing ////////////////////////////////////////////////////////////

// Label Statement
//
// A label statement is a statement that is used to define an location in the code that can be
// jumped to or otherwise referenced by data and instructions.
//
// The syntax of a label statement is as follows:
//
//    `<identifier>:`
//
GABUILD_Syntax* GABUILD_ParseLabelSyntax ()
{
    // Store the identifier token, then advance past it and the following colon.
    const GABUILD_Token* l_IdentifierToken = GABUILD_AdvanceToken();
    GABUILD_AdvanceToken();

    // Create the label syntax node.
    GABUILD_Syntax* l_LabelSyntax = GABUILD_CreateSyntax(GABUILD_ST_LABEL, l_IdentifierToken);

    // Copy the identifier token's lexeme into the label syntax node's string.
    strncpy(l_LabelSyntax->m_String, l_IdentifierToken->m_Lexeme, GABUILD_STRING_CAPACITY);

    return l_LabelSyntax;
}

// Data Statement
//
// A data statement is a statement that is used to define a sequence of bytes or words in the output
// file. The data statement is used to define values, constants and strings in the output file.
//
// The syntax of a data statement is as follows:
//
//    `db <expression> [, <expression> ...]`                ; Define one or more bytes (8-bit values), ASCII characters or strings.
//    `dw <expression> [, <expression> ...]`                ; Define one or more words (16-bit values).
//    `dl <expression> [, <expression> ...]`                ; Define one or more longs (32-bit values).
//    `ds <count>, <expression> [, <expression> ...]`       ; Define a sequence of bytes with a count.
//
GABUILD_Syntax* GABUILD_ParseDataSyntax (const GABUILD_Keyword* p_Keyword)
{
    // Create the data syntax node.
    GABUILD_Syntax* l_DataSyntax = GABUILD_CreateSyntax(GABUILD_ST_DATA, s_Parser.m_LeadToken);

    // Set the keyword type of the data syntax node.
    l_DataSyntax->m_KeywordType = p_Keyword->m_Type;

    // If a `ds` statement is being parsed, then this statement requires a count expression,
    // followed by one or more data expressions.
    if (p_Keyword->m_Type == GABUILD_KT_DS)
    {
        // Parse the count expression.
        GABUILD_Syntax* l_CountExpr = GABUILD_ParseExpression();
        if (l_CountExpr == NULL)
        {
            GABUILD_DestroySyntax(l_DataSyntax);
            return NULL;
        }

        // Set the count expression of the node to what was parsed.
        l_DataSyntax->m_CountExpr = l_CountExpr;

        // Expect a comma after the count expression.
        if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_COMMA) == NULL)
        {
            GABLE_error("Expected a comma after the count expression in a 'ds' statement.");
            GABUILD_DestroySyntax(l_DataSyntax);
            return NULL;
        }

        // Parse the data expressions thereafter.
        while (true)
        {
            // Parse the next expression.
            GABUILD_Syntax* l_DataExpr = GABUILD_ParseExpression();
            if (l_DataExpr == NULL)
            {
                GABUILD_DestroySyntax(l_DataSyntax);
                return NULL;
            }

            // Add the data expression to the data syntax node.
            GABUILD_PushToSyntaxBody(l_DataSyntax, l_DataExpr);

            // If there is no comma after the data expression, then break the loop.
            if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_COMMA) == NULL)
            {
                break;
            }
        }
    }

    // If a `db`, `dw`, or `dl` statement is being parsed, then this statement requires one or more
    // data expressions.
    else
    {
        // Parse the data expressions.
        while (true)
        {
            // Parse the next expression.
            GABUILD_Syntax* l_DataExpr = GABUILD_ParseExpression();
            if (l_DataExpr == NULL)
            {
                GABUILD_DestroySyntax(l_DataSyntax);
                return NULL;
            }

            // Add the data expression to the data syntax node.
            GABUILD_PushToSyntaxBody(l_DataSyntax, l_DataExpr);

            // If there is no comma after the data expression, then break the loop.
            if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_COMMA) == NULL)
            {
                break;
            }
        }
    }

    return l_DataSyntax;
}

// Define Statement
//
// A define statement is a statement that is used to define a variable or constant value which can
// be referenced by other statements and expressions in the code.
//
// The syntax of a define statement is as follows:
//
//     `def <identifier> <operator> <expression>`            ; Define a variable or constant value.
//
GABUILD_Syntax* GABUILD_ParseDefineSyntax ()
{
    // Store the identifier token, then advance past it.
    const GABUILD_Token* l_IdentifierToken = GABUILD_AdvanceToken();

    // The next token should be an operator token - an assignment operator.
    const GABUILD_Token* l_OperatorToken = GABUILD_AdvanceToken();
    if (GABUILD_IsAssignmentOperator(l_OperatorToken->m_Type) == false)
    {
        GABLE_error("Expected an assignment operator after an identifier in a 'def' statement.");
        return NULL;
    }

    // Parse the expression.
    GABUILD_Syntax* l_Expression = GABUILD_ParseExpression();
    if (l_Expression == NULL)
    {
        return NULL;
    }

    // Create the define syntax node.
    GABUILD_Syntax* l_DefineSyntax = GABUILD_CreateSyntax(GABUILD_ST_DEF, l_IdentifierToken);
    strncpy(l_DefineSyntax->m_String, l_IdentifierToken->m_Lexeme, GABUILD_STRING_CAPACITY);
    l_DefineSyntax->m_Operator = l_OperatorToken->m_Type;
    l_DefineSyntax->m_RightExpr = l_Expression;

    return l_DefineSyntax;
}

GABUILD_Syntax* GABUILD_ParseStatement ()
{
    // Skip any newline tokens.
    while (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_NEWLINE) != NULL) {}

    // If the lead token is an identifier, and the token immediately following it is a colon, then
    // a label statement is being parsed.
    if (
        GABUILD_PeekToken(0)->m_Type == GABUILD_TOKEN_IDENTIFIER &&
        GABUILD_PeekToken(1)->m_Type == GABUILD_TOKEN_COLON
    )
    {
        return GABUILD_ParseLabelSyntax();
    }

    // If the lead token is a keyword, then check what keyword it is.
    if (GABUILD_PeekToken(0)->m_Type == GABUILD_TOKEN_KEYWORD)
    {
        // Get the keyword token.
        const GABUILD_Token* l_KeywordToken = GABUILD_AdvanceToken();

        // If the keyword token is a data keyword, then parse a data statement.
        if (
            l_KeywordToken->m_Keyword->m_Type == GABUILD_KT_DB ||
            l_KeywordToken->m_Keyword->m_Type == GABUILD_KT_DW ||
            l_KeywordToken->m_Keyword->m_Type == GABUILD_KT_DL ||
            l_KeywordToken->m_Keyword->m_Type == GABUILD_KT_DS
        )
        {
            return GABUILD_ParseDataSyntax(l_KeywordToken->m_Keyword);
        }

        // If the keyword token is a define keyword, then parse a define statement.
        else if (l_KeywordToken->m_Keyword->m_Type == GABUILD_KT_DEF)
        {
            return GABUILD_ParseDefineSyntax();
        }

        else
        {
            GABLE_error("Unexpected keyword token '%s'.", l_KeywordToken->m_Keyword->m_Name);
            return NULL;
        }
    }

    // If all else fails, then emit an error.
    GABLE_error("Unexpected token '%s'.", GABUILD_PeekToken(0)->m_Lexeme);
    return NULL;
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void GABUILD_InitParser ()
{
    // Create the root block syntax node.
    s_Parser.m_RootBlock = GABUILD_CreateSyntax(GABUILD_ST_BLOCK, GABUILD_PeekToken(0));
}

void GABUILD_ShutdownParser ()
{
    // Destroy the root block syntax node.
    GABUILD_DestroySyntax(s_Parser.m_RootBlock);
    s_Parser.m_RootBlock = NULL;
}

Bool GABUILD_Parse (GABUILD_Syntax* p_SyntaxBlock)
{
    // If `p_SyntaxBlock` is not NULL, then make sure it is a block node.
    if (p_SyntaxBlock != NULL && p_SyntaxBlock->m_Type != GABUILD_ST_BLOCK)
    {
        GABLE_error("The provided syntax node is not a block node.");
        return false;
    }

    // Parse through the lexer's tokens.
    while (GABUILD_HasMoreTokens() == true)
    {
        // Skip if newline token.
        if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_NEWLINE) != NULL)
        {
            continue;
        }

        // The next token is the lead token of this statement.
        s_Parser.m_LeadToken = GABUILD_PeekToken(0);

        // Parse the next statement.
        GABUILD_Syntax* l_Statement = GABUILD_ParseStatement();
        if (l_Statement == NULL)
        {
            GABLE_error("Failed to parse statement.");
            GABLE_error(" - In file '%s:%zu:%zu.",
                s_Parser.m_LeadToken->m_SourceFile,
                s_Parser.m_LeadToken->m_Line,
                s_Parser.m_LeadToken->m_Column
            );
            return false;
        }

        // Add the statement to either the root block or the provided block.
        if (p_SyntaxBlock == NULL)
        {
            GABUILD_PushToSyntaxBody(s_Parser.m_RootBlock, l_Statement);
        }
        else
        {
            GABUILD_PushToSyntaxBody(p_SyntaxBlock, l_Statement);
        }
    }

    return true;
}

const GABUILD_Syntax* GABUILD_GetRootSyntax ()
{
    return s_Parser.m_RootBlock;
}
