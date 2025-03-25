/**
 * @file  GABUILD/Syntax.c
 */

#include <GABUILD/Syntax.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABUILD_Syntax* GABUILD_CreateSyntax (GABUILD_SyntaxType p_Type, const GABUILD_Token* p_Token)
{
    GABLE_assert(p_Token != NULL)

    GABUILD_Syntax* l_Syntax = GABLE_calloc(1, GABUILD_Syntax);
    GABLE_pexpect(l_Syntax, "Could not allocate memory for syntax node");

    l_Syntax->m_Type = p_Type;
    l_Syntax->m_Token = *p_Token;

    // If the syntax node calls for a string, allocate it.
    if (
        p_Type == GABUILD_ST_LABEL ||
        p_Type == GABUILD_ST_DEF ||
        p_Type == GABUILD_ST_MACRO ||
        p_Type == GABUILD_ST_MACRO_CALL ||
        p_Type == GABUILD_ST_IDENTIFIER ||
        p_Type == GABUILD_ST_STRING
    )
    {
        l_Syntax->m_String = GABLE_calloc(GABUILD_STRING_CAPACITY, Char);
        GABLE_pexpect(l_Syntax->m_String, "Could not allocate memory for syntax node string");
    }

    // If the syntax node calls for a body of child nodes, allocate it.
    if (
        p_Type == GABUILD_ST_BLOCK ||
        p_Type == GABUILD_ST_DATA ||
        p_Type == GABUILD_ST_MACRO_CALL
    )
    {
        l_Syntax->m_Body = GABLE_calloc(GABUILD_SYNTAX_BODY_INITIAL_CAPACITY, GABUILD_Syntax*);
        GABLE_pexpect(l_Syntax->m_Body, "Could not allocate memory for syntax node body");
        l_Syntax->m_BodyCapacity = GABUILD_SYNTAX_BODY_INITIAL_CAPACITY;
    }

    return l_Syntax;
}

GABUILD_Syntax* GABUILD_CopySyntax (const GABUILD_Syntax* p_Syntax)
{
    if (p_Syntax == NULL)
    {
        return NULL;
    }

    GABUILD_Syntax* l_Copy = GABUILD_CreateSyntax(p_Syntax->m_Type, &p_Syntax->m_Token);
    
    // Copy the string, if it exists.
    if (p_Syntax->m_String != NULL)
    {
        strncpy(l_Copy->m_String, p_Syntax->m_String, GABUILD_STRING_CAPACITY);
    }

    // Copy the body of child nodes, if it exists.
    if (p_Syntax->m_Body != NULL)
    {
        for (Size i = 0; i < p_Syntax->m_BodySize; ++i)
        {
            GABUILD_PushToSyntaxBody(l_Copy, GABUILD_CopySyntax(p_Syntax->m_Body[i]));
        }
    }

    // Copy the other child syntax nodes, if they exist.
    l_Copy->m_CountExpr = GABUILD_CopySyntax(p_Syntax->m_CountExpr);
    l_Copy->m_LeftExpr = GABUILD_CopySyntax(p_Syntax->m_LeftExpr);
    l_Copy->m_RightExpr = GABUILD_CopySyntax(p_Syntax->m_RightExpr);
    l_Copy->m_Operator = p_Syntax->m_Operator;
    l_Copy->m_Number = p_Syntax->m_Number;
    l_Copy->m_KeywordType = p_Syntax->m_KeywordType;

    return l_Copy;
}

void GABUILD_DestroySyntax (GABUILD_Syntax* p_Syntax)
{
    if (p_Syntax != NULL)
    {
        // Destroy the string, if it exists.
        if (p_Syntax->m_String != NULL)
        {
            GABLE_free(p_Syntax->m_String);
        }

        // Destroy the body of child nodes, if it exists.
        if (p_Syntax->m_Body != NULL)
        {
            for (Size i = 0; i < p_Syntax->m_BodySize; ++i)
            {
                GABUILD_DestroySyntax(p_Syntax->m_Body[i]);
            }

            GABLE_free(p_Syntax->m_Body);
        }

        // Destroy the other child syntax nodes, if they exist.
        GABUILD_DestroySyntax(p_Syntax->m_CountExpr);
        GABUILD_DestroySyntax(p_Syntax->m_LeftExpr);
        GABUILD_DestroySyntax(p_Syntax->m_RightExpr);

        GABLE_free(p_Syntax);
    }
}

void GABUILD_PushToSyntaxBody (GABUILD_Syntax* p_Parent, GABUILD_Syntax* p_Child)
{
    GABLE_assert(p_Parent != NULL)
    GABLE_assert(p_Child != NULL)

    // Do nothing if the syntax body is not allocated.
    if (p_Parent->m_Body == NULL)
    {
        return;
    }

    // Resize the body array if necessary.
    if (p_Parent->m_BodySize + 1 >= p_Parent->m_BodyCapacity)
    {
        Size l_NewCapacity = p_Parent->m_BodyCapacity * 2;
        GABUILD_Syntax** l_NewBody = GABLE_realloc(p_Parent->m_Body, l_NewCapacity, GABUILD_Syntax*);
        GABLE_pexpect(l_NewBody, "Could not reallocate memory for syntax body");

        p_Parent->m_Body = l_NewBody;
        p_Parent->m_BodyCapacity = l_NewCapacity;
    }

    // Add the child to the body.
    p_Parent->m_Body[p_Parent->m_BodySize++] = p_Child;
}