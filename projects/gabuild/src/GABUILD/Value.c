/**
 * @file  GABUILD/Value.c
 */

#include <GABUILD/Value.h>

// Static Functions ////////////////////////////////////////////////////////////////////////////////

static GABUILD_Value* GABUILD_CreateValue (GABUILD_ValueType p_Type)
{
    GABUILD_Value* l_Value = GABLE_calloc(1, GABUILD_Value);
    GABLE_pexpect(l_Value != NULL, "Could not allocate memory for a runtime value");

    l_Value->m_Type = p_Type;
    return l_Value;
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABUILD_Value* GABUILD_CreateVoidValue ()
{
    return GABUILD_CreateValue(GABUILD_VT_VOID);
}

GABUILD_Value* GABUILD_CreateNumberValue (Float64 p_Number)
{
    GABUILD_Value* l_Value = GABUILD_CreateValue(GABUILD_VT_NUMBER);
    
    Float64 l_IntegerPart = 0.0;
    Float64 l_FractionalPart = modf(p_Number, &l_IntegerPart);

    l_Value->m_Number = p_Number;
    l_Value->m_IntegerPart = (Uint64) l_IntegerPart;
    l_Value->m_FractionalPart = (Uint64) (l_FractionalPart * UINT64_MAX);

    return l_Value;
}

GABUILD_Value* GABUILD_CreateStringValue (const Char* p_String)
{
    GABLE_assert(p_String != NULL);

    GABUILD_Value* l_Value = GABUILD_CreateValue(GABUILD_VT_STRING);

    // Get the size of the string. Allocate memory for the string and copy it.
    Size l_Strlen = strlen(p_String);
    l_Value->m_String = GABLE_calloc(l_Strlen + 1, Char);
    GABLE_pexpect(l_Value->m_String != NULL, "Could not allocate memory for a string value");

    strncpy(l_Value->m_String, p_String, l_Strlen);
    return l_Value;
}

GABUILD_Value* GABUILD_CopyValue (const GABUILD_Value* p_Value)
{
    if (p_Value == NULL)
    {
        return NULL;
    }

    switch (p_Value->m_Type)
    {
        case GABUILD_VT_VOID:
            return GABUILD_CreateVoidValue();

        case GABUILD_VT_NUMBER:
            return GABUILD_CreateNumberValue(p_Value->m_Number);

        case GABUILD_VT_STRING:
            return GABUILD_CreateStringValue(p_Value->m_String);

        default:
            return NULL;
    }
}

void GABUILD_DestroyValue (GABUILD_Value* p_Value)
{
    if (p_Value != NULL)
    {
        if (p_Value->m_Type == GABUILD_VT_STRING)
        {
            GABLE_free(p_Value->m_String);
        }

        GABLE_free(p_Value);
    }
}

void GABUILD_PrintValue (const GABUILD_Value* p_Value)
{
    if (p_Value == NULL)
    {
        printf("null");
        return;
    }

    switch (p_Value->m_Type)
    {
        case GABUILD_VT_VOID:
            printf("void");
            break;

        case GABUILD_VT_NUMBER:
            if (p_Value->m_FractionalPart == 0)
            {
                printf("%lu", p_Value->m_IntegerPart);
            }
            else
            {
                printf("%lf", p_Value->m_Number);
            }
            break;

        case GABUILD_VT_STRING:
            printf("%s", p_Value->m_String);
            break;

        default:
            printf("unknown");
            break;
    }
}

GABUILD_Value* GABUILD_ConcatenateStringValues (const GABUILD_Value* p_LeftValue, const GABUILD_Value* p_RightValue)
{
    GABLE_assert(p_LeftValue != NULL && p_LeftValue->m_String != NULL);
    GABLE_assert(p_RightValue != NULL && p_RightValue->m_String != NULL);

    if (p_LeftValue->m_Type != GABUILD_VT_STRING || p_RightValue->m_Type != GABUILD_VT_STRING)
    {
        GABLE_error("Cannot concatenate non-string values.");
        return NULL;
    }

    // Get the size of the new string.
    Size l_LeftSize = strlen(p_LeftValue->m_String);
    Size l_RightSize = strlen(p_RightValue->m_String);
    Size l_NewSize = l_LeftSize + l_RightSize;

    // Allocate memory for the new string.
    Char* l_NewString = GABLE_calloc(l_NewSize + 1, Char);
    GABLE_pexpect(l_NewString != NULL, "Could not allocate memory for a concatenated string value");

    // Copy the left and right strings into the new string.
    strncpy(l_NewString, p_LeftValue->m_String, l_LeftSize);
    strncpy(l_NewString + l_LeftSize, p_RightValue->m_String, l_RightSize);

    // Create a new string value and return it.
    GABUILD_Value* l_NewValue = GABUILD_CreateValue(GABUILD_VT_STRING);
    l_NewValue->m_String = l_NewString;
    
    return l_NewValue;
}