/**
 * @file     GABUILD/Syntax.c
 */

#include <GABUILD/Syntax.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define GABUILD_SYNTAX_BLOCK_CAPACITY 16

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABUILD_Syntax* GABUILD_CreateSyntax (GABUILD_SyntaxType p_Type, const GABUILD_Token* p_Token)
{
    GABUILD_Syntax* l_Syntax = GABLE_calloc(1, GABUILD_Syntax);
    GABLE_pexpect(l_Syntax, "Failed to allocate syntax structure");

    l_Syntax->m_Type = p_Type;
    l_Syntax->m_LeadToken = p_Token;

    // The following syntax types call for a string value:
    // - GABUILD_SYNTAX_LABEL
    // - GABUILD_SYNTAX_LET
    // - GABUILD_SYNTAX_CONST
    // - GABUILD_SYNTAX_INCLUDE
    // - GABUILD_SYNTAX_INCBIN
    // - GABUILD_SYNTAX_STRING
    // - GABUILD_SYNTAX_IDENTIFIER
    // - GABUILD_SYNTAX_FUNCTION
    // - GABUILD_SYNTAX_CALL
    if (
        p_Type == GABUILD_SYNTAX_LABEL ||
        p_Type == GABUILD_SYNTAX_LET ||
        p_Type == GABUILD_SYNTAX_CONST ||
        p_Type == GABUILD_SYNTAX_INCLUDE ||
        p_Type == GABUILD_SYNTAX_INCBIN ||
        p_Type == GABUILD_SYNTAX_STRING ||
        p_Type == GABUILD_SYNTAX_IDENTIFIER ||
        p_Type == GABUILD_SYNTAX_FUNCTION ||
        p_Type == GABUILD_SYNTAX_CALL
    )
    {
        l_Syntax->m_String = GABLE_calloc(GABUILD_LITERAL_STRLEN, Char);
        GABLE_pexpect(l_Syntax->m_String, "Failed to allocate syntax string value");
    }

    // The following syntax types call for a block of statements:
    // - GABUILD_SYNTAX_BLOCK
    // - GABUILD_SYNTAX_ASM
    // - GABUILD_SYNTAX_FUNCTION
    // - GABUILD_SYNTAX_DATA
    // - GABUILD_SYNTAX_ARRAY
    // - GABUILD_SYNTAX_STRUCTURE
    if (
        p_Type == GABUILD_SYNTAX_BLOCK ||
        p_Type == GABUILD_SYNTAX_ASM ||
        p_Type == GABUILD_SYNTAX_FUNCTION ||
        p_Type == GABUILD_SYNTAX_DATA ||
        p_Type == GABUILD_SYNTAX_ARRAY ||
        p_Type == GABUILD_SYNTAX_STRUCTURE
    )
    {
        l_Syntax->m_Body = GABLE_calloc(GABUILD_SYNTAX_BLOCK_CAPACITY, GABUILD_Syntax*);
        GABLE_pexpect(l_Syntax->m_Body, "Failed to allocate syntax block");
        l_Syntax->m_BodyCapacity = GABUILD_SYNTAX_BLOCK_CAPACITY;
        l_Syntax->m_BodySize = 0;
    }

    // The following syntax types call for a list of arguments/parameters:
    // - GABUILD_SYNTAX_FUNCTION
    // - GABUILD_SYNTAX_CALL
    if (
        p_Type == GABUILD_SYNTAX_FUNCTION ||
        p_Type == GABUILD_SYNTAX_CALL
    )
    {
        l_Syntax->m_Arguments = GABLE_calloc(GABUILD_SYNTAX_BLOCK_CAPACITY, GABUILD_Syntax*);
        GABLE_pexpect(l_Syntax->m_Arguments, "Failed to allocate syntax arguments");
        l_Syntax->m_ArgumentCapacity = GABUILD_SYNTAX_BLOCK_CAPACITY;
        l_Syntax->m_ArgumentSize = 0;
    }

    return l_Syntax;
}

void GABUILD_DestroySyntax (GABUILD_Syntax* p_Syntax)
{
    if (p_Syntax != NULL)
    {
        // Free the string value of the syntax node, if it exists:
        GABLE_free(p_Syntax->m_String);

        // Free the body of the syntax node, if it exists:
        if (p_Syntax->m_Body != NULL)
        {
            for (Size i = 0; i < p_Syntax->m_BodySize; ++i)
            {
                GABUILD_DestroySyntax(p_Syntax->m_Body[i]);
            }

            GABLE_free(p_Syntax->m_Body);
        }

        // Free the arguments/parameters of the syntax node, if they exist:
        if (p_Syntax->m_Arguments != NULL)
        {
            for (Size i = 0; i < p_Syntax->m_ArgumentSize; ++i)
            {
                GABUILD_DestroySyntax(p_Syntax->m_Arguments[i]);
            }

            GABLE_free(p_Syntax->m_Arguments);
        }

        // Destroy every other member of the syntax node, if they exist:
        GABUILD_DestroySyntax(p_Syntax->m_Condition);
        GABUILD_DestroySyntax(p_Syntax->m_TrueBranch);
        GABUILD_DestroySyntax(p_Syntax->m_FalseBranch);
        GABUILD_DestroySyntax(p_Syntax->m_Initialization);
        GABUILD_DestroySyntax(p_Syntax->m_Increment);
        GABUILD_DestroySyntax(p_Syntax->m_Branch);
        GABUILD_DestroySyntax(p_Syntax->m_VariableValue);
        GABUILD_DestroySyntax(p_Syntax->m_Left);
        GABUILD_DestroySyntax(p_Syntax->m_Right);
        GABUILD_DestroySyntax(p_Syntax->m_Callee);

        GABLE_free(p_Syntax);
    }
}

void GABUILD_ResizeSyntax (GABUILD_Syntax* p_Syntax)
{
    if (p_Syntax == NULL)
    {
        return;
    }

    // Only resize the body of the syntax node if it is of the proper type:
    // - GABUILD_SYNTAX_BLOCK
    // - GABUILD_SYNTAX_ASM
    // - GABUILD_SYNTAX_FUNCTION
    // - GABUILD_SYNTAX_DATA
    // - GABUILD_SYNTAX_CALL
    // - GABUILD_SYNTAX_ARRAY
    // - GABUILD_SYNTAX_STRUCTURE
    switch (p_Syntax->m_Type)
    {
        case GABUILD_SYNTAX_BLOCK:
        case GABUILD_SYNTAX_ASM:
        case GABUILD_SYNTAX_FUNCTION:
        case GABUILD_SYNTAX_DATA:
        case GABUILD_SYNTAX_CALL:
        case GABUILD_SYNTAX_ARRAY:
        case GABUILD_SYNTAX_STRUCTURE:
            break;
        default:
            return;
    }

    // Resize the body of the syntax node, if it exists and is about to overflow:
    if (
        p_Syntax->m_Body != NULL &&
        p_Syntax->m_BodySize + 1 >= p_Syntax->m_BodyCapacity
    )
    {
        Size l_NewCapacity = p_Syntax->m_BodyCapacity * 2;
        GABUILD_Syntax** l_NewBody = GABLE_realloc(p_Syntax->m_Body, l_NewCapacity, GABUILD_Syntax*);
        GABLE_pexpect(l_NewBody, "Failed to resize syntax body");

        p_Syntax->m_Body = l_NewBody;
        p_Syntax->m_BodyCapacity = l_NewCapacity;
    }

    // Repeat the process for the arguments/parameters of the syntax node:
    if (
        p_Syntax->m_Arguments != NULL &&
        p_Syntax->m_ArgumentSize + 1 >= p_Syntax->m_ArgumentCapacity
    )
    {
        Size l_NewCapacity = p_Syntax->m_ArgumentCapacity * 2;
        GABUILD_Syntax** l_NewArguments = GABLE_realloc(p_Syntax->m_Arguments, l_NewCapacity, GABUILD_Syntax*);
        GABLE_pexpect(l_NewArguments, "Failed to resize syntax arguments");

        p_Syntax->m_Arguments = l_NewArguments;
        p_Syntax->m_ArgumentCapacity = l_NewCapacity;
    }
}

void GABUILD_PushToBody (GABUILD_Syntax* p_Syntax, GABUILD_Syntax* p_Node)
{
    if (p_Syntax == NULL || p_Node == NULL)
    {
        return;
    }

    // Only push to the body of the syntax node if it is of the proper type:
    // - GABUILD_SYNTAX_BLOCK
    // - GABUILD_SYNTAX_ASM
    // - GABUILD_SYNTAX_FUNCTION
    // - GABUILD_SYNTAX_DATA
    // - GABUILD_SYNTAX_CALL
    // - GABUILD_SYNTAX_ARRAY
    // - GABUILD_SYNTAX_STRUCTURE
    switch (p_Syntax->m_Type)
    {
        case GABUILD_SYNTAX_BLOCK:
        case GABUILD_SYNTAX_ASM:
        case GABUILD_SYNTAX_FUNCTION:
        case GABUILD_SYNTAX_DATA:
        case GABUILD_SYNTAX_CALL:
        case GABUILD_SYNTAX_ARRAY:
        case GABUILD_SYNTAX_STRUCTURE:
            break;
        default:
            return;
    }

    // Resize the body of the syntax node, if necessary:
    GABUILD_ResizeSyntax(p_Syntax);

    // Push the node to the body of the syntax node:
    p_Syntax->m_Body[p_Syntax->m_BodySize++] = p_Node;
}

void GABUILD_PushToArguments (GABUILD_Syntax* p_Syntax, GABUILD_Syntax* p_Argument)
{
    if (p_Syntax == NULL || p_Argument == NULL)
    {
        return;
    }

    // Only push to the arguments/parameters of the syntax node if it is of the proper type:
    // - GABUILD_SYNTAX_FUNCTION
    // - GABUILD_SYNTAX_CALL
    switch (p_Syntax->m_Type)
    {
        case GABUILD_SYNTAX_FUNCTION:
        case GABUILD_SYNTAX_CALL:
            break;
        default:
            return;
    }

    // Resize the arguments/parameters of the syntax node, if necessary:
    GABUILD_ResizeSyntax(p_Syntax);

    // Push the argument to the arguments/parameters of the syntax node:
    p_Syntax->m_Arguments[p_Syntax->m_ArgumentSize++] = p_Argument;
}
