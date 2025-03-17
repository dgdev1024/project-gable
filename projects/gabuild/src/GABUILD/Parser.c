/**
 * @file     GABUILD/Parser.c
 */

#include <GABUILD/Parser.h>
#include <GABUILD/Token.h>
#include <GABUILD/Lexer.h>

// Parser Context Structure ////////////////////////////////////////////////////////////////////////

static struct
{
    GABUILD_Syntax*             m_Root;             ///< @brief Root Syntax Node
    const GABUILD_Token*        m_Lead;             ///< @brief Leading Token of the Current Statement
} s_Parser = {
    .m_Root         = NULL,
    .m_Lead         = NULL
};

// Static Function Prototypes //////////////////////////////////////////////////////////////////////

static GABUILD_Syntax* GABUILD_ParseMemberExpression ();
static GABUILD_Syntax* GABUILD_ParseCallExpression ();
static GABUILD_Syntax* GABUILD_ParseExpression ();
static GABUILD_Syntax* GABUILD_ParseStatement (Bool p_RequireSemicolon);

// Static Functions - Expressions //////////////////////////////////////////////////////////////////

// In the `GABUILD` language, an expression is a combination of values, variables, operators, and 
// functions that are evaluated to produce a result. Expressions can be as simple as a single value
// or as complex as a function call with multiple arguments.
//
// Expressions are parsed using a recursive descent parser. The parser starts by parsing the lowest
// precedence expression, and then moves up to higher precedence expressions. The parser uses the
// following order of operator precedence, based on the JavaScript (ES5) scripting language:
//
// 1. Grouping and Primary Expressions: `(...)` (highest precedence)
// 2. Member and Call Expressions: `expr.prop`, `expr[expr]`, `expr(args)`, `expr.prop(args)`, `expr[prop](args)`
// 3. Postfix Expressions: `expr++`, `expr--`
// 4. Prefix Expressions: `++expr`, `--expr`, `+expr`, `-expr`, `!expr`, `~expr`
// 5. Exponentiation: `**`
// 6. Multiplicative: `*`, `/`, `%`
// 7. Additive: `+`, `-`
// 8. Bitwise Shift: `<<`, `>>`
// 9. Relational: `<`, `>`, `<=`, `>=`
// 10. Equality: `==`, `!=`
// 11. Bitwise AND: `&`
// 12. Bitwise XOR: `^`
// 13. Bitwise OR: `|`
// 14. Logical AND: `&&`
// 15. Logical OR: `||`
// 16. Conditional: `? :`
// 17. Assignment: `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `<<=`, `>>=`, `&=`, `^=`, `|=` (lowest precedence)

// 1. Grouping and Primary Expressions
//
// Grouping expressions are enclosed in parentheses, and primary expressions are the simplest
// expressions in the language. Primary expressions include literals, identifiers, and grouped
// expressions.
static GABUILD_Syntax* GABUILD_ParsePrimaryExpression ()
{
    // Get the next token:
    const GABUILD_Token* l_Token = GABUILD_AdvanceToken();

    // Based on the token's type, create the appropriate syntax node:
    switch (l_Token->m_Type)
    {

        // Identifiers (eg. `x`, `index`, `myVariable`):
        case GABUILD_TOKEN_IDENTIFIER:
        {
            GABUILD_Syntax* l_Syntax = GABUILD_CreateSyntax(GABUILD_SYNTAX_IDENTIFIER, l_Token);
            strncpy(l_Syntax->m_String, l_Token->m_Lexeme, GABUILD_LITERAL_STRLEN);
            return l_Syntax;
        }

        // Numbers (eg. `42`, `3.14159`)
        case GABUILD_TOKEN_NUMBER:
        {
            GABUILD_Syntax* l_Syntax = GABUILD_CreateSyntax(GABUILD_SYNTAX_NUMBER, l_Token);
            l_Syntax->m_Number = strtod(l_Token->m_Lexeme, NULL);
            return l_Syntax;
        }

        // Binary Numbers (eg. `0b1010`, `0b1101`)
        case GABUILD_TOKEN_BINARY:
        {
            GABUILD_Syntax* l_Syntax = GABUILD_CreateSyntax(GABUILD_SYNTAX_NUMBER, l_Token);
            l_Syntax->m_Number = (Float64) strtoul(l_Token->m_Lexeme, NULL, 2);
            return l_Syntax;
        }

        // Octal Numbers (eg. `0o10`, `0o15`)
        case GABUILD_TOKEN_OCTAL:
        {
            GABUILD_Syntax* l_Syntax = GABUILD_CreateSyntax(GABUILD_SYNTAX_NUMBER, l_Token);
            l_Syntax->m_Number = (Float64) strtoul(l_Token->m_Lexeme, NULL, 8);
            return l_Syntax;
        }

        // Hexadecimal Numbers (eg. `0x0A`, `0x0D`)
        case GABUILD_TOKEN_HEXADECIMAL:
        {
            GABUILD_Syntax* l_Syntax = GABUILD_CreateSyntax(GABUILD_SYNTAX_NUMBER, l_Token);
            l_Syntax->m_Number = (Float64) strtoul(l_Token->m_Lexeme, NULL, 16);
            return l_Syntax;
        }

        // Character Literals (eg. `'a'`, `'b'`, `'\n'`)
        case GABUILD_TOKEN_CHARACTER:
        {
            GABUILD_Syntax* l_Syntax = GABUILD_CreateSyntax(GABUILD_SYNTAX_CHARACTER, l_Token);
            
            if (l_Token->m_Lexeme[0] == '\\')
            {
                switch (l_Token->m_Lexeme[1])
                {
                    case '0': l_Syntax->m_Number = (Float64) '\0'; break;
                    case 'a': l_Syntax->m_Number = (Float64) '\a'; break;
                    case 'b': l_Syntax->m_Number = (Float64) '\b'; break;
                    case 'f': l_Syntax->m_Number = (Float64) '\f'; break;
                    case 'n': l_Syntax->m_Number = (Float64) '\n'; break;
                    case 'r': l_Syntax->m_Number = (Float64) '\r'; break;
                    case 't': l_Syntax->m_Number = (Float64) '\t'; break;
                    case 'v': l_Syntax->m_Number = (Float64) '\v'; break;
                    case '\\': l_Syntax->m_Number = (Float64) '\\'; break;
                    case '\'': l_Syntax->m_Number = (Float64) '\''; break;
                    case '\"': l_Syntax->m_Number = (Float64) '\"'; break;
                    case '?': l_Syntax->m_Number = (Float64) '\?'; break;
                    default:
                        GABLE_error("Invalid escape sequence in character literal.");
                        GABUILD_DestroySyntax(l_Syntax);
                        return NULL;
                }
            }
            else
            {
                l_Syntax->m_Number = (Float64) l_Token->m_Lexeme[0];
            }

            return l_Syntax;
        }

        // Strings (eg. `"Hello, World!"`, `"This is a string."`)
        case GABUILD_TOKEN_STRING:
        {
            GABUILD_Syntax* l_Syntax = GABUILD_CreateSyntax(GABUILD_SYNTAX_STRING, l_Token);
            strncpy(l_Syntax->m_String, l_Token->m_Lexeme, GABUILD_LITERAL_STRLEN);
            return l_Syntax;
        }

        // Grouped Expressions (eg. `(x + y)`, `(a * b)`)
        case GABUILD_TOKEN_PARENTHESIS_OPEN:
        {
            GABUILD_Syntax* l_Syntax = GABUILD_ParseExpression();
            if (l_Syntax == NULL)
            {
                return NULL;
            }

            if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_PARENTHESIS_CLOSE) == NULL)
            {
                GABLE_error("Expected a closing parenthesis after the grouped expression.");
                GABUILD_DestroySyntax(l_Syntax);
                return NULL;
            }

            return l_Syntax;
        }

        default:
            GABLE_error("Unexpected '%s' token while parsing primary expression.", 
                GABUILD_StringifyToken(l_Token));
            return NULL;
    }
}

// 2. Member and Call Expressions
//
// Member expressions are used to access properties of an object, and call expressions are used to
// call functions with arguments. Both expressions are parsed in a similar manner, and may be
// combined to form more complex expressions.
//
// Examples:
// - `object.property` (accessing a property of an object)
// - `object[property]` (accessing a property of an object using a variable; a computed property)
// - `function()` (calling a function with no arguments)
// - `function(arg)` (calling a function with a single argument)
// - `function(arg1, arg2)` (calling a function with multiple arguments)
// - `object.method()` (calling a method of an object)
// - `object.method(arg1, arg2)` (calling a method of an object with arguments)
// - `object[property](arg1, arg2)` (calling a method of an object with computed property and arguments)
// - `object.method(arg1).method2(arg2)` (chaining method calls)
static GABUILD_Syntax* GABUILD_ParseMemberExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParsePrimaryExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }
    
    while (true)
    {
        // Peek the next token. Is it...
        // - ...a period `.` (member access)?
        // - ...an opening square bracket `[` (computed property access)?
        // - ...an opening parenthesis `(` (function call)?
        const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
        if (l_Next->m_Type == GABUILD_TOKEN_PERIOD)
        {
            // A member access is being parsed (eg. `object.property`).
            // Advance the token past the period:
            GABUILD_AdvanceToken();

            // Parse the member identifier:
            GABUILD_Syntax* l_Member = GABUILD_ParseMemberExpression();
            if (l_Member == NULL)
            {
                GABUILD_DestroySyntax(l_Syntax);
                return NULL;
            }

            // Create a new syntax node for the member access:
            GABUILD_Syntax* l_Access = GABUILD_CreateSyntax(GABUILD_SYNTAX_MEMBER, l_Next);
            l_Access->m_Left = l_Syntax;
            l_Access->m_Right = l_Member;
            l_Syntax = l_Access;
        }
        else if (l_Next->m_Type == GABUILD_TOKEN_BRACKET_OPEN)
        {
            // A computed property access is being parsed (eg. `object[property]`).
            // Advance the token past the opening square bracket:
            GABUILD_AdvanceToken();

            // Parse the property expression:
            GABUILD_Syntax* l_Property = GABUILD_ParseExpression();
            if (l_Property == NULL)
            {
                GABUILD_DestroySyntax(l_Syntax);
                return NULL;
            }

            // Create a new syntax node for the computed property access:
            GABUILD_Syntax* l_Access = GABUILD_CreateSyntax(GABUILD_SYNTAX_MEMBER, l_Next);
            l_Access->m_Left = l_Syntax;
            l_Access->m_Right = l_Property;
            l_Syntax = l_Access;

            // Ensure a closing square bracket is present:
            if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_BRACKET_CLOSE) == NULL)
            {
                GABLE_error("Expected a closing square bracket after the computed property access.");
                GABUILD_DestroySyntax(l_Syntax);
                return NULL;
            }
        }
        else if (l_Next->m_Type == GABUILD_TOKEN_PARENTHESIS_OPEN)
        {
            // A function or method call is being parsed (eg. `function()`, `object.method()`).
            // Advance the token past the opening parenthesis:
            GABUILD_AdvanceToken();

            // Create a new syntax node for the call expression:
            GABUILD_Syntax* l_Call = GABUILD_CreateSyntax(GABUILD_SYNTAX_CALL, l_Next);
            l_Call->m_Callee = l_Syntax;
            l_Syntax = l_Call;

            // Parse the arguments of the call expression:
            while (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_PARENTHESIS_CLOSE) == NULL)
            {
                // Parse the argument expression:
                GABUILD_Syntax* l_Argument = GABUILD_ParseExpression();
                if (l_Argument == NULL)
                {
                    GABUILD_DestroySyntax(l_Syntax);
                    return NULL;
                }

                // Add the argument to the call expression's argument list.
                GABUILD_PushToArguments(l_Syntax, l_Argument);

                // If the next token is a comma, advance past it and continue.
                if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_COMMA) != NULL)
                {
                    continue;
                }
                else if (GABUILD_PeekToken(0)->m_Type != GABUILD_TOKEN_PARENTHESIS_CLOSE)
                {
                    GABLE_error("Expected a comma or closing parenthesis after the argument.");
                    GABUILD_DestroySyntax(l_Syntax);
                    return NULL;
                }
            }
        }
        else
        {
            break;
        }
    }

    return l_Syntax;
}

// 3. Postfix Expressions
//
// Postfix expressions are unary expressions which follow the operand. The most common postfix
// expressions are the increment and decrement operators, which increment or decrement the operand
// by one after the expression is evaluated.
//
// Examples:
// - `x++` (evaluate `x`, then increment `x`)
// - `y--` (evaluate `y`, then decrement `y`)
static GABUILD_Syntax* GABUILD_ParsePostfixExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseMemberExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it an increment or decrement operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (
        l_Next->m_Type == GABUILD_TOKEN_INCREMENT || 
        l_Next->m_Type == GABUILD_TOKEN_DECREMENT
    )
    {
        // A postfix increment or decrement operator is being parsed.
        // Advance the token past the operator:
        GABUILD_AdvanceToken();

        // Create a new syntax node for the postfix expression.
        // Postfix unary expressions have their operand on the lefthand side.
        GABUILD_Syntax* l_Postfix = GABUILD_CreateSyntax(GABUILD_SYNTAX_UNARY, l_Next);
        l_Postfix->m_Left = l_Syntax;
        l_Syntax = l_Postfix;
    }

    return l_Syntax;
}

// 4. Prefix Expressions
//
// Prefix expressions are unary expressions which precede the operand. The most common prefix
// expressions are the increment and decrement operators, which increment or decrement the operand
// by one before the expression is evaluated.
//
// Examples:
// - `++x` (increment `x`, then evaluate `x`)
// - `--y` (decrement `y`, then evaluate `y`)
// - `+a` (evaluate `a` as a positive number)
// - `-b` (evaluate `b` as a negative number)
// - `!c` (evaluate `c` as a boolean NOT)
// - `~d` (evaluate `d` as a bitwise NOT)
static GABUILD_Syntax* GABUILD_ParsePrefixExpression ()
{
    // Peek the next token. Is it a prefix operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (
        l_Next->m_Type == GABUILD_TOKEN_INCREMENT || 
        l_Next->m_Type == GABUILD_TOKEN_DECREMENT ||
        l_Next->m_Type == GABUILD_TOKEN_PLUS || 
        l_Next->m_Type == GABUILD_TOKEN_MINUS ||
        l_Next->m_Type == GABUILD_TOKEN_LOGICAL_NOT || 
        l_Next->m_Type == GABUILD_TOKEN_BITWISE_NOT
    )
    {
        // A prefix operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the operand expression:
        GABUILD_Syntax* l_Operand = GABUILD_ParsePrefixExpression();
        if (l_Operand == NULL)
        {
            return NULL;
        }

        // Create a new syntax node for the prefix expression.
        // Prefix unary expressions have their operand on the righthand side.
        GABUILD_Syntax* l_Prefix = GABUILD_CreateSyntax(GABUILD_SYNTAX_UNARY, l_Next);
        l_Prefix->m_Right = l_Operand;
        return l_Prefix;
    }

    // If the next token is not a prefix operator, parse a postfix expression:
    return GABUILD_ParsePostfixExpression();
}

// 5. Exponentiation
//
// Exponentiation expressions are binary expressions which raise the lefthand side to the power of
// the righthand side. Exponentiation has right-to-left associativity.
//
// Examples:
// - `a ** b` (raise `a` to the power of `b`)
static GABUILD_Syntax* GABUILD_ParseExponentiationExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParsePrefixExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it an exponentiation operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (l_Next->m_Type == GABUILD_TOKEN_EXPONENT)
    {
        // An exponentiation operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the exponentiation expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseExponentiationExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the exponentiation expression:
        GABUILD_Syntax* l_Exponentiation = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_Exponentiation->m_Operator = l_Next->m_Type;
        l_Exponentiation->m_Left = l_Syntax;
        l_Exponentiation->m_Right = l_Right;
        return l_Exponentiation;
    }

    return l_Syntax;
}

// 6. Multiplicative
//
// Multiplicative expressions are binary expressions which multiply, divide, or find the remainder
// of the lefthand side and righthand side. Multiplicative expressions have left-to-right
// associativity.
//
// Examples:
// - `a * b` (multiply `a` by `b`)
// - `a / b` (divide `a` by `b`)
// - `a % b` (find the remainder of `a` divided by `b`)
static GABUILD_Syntax* GABUILD_ParseMultiplicativeExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseExponentiationExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a multiplicative operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (
        l_Next->m_Type == GABUILD_TOKEN_MULTIPLY || 
        l_Next->m_Type == GABUILD_TOKEN_DIVIDE || 
        l_Next->m_Type == GABUILD_TOKEN_MODULO
    )
    {
        // A multiplicative operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the multiplicative expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseMultiplicativeExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the multiplicative expression:
        GABUILD_Syntax* l_Multiplicative = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_Multiplicative->m_Operator = l_Next->m_Type;
        l_Multiplicative->m_Left = l_Syntax;
        l_Multiplicative->m_Right = l_Right;
        return l_Multiplicative;
    }

    return l_Syntax;
}

// 7. Additive
//
// Additive expressions are binary expressions which add or subtract the lefthand side and righthand
// side. Additive expressions have left-to-right associativity.
//
// Examples:
// - `a + b` (add `a` and `b`)
// - `a - b` (subtract `b` from `a`)
static GABUILD_Syntax* GABUILD_ParseAdditiveExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseMultiplicativeExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it an additive operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (l_Next->m_Type == GABUILD_TOKEN_PLUS || l_Next->m_Type == GABUILD_TOKEN_MINUS)
    {
        // An additive operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the additive expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseAdditiveExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the additive expression:
        GABUILD_Syntax* l_Additive = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_Additive->m_Operator = l_Next->m_Type;
        l_Additive->m_Left = l_Syntax;
        l_Additive->m_Right = l_Right;
        return l_Additive;
    }

    return l_Syntax;
}

// 8. Bitwise Shift
//
// Bitwise shift expressions are binary expressions which shift the bits of the lefthand side left
// or right by the number of bits specified by the righthand side. Bitwise shift expressions have
// left-to-right associativity.
//
// Examples:
// - `a << b` (shift the bits of `a` left by `b` bits)
// - `a >> b` (shift the bits of `a` right by `b` bits)
static GABUILD_Syntax* GABUILD_ParseBitwiseShiftExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseAdditiveExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a bitwise shift operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (
        l_Next->m_Type == GABUILD_TOKEN_BITWISE_SHIFT_LEFT || 
        l_Next->m_Type == GABUILD_TOKEN_BITWISE_SHIFT_RIGHT
    )
    {
        // A bitwise shift operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the bitwise shift expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseBitwiseShiftExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the bitwise shift expression:
        GABUILD_Syntax* l_Shift = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_Shift->m_Operator = l_Next->m_Type;
        l_Shift->m_Left = l_Syntax;
        l_Shift->m_Right = l_Right;
        return l_Shift;
    }

    return l_Syntax;
}

// 9. Relational
//
// Relational expressions are binary expressions which compare the lefthand side and righthand side
// to determine if the lefthand side is less than, greater than, less than or equal to, or greater
// than or equal to the righthand side. Relational expressions have left-to-right associativity.
//
// Examples:
// - `a < b` (is `a` less than `b`?)
// - `a > b` (is `a` greater than `b`?)
// - `a <= b` (is `a` less than or equal to `b`?)
// - `a >= b` (is `a` greater than or equal to `b`?)
static GABUILD_Syntax* GABUILD_ParseRelationalExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseBitwiseShiftExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a relational operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (
        l_Next->m_Type == GABUILD_TOKEN_COMPARE_LESS || 
        l_Next->m_Type == GABUILD_TOKEN_COMPARE_GREATER || 
        l_Next->m_Type == GABUILD_TOKEN_COMPARE_LESS_EQUAL || 
        l_Next->m_Type == GABUILD_TOKEN_COMPARE_GREATER_EQUAL
    )
    {
        // A relational operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the relational expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseRelationalExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the relational expression:
        GABUILD_Syntax* l_Relational = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_Relational->m_Operator = l_Next->m_Type;
        l_Relational->m_Left = l_Syntax;
        l_Relational->m_Right = l_Right;
        return l_Relational;
    }

    return l_Syntax;
}

// 10. Equality
//
// Equality expressions are binary expressions which compare the lefthand side and righthand side to
// determine if they are equal or not equal. Equality expressions have left-to-right associativity.
//
// Examples:
// - `a == b` (is `a` equal to `b`?)
// - `a != b` (is `a` not equal to `b`?)
static GABUILD_Syntax* GABUILD_ParseEqualityExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseRelationalExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it an equality operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (l_Next->m_Type == GABUILD_TOKEN_COMPARE_EQUAL || l_Next->m_Type == GABUILD_TOKEN_COMPARE_NOT_EQUAL)
    {
        // An equality operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the equality expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseEqualityExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the equality expression:
        GABUILD_Syntax* l_Equality = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_Equality->m_Operator = l_Next->m_Type;
        l_Equality->m_Left = l_Syntax;
        l_Equality->m_Right = l_Right;
        return l_Equality;
    }

    return l_Syntax;
}

// 11. Bitwise AND
//
// Bitwise AND expressions are binary expressions which perform a bitwise AND operation on the
// lefthand side and righthand side. Bitwise AND expressions have left-to-right associativity.
//
// Examples:
// - `a & b` (perform a bitwise AND on `a` and `b`)
static GABUILD_Syntax* GABUILD_ParseBitwiseANDExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseEqualityExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a bitwise AND operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (l_Next->m_Type == GABUILD_TOKEN_BITWISE_AND)
    {
        // A bitwise AND operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the bitwise AND expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseBitwiseANDExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the bitwise AND expression:
        GABUILD_Syntax* l_BitwiseAnd = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_BitwiseAnd->m_Operator = l_Next->m_Type;
        l_BitwiseAnd->m_Left = l_Syntax;
        l_BitwiseAnd->m_Right = l_Right;
        return l_BitwiseAnd;
    }

    return l_Syntax;
}

// 12. Bitwise XOR
//
// Bitwise XOR expressions are binary expressions which perform a bitwise XOR operation on the
// lefthand side and righthand side. Bitwise XOR expressions have left-to-right associativity.
//
// Examples:
// - `a ^ b` (perform a bitwise XOR on `a` and `b`)
static GABUILD_Syntax* GABUILD_ParseBitwiseXORExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseBitwiseANDExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a bitwise XOR operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (l_Next->m_Type == GABUILD_TOKEN_BITWISE_XOR)
    {
        // A bitwise XOR operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the bitwise XOR expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseBitwiseXORExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the bitwise XOR expression:
        GABUILD_Syntax* l_BitwiseXOR = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_BitwiseXOR->m_Operator = l_Next->m_Type;
        l_BitwiseXOR->m_Left = l_Syntax;
        l_BitwiseXOR->m_Right = l_Right;
        return l_BitwiseXOR;
    }

    return l_Syntax;
}

// 13. Bitwise OR
//
// Bitwise OR expressions are binary expressions which perform a bitwise OR operation on the
// lefthand side and righthand side. Bitwise OR expressions have left-to-right associativity.
//
// Examples:
// - `a | b` (perform a bitwise OR on `a` and `b`)
static GABUILD_Syntax* GABUILD_ParseBitwiseORExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseBitwiseXORExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a bitwise OR operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (l_Next->m_Type == GABUILD_TOKEN_BITWISE_OR)
    {
        // A bitwise OR operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the bitwise OR expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseBitwiseORExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the bitwise OR expression:
        GABUILD_Syntax* l_BitwiseOR = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_BitwiseOR->m_Operator = l_Next->m_Type;
        l_BitwiseOR->m_Left = l_Syntax;
        l_BitwiseOR->m_Right = l_Right;
        return l_BitwiseOR;
    }

    return l_Syntax;
}

// 14. Logical AND
//
// Logical AND expressions are binary expressions which perform a logical AND operation on the
// lefthand side and righthand side. Logical AND expressions have left-to-right associativity.
//
// Examples:
// - `a && b` (is `a` true AND `b` true?)
static GABUILD_Syntax* GABUILD_ParseLogicalANDExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseBitwiseORExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a logical AND operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (l_Next->m_Type == GABUILD_TOKEN_LOGICAL_AND)
    {
        // A logical AND operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the logical AND expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseLogicalANDExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the logical AND expression:
        GABUILD_Syntax* l_LogicalAnd = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_LogicalAnd->m_Operator = l_Next->m_Type;
        l_LogicalAnd->m_Left = l_Syntax;
        l_LogicalAnd->m_Right = l_Right;
        return l_LogicalAnd;
    }

    return l_Syntax;
}

// 15. Logical OR
//
// Logical OR expressions are binary expressions which perform a logical OR operation on the
// lefthand side and righthand side. Logical OR expressions have left-to-right associativity.
//
// Examples:
// - `a || b` (is `a` true OR `b` true?)
static GABUILD_Syntax* GABUILD_ParseLogicalORExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseLogicalANDExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a logical OR operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (l_Next->m_Type == GABUILD_TOKEN_LOGICAL_OR)
    {
        // A logical OR operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the logical OR expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseLogicalORExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the logical OR expression:
        GABUILD_Syntax* l_LogicalOr = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_LogicalOr->m_Operator = l_Next->m_Type;
        l_LogicalOr->m_Left = l_Syntax;
        l_LogicalOr->m_Right = l_Right;
        return l_LogicalOr;
    }

    return l_Syntax;
}

// 16. Conditional
//
// Conditional expressions are ternary expressions which evaluate the condition expression. If the
// condition is true, the lefthand side is evaluated; otherwise, the righthand side is evaluated.
// Conditional expressions have right-to-left associativity.
//
// Examples:
// - `a ? b : c` (if `a` is true, evaluate `b`; otherwise, evaluate `c`)
static GABUILD_Syntax* GABUILD_ParseConditionalExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseLogicalORExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it a conditional operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (l_Next->m_Type == GABUILD_TOKEN_QUESTION)
    {
        // A conditional operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the lefthand side of the conditional expression:
        GABUILD_Syntax* l_Left = GABUILD_ParseExpression();
        if (l_Left == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Ensure a colon is present after the lefthand side:
        if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_COLON) == NULL)
        {
            GABLE_error("Expected a colon after the lefthand side of the conditional expression.");
            GABUILD_DestroySyntax(l_Syntax);
            GABUILD_DestroySyntax(l_Left);
            return NULL;
        }

        // Parse the righthand side of the conditional expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            GABUILD_DestroySyntax(l_Left);
            return NULL;
        }

        // Create a new syntax node for the conditional expression:
        GABUILD_Syntax* l_Conditional = GABUILD_CreateSyntax(GABUILD_SYNTAX_TERNARY, l_Next);
        l_Conditional->m_Condition = l_Syntax;
        l_Conditional->m_TrueBranch = l_Left;
        l_Conditional->m_FalseBranch = l_Right;
        return l_Conditional;
    }

    return l_Syntax;
}

// 17. Assignment
//
// Assignment expressions are binary expressions which assign the righthand side to the lefthand
// side. Assignment expressions have right-to-left associativity.
//
// Examples:
// - `a = b` (assign `b` to `a`)
// - `a += b` (add `b` to `a`, then assign the result to `a`)
// - `a -= b` (subtract `b` from `a`, then assign the result to `a`)
// - `a *= b` (multiply `a` by `b`, then assign the result to `a`)
// - `a **= b` (raise `a` to the power of `b`, then assign the result to `a`)
// - `a /= b` (divide `a` by `b`, then assign the result to `a`)
// - `a %= b` (find the remainder of `a` divided by `b`, then assign the result to `a`)
// - `a <<= b` (shift the bits of `a` left by `b` bits, then assign the result to `a`)
// - `a >>= b` (shift the bits of `a` right by `b` bits, then assign the result to `a`)
// - `a &= b` (perform a bitwise AND on `a` and `b`, then assign the result to `a`)
// - `a ^= b` (perform a bitwise XOR on `a` and `b`, then assign the result to `a`)
// - `a |= b` (perform a bitwise OR on `a` and `b`, then assign the result to `a`)
static GABUILD_Syntax* GABUILD_ParseAssignmentExpression ()
{
    GABUILD_Syntax* l_Syntax = GABUILD_ParseConditionalExpression();
    if (l_Syntax == NULL)
    {
        return NULL;
    }

    // Peek the next token. Is it an assignment operator?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_EQUAL || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_PLUS || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_MINUS || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_MULTIPLY || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_EXPONENT || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_DIVIDE || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_MODULO || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_BITWISE_SHIFT_LEFT || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_BITWISE_SHIFT_RIGHT || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_BITWISE_AND || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_BITWISE_XOR || 
        l_Next->m_Type == GABUILD_TOKEN_ASSIGN_BITWISE_OR
    )
    {
        // An assignment operator is being parsed. Advance the token:
        GABUILD_AdvanceToken();

        // Parse the righthand side of the assignment expression:
        GABUILD_Syntax* l_Right = GABUILD_ParseExpression();
        if (l_Right == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Create a new syntax node for the assignment expression:
        GABUILD_Syntax* l_Assignment = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, l_Next);
        l_Assignment->m_Left = l_Syntax;
        l_Assignment->m_Right = l_Right;
        l_Assignment->m_Operator = l_Next->m_Type;
        return l_Assignment;
    }

    return l_Syntax;
}

// Object Expressions
//
// Object expressions are unary expressions which create a new object. Object expressions have the
// following syntax:
//
//     `{ key1: value1, key2: value2, ... }`
//
// Examples:
// - `{}` (create an empty object)
// - `{ x: 1, y: 2 }` (create an object with two properties)
static GABUILD_Syntax* GABUILD_ParseObjectExpression ()
{
    // Advance the token past the opening brace:
    GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_BRACE_OPEN);

    // Create a new syntax node for the object expression:
    GABUILD_Syntax* l_Syntax = GABUILD_CreateSyntax(GABUILD_SYNTAX_STRUCTURE, s_Parser.m_Lead);

    // Parse the properties of the object expression:
    while (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_BRACE_CLOSE) == NULL)
    {
        // Parse the property's key. It's expected to be an identifier:
        GABUILD_Syntax* l_Key = GABUILD_ParsePrimaryExpression();
        if (l_Key == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }
        else if (l_Key->m_Type != GABUILD_SYNTAX_IDENTIFIER)
        {
            GABLE_error("Expected an identifier as the key of the object property.");
            GABUILD_DestroySyntax(l_Syntax);
            GABUILD_DestroySyntax(l_Key);
            return NULL;
        }

        // Ensure a colon is present after the key:
        if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_COLON) == NULL)
        {
            GABLE_error("Expected a colon after the key of the object property.");
            GABUILD_DestroySyntax(l_Syntax);
            GABUILD_DestroySyntax(l_Key);
            return NULL;
        }

        // Parse the property's value:
        GABUILD_Syntax* l_Value = GABUILD_ParseExpression();
        if (l_Value == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            GABUILD_DestroySyntax(l_Key);
            return NULL;
        }

        // Create a new syntax node for the object property:
        GABUILD_Syntax* l_Property = GABUILD_CreateSyntax(GABUILD_SYNTAX_BINARY, s_Parser.m_Lead);
        l_Property->m_Left = l_Key;
        l_Property->m_Right = l_Value;
        l_Property->m_Operator = GABUILD_TOKEN_COLON;

        // Add the property to the object expression:
        GABUILD_PushToBody(l_Syntax, l_Property);

        // If the next token is a comma, advance the token and continue parsing properties:
        if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_COMMA) != NULL)
        {
            continue;
        }
        else if (GABUILD_PeekToken(0)->m_Type != GABUILD_TOKEN_BRACE_CLOSE)
        {
            GABLE_error("Expected a comma or closing brace after the object property.");
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }
    }

    return l_Syntax;
}

// Array Expressions
//
// Array expressions are unary expressions which create a new array. Array expressions have the
// following syntax:
//
//     `[ element1, element2, ... ]`
//
// Examples:
// - `[]` (create an empty array)
// - `[1, 2, 3]` (create an array with three elements)
static GABUILD_Syntax* GABUILD_ParseArrayExpression ()
{
    // Advance the token past the opening bracket:
    GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_BRACKET_OPEN);

    // Create a new syntax node for the array expression:
    GABUILD_Syntax* l_Syntax = GABUILD_CreateSyntax(GABUILD_SYNTAX_ARRAY, s_Parser.m_Lead);

    // Parse the elements of the array expression:
    while (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_BRACKET_CLOSE) == NULL)
    {
        // Parse the element of the array:
        GABUILD_Syntax* l_Element = GABUILD_ParseExpression();
        if (l_Element == NULL)
        {
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }

        // Add the element to the array expression:
        GABUILD_PushToBody(l_Syntax, l_Element);

        // If the next token is a comma, advance the token and continue parsing elements:
        if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_COMMA) != NULL)
        {
            continue;
        }
        else if (GABUILD_PeekToken(0)->m_Type != GABUILD_TOKEN_BRACKET_CLOSE)
        {
            GABLE_error("Expected a comma or closing bracket after the array element.");
            GABUILD_DestroySyntax(l_Syntax);
            return NULL;
        }
    }

    return l_Syntax;
}

GABUILD_Syntax* GABUILD_ParseExpression ()
{
    // Peek the next token. Is it an opening brace or bracket?
    const GABUILD_Token* l_Next = GABUILD_PeekToken(0);
    if (l_Next->m_Type == GABUILD_TOKEN_BRACE_OPEN)
    {
        // An object expression is being parsed.
        return GABUILD_ParseObjectExpression();
    }
    else if (l_Next->m_Type == GABUILD_TOKEN_BRACKET_OPEN)
    {
        // An array expression is being parsed.
        return GABUILD_ParseArrayExpression();
    }

    return GABUILD_ParseAssignmentExpression();
}

// Static Functions - Statements ///////////////////////////////////////////////////////////////////

GABUILD_Syntax* GABUILD_ParseStatement (Bool p_RequireSemicolon)
{
    GABUILD_Syntax* l_Statement = NULL;

    // Look up the type of the leading token. Is it a keyword?
    const GABUILD_Keyword* l_Keyword = s_Parser.m_Lead->m_Keyword;
    if (l_Keyword != NULL && l_Keyword->m_Type != GABUILD_KT_NONE)
    {
        switch (l_Keyword->m_Type)
        {
            default:
                GABLE_error("Unexpected keyword '%s' while parsing statement.", l_Keyword->m_Name);
                return NULL;
        }
    }
    else
    {
        l_Statement = GABUILD_ParseExpression();
        if (l_Statement == NULL)
        {
            return NULL;
        }
    }

    // If a semicolon is required, make sure it is present:
    if (p_RequireSemicolon == true)
    {
        if (GABUILD_AdvanceTokenIfType(GABUILD_TOKEN_SEMICOLON) == NULL)
        {
            GABLE_error("Expected a semicolon at the end of the statement.");
            GABUILD_DestroySyntax(l_Statement);
            return NULL;
        }
    }

    return l_Statement;
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void GABUILD_InitParser ()
{
    // Initialize the parser's root syntax node as a block:
    s_Parser.m_Root = GABUILD_CreateSyntax(GABUILD_SYNTAX_BLOCK, NULL);
}

void GABUILD_ShutdownParser ()
{
    // Destroy the parser's root syntax node:
    GABUILD_DestroySyntax(s_Parser.m_Root);
}

Bool GABUILD_ParseTokens (GABUILD_Syntax* p_Syntax)
{
    // If a valid syntax node was provided in `p_Syntax`, make sure it is a block:
    if (p_Syntax != NULL && p_Syntax->m_Type != GABUILD_SYNTAX_BLOCK)
    {
        GABLE_error("A block syntax node is required to parse tokens.");
        return false;
    }

    // Parse as long as the lexer has tokens to provide:
    while (GABUILD_HasMoreTokens() == true)
    {
        // Get the leading token of the current statement:
        s_Parser.m_Lead = GABUILD_PeekToken(0);

        // Attempt to parse the current statement:
        GABUILD_Syntax* l_Statement = GABUILD_ParseStatement(true);
        if (l_Statement == NULL)
        {
            GABLE_error("Error parsing statement.");
            GABLE_error(" - In file '%s:%zu:%zu'.",
                s_Parser.m_Lead->m_SourceFile,
                s_Parser.m_Lead->m_Line,
                s_Parser.m_Lead->m_Column
            );
            return false;
        }

        // Add the statement to the root syntax node, or `p_Syntax` if provided:
        if (p_Syntax != NULL)
        {
            GABUILD_PushToBody(p_Syntax, l_Statement);
        }
        else
        {
            GABUILD_PushToBody(s_Parser.m_Root, l_Statement);
        }
    }

    return true;
}
