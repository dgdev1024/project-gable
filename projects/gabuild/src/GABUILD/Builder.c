/**
 * @file  GABUILD/Builder.c
 */

#include <GABUILD/Lexer.h>
#include <GABUILD/Parser.h>
#include <GABUILD/Builder.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define GABUILD_BUILDER_INITIAL_CAPACITY 8
#define GABUILD_BUILDER_OUTPUT_CAPACITY 0x4000
#define GABUILD_BUILDER_CALL_STACK_SIZE 32

// Label Structure /////////////////////////////////////////////////////////////////////////////////

typedef struct GABUILD_Label
{
    Char*       m_Name;
    Uint16*     m_References;
    Size        m_ReferenceCount;
    Size        m_ReferenceCapacity;
    Uint16      m_Address;
    Bool        m_Resolved;
} GABUILD_Label;

// Macro Structure /////////////////////////////////////////////////////////////////////////////////

typedef struct GABUILD_Macro
{
    Char*           m_Name;
    GABUILD_Syntax* m_Block;
} GABUILD_Macro;

// Macro Call Structure ////////////////////////////////////////////////////////////////////////////

typedef struct GABUILD_MacroCall
{
    GABUILD_Macro*  m_Macro;
    GABUILD_Value** m_Arguments;
    Size            m_ArgumentCount;
    Size            m_ArgumentOffset;
} GABUILD_MacroCall;

// Builder Context Structure ///////////////////////////////////////////////////////////////////////

static struct
{
    Uint8               m_Output[GABUILD_BUILDER_OUTPUT_CAPACITY];
    GABUILD_Value*      m_Result;

    GABUILD_Label*      m_Labels;
    Size                m_LabelCount;
    Size                m_LabelCapacity;

    GABUILD_Macro*      m_Macros;
    Size                m_MacroCount;
    Size                m_MacroCapacity;

    GABUILD_Value**     m_DefineValues;
    Char**              m_DefineKeys;
    Size                m_DefineCount;
    Size                m_DefineCapacity;

    GABUILD_MacroCall*  m_MacroCallStack[GABUILD_BUILDER_CALL_STACK_SIZE];
    Size                m_MacroCallStackIndex;

    Size                m_OutputSize;
} s_Builder = {
    .m_Output = { 0 },
    .m_Result = NULL,
    .m_Labels = NULL,
    .m_LabelCount = 0,
    .m_LabelCapacity = 0,
    .m_Macros = NULL,
    .m_MacroCount = 0,
    .m_MacroCapacity = 0,
    .m_DefineValues = NULL,
    .m_DefineKeys = NULL,
    .m_DefineCount = 0,
    .m_DefineCapacity = 0,
    .m_MacroCallStack = { 0 },
    .m_MacroCallStackIndex = 0,
    .m_OutputSize = 0
};

// Static Function Prototypes //////////////////////////////////////////////////////////////////////

static GABUILD_Value* GABUILD_Evaluate (const GABUILD_Syntax* p_SyntaxNode);
static GABUILD_Value* GABUILD_EvaluateBlock (const GABUILD_Syntax* p_SyntaxNode);

// Static Functions - Macro Call Management ////////////////////////////////////////////////////////

static GABUILD_MacroCall* GABUILD_CreateMacroCall (Size p_ArgumentCount)
{
    GABUILD_MacroCall* l_MacroCall = GABLE_calloc(1, GABUILD_MacroCall);
    GABLE_pexpect(l_MacroCall != NULL, "Failed to allocate memory for macro call structure");

    l_MacroCall->m_Arguments = GABLE_calloc(p_ArgumentCount, GABUILD_Value*);
    GABLE_pexpect(l_MacroCall->m_Arguments != NULL, "Failed to allocate memory for macro call arguments");

    l_MacroCall->m_ArgumentCount = p_ArgumentCount;
    l_MacroCall->m_ArgumentOffset = 0;
    return l_MacroCall;
}

static void GABUILD_DestroyMacroCall (GABUILD_MacroCall* p_MacroCall)
{
    if (p_MacroCall != NULL)
    {
        if (p_MacroCall->m_Arguments != NULL)
        {
            for (Size i = 0; i < p_MacroCall->m_ArgumentCount; ++i)
            {
                GABUILD_DestroyValue(p_MacroCall->m_Arguments[i]);
            }

            GABLE_free(p_MacroCall->m_Arguments);
        }

        GABLE_free(p_MacroCall);
    }
}

// Static Functions - Internal Array Management ////////////////////////////////////////////////////

static void GABUILD_ResizeLabelReferences (GABUILD_Label* p_Label)
{
    if (p_Label->m_ReferenceCount + 1 >= p_Label->m_ReferenceCapacity)
    {
        Size l_NewCapacity      = p_Label->m_ReferenceCapacity * 2;
        Uint16* l_NewReferences = GABLE_realloc(p_Label->m_References, l_NewCapacity, Uint16);
        GABLE_pexpect(l_NewReferences != NULL, "Failed to reallocate memory for label references array");

        p_Label->m_References           = l_NewReferences;
        p_Label->m_ReferenceCapacity    = l_NewCapacity;
    }
}

static void GABUILD_ResizeLabelsArray ()
{
    if (s_Builder.m_LabelCount + 1 >= s_Builder.m_LabelCapacity)
    {
        Size l_NewCapacity  = s_Builder.m_LabelCapacity * 2;
        GABUILD_Label* l_NewLabels = GABLE_realloc(s_Builder.m_Labels, l_NewCapacity, GABUILD_Label);
        GABLE_pexpect(l_NewLabels != NULL, "Failed to reallocate memory for the builder's address labels array");

        s_Builder.m_Labels          = l_NewLabels;
        s_Builder.m_LabelCapacity   = l_NewCapacity;
    }
}

static void GABUILD_ResizeMacrosArray ()
{
    if (s_Builder.m_MacroCount + 1 >= s_Builder.m_MacroCapacity)
    {
        Size l_NewCapacity  = s_Builder.m_MacroCapacity * 2;
        GABUILD_Macro* l_NewMacros = GABLE_realloc(s_Builder.m_Macros, l_NewCapacity, GABUILD_Macro);
        GABLE_pexpect(l_NewMacros != NULL, "Failed to reallocate memory for the builder's macros array");

        s_Builder.m_Macros          = l_NewMacros;
        s_Builder.m_MacroCapacity   = l_NewCapacity;
    }
}

static void GABUILD_ResizeDefinesArrays ()
{
    if (s_Builder.m_DefineCount + 1 >= s_Builder.m_DefineCapacity)
    {
        Size l_NewCapacity = s_Builder.m_DefineCapacity * 2;

        GABUILD_Value** l_NewDefineValues = GABLE_realloc(s_Builder.m_DefineValues, l_NewCapacity, GABUILD_Value*);
        GABLE_pexpect(l_NewDefineValues != NULL, "Failed to reallocate memory for the builder's define values array");

        Char** l_NewDefineKeys = GABLE_realloc(s_Builder.m_DefineKeys, l_NewCapacity, Char*);
        GABLE_pexpect(l_NewDefineKeys != NULL, "Failed to reallocate memory for the builder's define keys array");

        s_Builder.m_DefineValues    = l_NewDefineValues;
        s_Builder.m_DefineKeys      = l_NewDefineKeys;
        s_Builder.m_DefineCapacity  = l_NewCapacity;
    }
}

// Static Functions - Output Buffer Management /////////////////////////////////////////////////////

static Bool GABUILD_DefineByte (Uint8 p_Value)
{
    if (s_Builder.m_OutputSize + 1 >= GABUILD_BUILDER_OUTPUT_CAPACITY)
    {
        GABLE_error("Output buffer overflowed while defining a byte.");
        return false;
    }

    s_Builder.m_Output[s_Builder.m_OutputSize++] = p_Value;
    return true;
}

static Bool GABUILD_DefineWord (Uint16 p_Value)
{
    if (s_Builder.m_OutputSize + 2 >= GABUILD_BUILDER_OUTPUT_CAPACITY)
    {
        GABLE_error("Output buffer overflowed while defining a word.");
        return false;
    }

    s_Builder.m_Output[s_Builder.m_OutputSize++] = (Uint8) (p_Value & 0xFF);
    s_Builder.m_Output[s_Builder.m_OutputSize++] = (Uint8) ((p_Value >> 8) & 0xFF);
    return true;
}

static Bool GABUILD_DefineLong (Uint32 p_Value)
{
    if (s_Builder.m_OutputSize + 4 >= GABUILD_BUILDER_OUTPUT_CAPACITY)
    {
        GABLE_error("Output buffer overflowed while defining a long.");
        return false;
    }

    s_Builder.m_Output[s_Builder.m_OutputSize++] = (Uint8) (p_Value & 0xFF);
    s_Builder.m_Output[s_Builder.m_OutputSize++] = (Uint8) ((p_Value >> 8) & 0xFF);
    s_Builder.m_Output[s_Builder.m_OutputSize++] = (Uint8) ((p_Value >> 16) & 0xFF);
    s_Builder.m_Output[s_Builder.m_OutputSize++] = (Uint8) ((p_Value >> 24) & 0xFF);
    return true;
}

static Bool GABUILD_DefineStringASCII (const Char* p_String)
{
    Size l_Length = strlen(p_String);
    if (s_Builder.m_OutputSize + l_Length + 1 >= GABUILD_BUILDER_OUTPUT_CAPACITY)
    {
        GABLE_error("Output buffer overflowed while defining an ASCII string.");
        return false;
    }

    for (Size i = 0; i < l_Length; ++i)
    {
        s_Builder.m_Output[s_Builder.m_OutputSize++] = (Uint8) p_String[i];
    }

    s_Builder.m_Output[s_Builder.m_OutputSize++] = 0;

    return true;
}

static Bool GABUILD_DefineBinaryFile (const Char* p_Filename)
{
    FILE* l_File = fopen(p_Filename, "rb");
    if (l_File == NULL)
    {
        GABLE_perror("Failed to open binary file '%s' for reading", p_Filename);
        return false;
    }

    // Get the file size.
    fseek(l_File, 0, SEEK_END);
    Int64 l_FileSize = ftell(l_File);
    if (l_FileSize < 0)
    {
        GABLE_perror("Failed to get size of binary file '%s'", p_Filename);
        fclose(l_File);
        return false;
    }
    else if (s_Builder.m_OutputSize + l_FileSize >= GABUILD_BUILDER_OUTPUT_CAPACITY)
    {
        GABLE_error("Output buffer overflowed while defining a binary file.");
        fclose(l_File);
        return false;
    }
    rewind(l_File);

    // Read the file into the output buffer.
    Size l_Read = fread(&s_Builder.m_Output[s_Builder.m_OutputSize], 1, l_FileSize, l_File);
    if (l_Read != l_FileSize || (ferror(l_File) && !feof(l_File)))
    {
        GABLE_perror("Failed to read binary file '%s'", p_Filename);
        fclose(l_File);
        return false;
    }

    fclose(l_File);
    return true;
}

// Static Functions - Assignment Operations ////////////////////////////////////////////////////////

static GABUILD_Value* GABUILD_PerformAssignmentOperation (const GABUILD_Value* p_LeftValue,
    const GABUILD_Value* p_RightValue, GABUILD_TokenType p_Operator)
{
    // If the operator is '=', then just return a copy of the right value.
    if (p_Operator == GABUILD_TOKEN_ASSIGN_EQUAL)
    {
        return GABUILD_CopyValue(p_RightValue);
    }

    // Check the type of the lefthand value.
    if (p_LeftValue->m_Type == GABUILD_VT_NUMBER)
    {
        // Check the type of the righthand value.
        if (p_RightValue->m_Type == GABUILD_VT_NUMBER)
        {
            // Check the operator type and perform the proper operation.
            switch (p_Operator)
            {
                case GABUILD_TOKEN_ASSIGN_PLUS:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number + p_RightValue->m_Number);
                case GABUILD_TOKEN_ASSIGN_MINUS:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number - p_RightValue->m_Number);
                case GABUILD_TOKEN_ASSIGN_MULTIPLY:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number * p_RightValue->m_Number);
                case GABUILD_TOKEN_ASSIGN_DIVIDE:
                    if (p_RightValue->m_IntegerPart == 0 && p_RightValue->m_FractionalPart == 0)
                    {
                        GABLE_error("Encountered attempted division by zero.");
                        return NULL;
                    }
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number / p_RightValue->m_Number);
                case GABUILD_TOKEN_ASSIGN_MODULO:
                    if (p_RightValue->m_IntegerPart == 0 && p_RightValue->m_FractionalPart == 0)
                    {
                        GABLE_error("Encountered modulo with attempted division by zero.");
                        return NULL;
                    }
                    return GABUILD_CreateNumberValue(fmod(p_LeftValue->m_Number, p_RightValue->m_Number));
                case GABUILD_TOKEN_ASSIGN_EXPONENT:
                    return GABUILD_CreateNumberValue(pow(p_LeftValue->m_Number, p_RightValue->m_Number));
                case GABUILD_TOKEN_ASSIGN_BITWISE_AND:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_IntegerPart & p_RightValue->m_IntegerPart);
                case GABUILD_TOKEN_ASSIGN_BITWISE_OR:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_IntegerPart | p_RightValue->m_IntegerPart);
                case GABUILD_TOKEN_ASSIGN_BITWISE_XOR:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_IntegerPart ^ p_RightValue->m_IntegerPart);
                case GABUILD_TOKEN_ASSIGN_BITWISE_SHIFT_LEFT:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_IntegerPart << p_RightValue->m_IntegerPart);
                case GABUILD_TOKEN_ASSIGN_BITWISE_SHIFT_RIGHT:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_IntegerPart >> p_RightValue->m_IntegerPart);
                default:
                    GABLE_error("Invalid operator type for number-vs-number assignment operation.");
                    return NULL;
            }
        }
        else
        {
            GABLE_error("Invalid righthand value type for assignment operation.");
            return NULL;
        }
    }
    else if (p_LeftValue->m_Type == GABUILD_VT_STRING)
    {
        // Check the type of the righthand value.
        if (p_RightValue->m_Type == GABUILD_VT_STRING)
        {
            // Check the operator type and perform the proper operation.
            switch (p_Operator)
            {
                case GABUILD_TOKEN_ASSIGN_PLUS:
                {
                    return GABUILD_ConcatenateStringValues(p_LeftValue, p_RightValue);
                }
                default:
                    GABLE_error("Invalid operator type for string-vs-string assignment operation.");
                    return NULL;
            }
        }
        else
        {
            GABLE_error("Invalid righthand value type for assignment operation.");
            return NULL;
        }
    }
    else
    {
        GABLE_error("Invalid lefthand value type for assignment operation.");
        return NULL;
    }
}

// Static Functions - Binary Operations ////////////////////////////////////////////////////////////

static GABUILD_Value* GABUILD_PerformBinaryOperation (const GABUILD_Value* p_LeftValue,
    const GABUILD_Value* p_RightValue, GABUILD_TokenType p_Operator)
{
    // Check the type of the lefthand value.
    if (p_LeftValue->m_Type == GABUILD_VT_NUMBER)
    {
        // Check the type of the righthand value.
        if (p_RightValue->m_Type == GABUILD_VT_NUMBER)
        {
            // Check the operator type and perform the proper operation.
            switch (p_Operator)
            {
                case GABUILD_TOKEN_PLUS:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number + p_RightValue->m_Number);
                case GABUILD_TOKEN_MINUS:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number - p_RightValue->m_Number);
                case GABUILD_TOKEN_MULTIPLY:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number * p_RightValue->m_Number);
                case GABUILD_TOKEN_DIVIDE:
                    if (p_RightValue->m_IntegerPart == 0 && p_RightValue->m_FractionalPart == 0)
                    {
                        GABLE_error("Encountered attempted division by zero.");
                        return NULL;
                    }
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number / p_RightValue->m_Number);
                case GABUILD_TOKEN_MODULO:
                    if (p_RightValue->m_IntegerPart == 0 && p_RightValue->m_FractionalPart == 0)
                    {
                        GABLE_error("Encountered modulo with attempted division by zero.");
                        return NULL;
                    }

                    return GABUILD_CreateNumberValue(fmod(p_LeftValue->m_Number, p_RightValue->m_Number));
                case GABUILD_TOKEN_EXPONENT:
                    return GABUILD_CreateNumberValue(pow(p_LeftValue->m_Number, p_RightValue->m_Number));
                case GABUILD_TOKEN_BITWISE_AND:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_IntegerPart & p_RightValue->m_IntegerPart);
                case GABUILD_TOKEN_BITWISE_OR:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_IntegerPart | p_RightValue->m_IntegerPart);
                case GABUILD_TOKEN_BITWISE_XOR:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_IntegerPart ^ p_RightValue->m_IntegerPart);
                case GABUILD_TOKEN_BITWISE_SHIFT_LEFT:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_IntegerPart << p_RightValue->m_IntegerPart);
                case GABUILD_TOKEN_BITWISE_SHIFT_RIGHT:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_IntegerPart >> p_RightValue->m_IntegerPart);
                case GABUILD_TOKEN_LOGICAL_AND:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number && p_RightValue->m_Number);
                case GABUILD_TOKEN_LOGICAL_OR:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number || p_RightValue->m_Number);
                case GABUILD_TOKEN_COMPARE_EQUAL:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number == p_RightValue->m_Number);
                case GABUILD_TOKEN_COMPARE_NOT_EQUAL:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number != p_RightValue->m_Number);
                case GABUILD_TOKEN_COMPARE_LESS:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number < p_RightValue->m_Number);
                case GABUILD_TOKEN_COMPARE_LESS_EQUAL:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number <= p_RightValue->m_Number);
                case GABUILD_TOKEN_COMPARE_GREATER:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number > p_RightValue->m_Number);
                case GABUILD_TOKEN_COMPARE_GREATER_EQUAL:
                    return GABUILD_CreateNumberValue(p_LeftValue->m_Number >= p_RightValue->m_Number);
                default:
                    GABLE_error("Invalid operator type for number-vs-number binary operation.");
                    return NULL;
            }
        }
        else
        {
            GABLE_error("Invalid righthand value type for binary operation.");
            return NULL;
        }
    }
    else if (p_LeftValue->m_Type == GABUILD_VT_STRING)
    {
        // Check the type of the righthand value.
        if (p_RightValue->m_Type == GABUILD_VT_STRING)
        {
            // Check the operator type and perform the proper operation.
            switch (p_Operator)
            {
                case GABUILD_TOKEN_PLUS:
                {
                    return GABUILD_ConcatenateStringValues(p_LeftValue, p_RightValue);
                }
                default:
                    GABLE_error("Invalid operator type for string-vs-string binary operation.");
                    return NULL;
            }
        }
        else
        {
            GABLE_error("Invalid righthand value type for binary operation.");
            return NULL;
        }
    }
    else
    {
        GABLE_error("Invalid lefthand value type for binary operation.");
        return NULL;
    }
}

// Static Functions - Unary Operations /////////////////////////////////////////////////////////////

static GABUILD_Value* GABUILD_PerformUnaryOperation (const GABUILD_Value* p_Value, GABUILD_TokenType p_Operator)
{
    // Check the type of the value.
    if (p_Value->m_Type == GABUILD_VT_NUMBER)
    {
        // Check the operator type and perform the proper operation.
        switch (p_Operator)
        {
            case GABUILD_TOKEN_PLUS:
                return GABUILD_CreateNumberValue(p_Value->m_Number);
            case GABUILD_TOKEN_MINUS:
                return GABUILD_CreateNumberValue(-p_Value->m_Number);
            case GABUILD_TOKEN_LOGICAL_NOT:
                return GABUILD_CreateNumberValue(!p_Value->m_Number);
            case GABUILD_TOKEN_BITWISE_NOT:
                return GABUILD_CreateNumberValue(~p_Value->m_IntegerPart);
            default:
                GABLE_error("Invalid operator type for number unary operation.");
                return NULL;
        }
    }
    else
    {
        GABLE_error("Invalid value type for unary operation.");
        return NULL;
    }
}

// Static Functions - Evaluation ///////////////////////////////////////////////////////////////////

static GABUILD_Value* GABUILD_EvaluateString (const GABUILD_Syntax* p_SyntaxNode)
{
    GABUILD_Value* l_Value = GABUILD_CreateStringValue(p_SyntaxNode->m_String);
    return l_Value;
}

static GABUILD_Value* GABUILD_EvaluateNumber (const GABUILD_Syntax* p_SyntaxNode)
{
    GABUILD_Value* l_Value = GABUILD_CreateNumberValue(p_SyntaxNode->m_Number);
    return l_Value;
}

static GABUILD_Value* GABUILD_EvaluateBinaryExpression (const GABUILD_Syntax* p_SyntaxNode)
{
    // Evaluate the left-hand side of the expression.
    GABUILD_Value* l_LeftValue = GABUILD_Evaluate(p_SyntaxNode->m_LeftExpr);
    if (l_LeftValue == NULL)
    {
        return NULL;
    }

    // Evaluate the right-hand side of the expression.
    GABUILD_Value* l_RightValue = GABUILD_Evaluate(p_SyntaxNode->m_RightExpr);
    if (l_RightValue == NULL)
    {
        GABUILD_DestroyValue(l_LeftValue);
        return NULL;
    }

    // Perform the binary operation.
    GABUILD_Value* l_Result = GABUILD_PerformBinaryOperation(l_LeftValue, l_RightValue, p_SyntaxNode->m_Operator);
    GABUILD_DestroyValue(l_LeftValue);
    GABUILD_DestroyValue(l_RightValue);

    return l_Result;
}

static GABUILD_Value* GABUILD_EvaluateUnaryExpression (const GABUILD_Syntax* p_SyntaxNode)
{
    // Evaluate the operand of the expression.
    GABUILD_Value* l_OperandValue = GABUILD_Evaluate(p_SyntaxNode->m_RightExpr);
    if (l_OperandValue == NULL)
    {
        return NULL;
    }

    // Perform the unary operation.
    GABUILD_Value* l_Result = GABUILD_PerformUnaryOperation(l_OperandValue, p_SyntaxNode->m_Operator);
    GABUILD_DestroyValue(l_OperandValue);

    return l_Result;
}

static GABUILD_Value* GABUILD_EvaluateIdentifier (const GABUILD_Syntax* p_SyntaxNode)
{
    // Check if the identifier is a reference to a defined value.
    for (Index i = 0; i < s_Builder.m_DefineCount; ++i)
    {
        if (strcmp(s_Builder.m_DefineKeys[i], p_SyntaxNode->m_String) == 0)
        {
            return GABUILD_CopyValue(s_Builder.m_DefineValues[i]);
        }
    }

    // The identifier must be a reference to a label.
    GABUILD_Label* l_Label = NULL;

    // Find the label by name.
    for (Index i = 0; i < s_Builder.m_LabelCount; ++i)
    {
        if (strcmp(s_Builder.m_Labels[i].m_Name, p_SyntaxNode->m_String) == 0)
        {
            l_Label = &s_Builder.m_Labels[i];
            break;
        }
    }

    // Was the label found?
    if (l_Label == NULL)
    {
        // Create a new unresolved label.
        GABUILD_ResizeLabelsArray();

        // Allocate memory for the label's string name.
        Size  l_LabelStrlen = strlen(p_SyntaxNode->m_String);
        Char* l_LabelName = GABLE_calloc(l_LabelStrlen + 1, Char);
        GABLE_pexpect(l_LabelName != NULL, "Failed to allocate memory for label name string");
        strncpy(l_LabelName, p_SyntaxNode->m_String, l_LabelStrlen);

        // Allocate memory for the label's references array.
        Uint16* l_LabelReferences = GABLE_calloc(GABUILD_BUILDER_INITIAL_CAPACITY, Uint16);
        GABLE_pexpect(l_LabelReferences != NULL, "Failed to allocate memory for label references array");

        // Point to the next available label.
        l_Label = &s_Builder.m_Labels[s_Builder.m_LabelCount++];
        l_Label->m_Name = l_LabelName;
        l_Label->m_References = l_LabelReferences;
        l_Label->m_ReferenceCount = 0;
        l_Label->m_ReferenceCapacity = GABUILD_BUILDER_INITIAL_CAPACITY;
        l_Label->m_Address = 0;
        l_Label->m_Resolved = false;
    }

    // Add the current output size as a reference to the label.
    GABUILD_ResizeLabelReferences(l_Label);
    l_Label->m_References[l_Label->m_ReferenceCount++] = s_Builder.m_OutputSize;

    // Return a number value with the address of the label if it has been resolved.
    // Otherwise, return a number value with the value 0.
    return (l_Label->m_Resolved == true) ?
        GABUILD_CreateNumberValue((Float64) l_Label->m_Address) :
        GABUILD_CreateNumberValue(0);
}

static GABUILD_Value* GABUILD_EvaluateLabel (const GABUILD_Syntax* p_SyntaxNode)
{
    // Check if the label has already been defined.
    GABUILD_Label* l_Label = NULL;
    for (Index i = 0; i < s_Builder.m_LabelCount; ++i)
    {
        if (strcmp(s_Builder.m_Labels[i].m_Name, p_SyntaxNode->m_String) == 0)
        {
            l_Label = &s_Builder.m_Labels[i];
            break;
        }
    }

    // Has the label been defined?
    if (l_Label == NULL)
    {
        // Resize the labels array.
        GABUILD_ResizeLabelsArray();

        // Allocate memory for the label's string name.
        Size  l_LabelStrlen = strlen(p_SyntaxNode->m_String);
        Char* l_LabelName = GABLE_calloc(l_LabelStrlen + 1, Char);
        GABLE_pexpect(l_LabelName != NULL, "Failed to allocate memory for label name string");

        // Allocate memory for the label's references array.
        Uint16* l_LabelReferences = GABLE_calloc(GABUILD_BUILDER_INITIAL_CAPACITY, Uint16);
        GABLE_pexpect(l_LabelReferences != NULL, "Failed to allocate memory for label references array");

        // Point to the next available label.
        l_Label = &s_Builder.m_Labels[s_Builder.m_LabelCount++];
        l_Label->m_Name = l_LabelName;
        l_Label->m_References = l_LabelReferences;
        l_Label->m_ReferenceCount = 0;
        l_Label->m_ReferenceCapacity = GABUILD_BUILDER_INITIAL_CAPACITY;
        
        // Label is resolved to the current output size.
        l_Label->m_Address = s_Builder.m_OutputSize;
        l_Label->m_Resolved = true;
    }
    else
    {
        // Set the label's resolved address to the current output size.
        l_Label->m_Address = s_Builder.m_OutputSize;

        // Has the label not been resolved?
        if (l_Label->m_Resolved == false)
        {
            // Resolve it now.
            l_Label->m_Resolved = true;
            for (Index i = 0; i < l_Label->m_ReferenceCount; ++i)
            {
                Uint16 l_Reference = l_Label->m_References[i];
                s_Builder.m_Output[l_Reference] = (Uint8) (l_Label->m_Address & 0xFF);
                s_Builder.m_Output[l_Reference + 1] = (Uint8) ((l_Label->m_Address >> 8) & 0xFF);
            }
        }
    }

    return GABUILD_CreateVoidValue();
}

static GABUILD_Value* GABUILD_EvaluateData (const GABUILD_Syntax* p_SyntaxNode)
{
    // The type of data being defined is hinted in the syntax node's stored keyword type.
    switch (p_SyntaxNode->m_KeywordType)
    {
        case GABUILD_KT_DB: // Define Bytes
        {
            // Evaluate each expression in the data syntax node.
            for (Index i = 0; i < p_SyntaxNode->m_BodySize; ++i)
            {
                GABUILD_Value* l_Value = GABUILD_Evaluate(p_SyntaxNode->m_Body[i]);
                if (l_Value == NULL)
                {
                    return NULL;
                }

                // Check the type of the value. It can be a number or a string.
                if (l_Value->m_Type == GABUILD_VT_NUMBER)
                {
                    if (l_Value->m_IntegerPart > 0xFF)
                    {
                        GABLE_warn("Value '%lu' is too large to fit in a byte, and will be truncated.", l_Value->m_IntegerPart);
                    }

                    if (GABUILD_DefineByte(l_Value->m_IntegerPart & 0xFF) == false)
                    {
                        GABUILD_DestroyValue(l_Value);
                        return NULL;
                    }
                }
                else if (l_Value->m_Type == GABUILD_VT_STRING)
                {
                    if (GABUILD_DefineStringASCII(l_Value->m_String) == false)
                    {
                        GABUILD_DestroyValue(l_Value);
                        return NULL;
                    }
                }
                else
                {
                    GABLE_error("Unexpected value type in 'db' statement.");
                    GABUILD_DestroyValue(l_Value);
                    return NULL;
                }

                GABUILD_DestroyValue(l_Value);
            }
            break;
        }
        case GABUILD_KT_DW: // Define Words
        {
            // Evaluate each expression in the data syntax node.
            for (Index i = 0; i < p_SyntaxNode->m_BodySize; ++i)
            {
                GABUILD_Value* l_Value = GABUILD_Evaluate(p_SyntaxNode->m_Body[i]);
                if (l_Value == NULL)
                {
                    return NULL;
                }

                // Check the type of the value. It must be a number.
                if (l_Value->m_Type == GABUILD_VT_NUMBER)
                {
                    if (l_Value->m_IntegerPart > 0xFFFF)
                    {
                        GABLE_warn("Value '%lu' is too large to fit in a word, and will be truncated.", l_Value->m_IntegerPart);
                    }

                    if (GABUILD_DefineWord(l_Value->m_IntegerPart & 0xFFFF) == false)
                    {
                        GABUILD_DestroyValue(l_Value);
                        return NULL;
                    }
                }
                else
                {
                    GABLE_error("Unexpected value type in 'dw' statement.");
                    GABUILD_DestroyValue(l_Value);
                    return NULL;
                }

                GABUILD_DestroyValue(l_Value);
            }
            break;
        }
        case GABUILD_KT_DL: // Define Longs
        {
            // Evaluate each expression in the data syntax node.
            for (Index i = 0; i < p_SyntaxNode->m_BodySize; ++i)
            {
                GABUILD_Value* l_Value = GABUILD_Evaluate(p_SyntaxNode->m_Body[i]);
                if (l_Value == NULL)
                {
                    return NULL;
                }

                // Check the type of the value. It must be a number.
                if (l_Value->m_Type == GABUILD_VT_NUMBER)
                {
                    if (GABUILD_DefineLong(l_Value->m_IntegerPart & 0xFFFFFFFF) == false)
                    {
                        GABUILD_DestroyValue(l_Value);
                        return NULL;
                    }
                }
                else
                {
                    GABLE_error("Unexpected value type in 'dl' statement.");
                    GABUILD_DestroyValue(l_Value);
                    return NULL;
                }

                GABUILD_DestroyValue(l_Value);
            }
            break;
        }
        case GABUILD_KT_DS: // Define Sequence (of bytes)
        {
            // The `ds` statement requires a count expression followed by one or more data expressions.
            // It defines the sequence of bytes after the count expression a certain number of times
            // equal to the value of the count expression.

            // Evaluate the count expression.
            GABUILD_Value* l_CountValue = GABUILD_Evaluate(p_SyntaxNode->m_CountExpr);
            if (l_CountValue == NULL)
            {
                return NULL;
            }

            // Check the type of the value. It must be a number.
            if (l_CountValue->m_Type != GABUILD_VT_NUMBER)
            {
                GABLE_error("Unexpected value type for count expression in 'ds' statement.");
                GABUILD_DestroyValue(l_CountValue);
                return NULL;
            }
            
            // Evaluate each expression in the data syntax node. Repeat the sequence of bytes
            // as many times as the value of the count expression.
            Uint64 l_Count = l_CountValue->m_IntegerPart;
            GABUILD_DestroyValue(l_CountValue);

            while (l_Count > 0)
            {
                for (Index i = 0; i < p_SyntaxNode->m_BodySize; ++i)
                {
                    GABUILD_Value* l_Value = GABUILD_Evaluate(p_SyntaxNode->m_Body[i]);
                    if (l_Value == NULL)
                    {
                        return NULL;
                    }

                    // Check the type of the value. It can be a number or a string.
                    if (l_Value->m_Type == GABUILD_VT_NUMBER)
                    {
                        if (l_Value->m_IntegerPart > 0xFF)
                        {
                            GABLE_warn("Value '%lu' is too large to fit in a byte, and will be truncated.", l_Value->m_IntegerPart);
                        }

                        if (GABUILD_DefineByte(l_Value->m_IntegerPart & 0xFF) == false)
                        {
                            GABUILD_DestroyValue(l_Value);
                            return NULL;
                        }
                    }
                    else if (l_Value->m_Type == GABUILD_VT_STRING)
                    {
                        if (GABUILD_DefineStringASCII(l_Value->m_String) == false)
                        {
                            GABUILD_DestroyValue(l_Value);
                            return NULL;
                        }
                    }
                    else
                    {
                        GABLE_error("Unexpected value type in 'ds' statement.");
                        GABUILD_DestroyValue(l_Value);
                        return NULL;
                    }

                    GABUILD_DestroyValue(l_Value);
                }

                l_Count--;
            }

            break;
        }
        default:
            GABLE_error("Unexpected keyword type for data syntax node.");
            return NULL;
    }

    return GABUILD_CreateVoidValue();
}

static GABUILD_Value* GABUILD_EvaluateDefine (const GABUILD_Syntax* p_SyntaxNode)
{
    // Evaluate the value of the define statement.
    GABUILD_Value* l_Value = GABUILD_Evaluate(p_SyntaxNode->m_RightExpr);
    if (l_Value == NULL)
    {
        return NULL;
    }

    // Check to see if the define already exists.
    GABUILD_Value** l_ExistingValue = NULL;
    for (Index i = 0; i < s_Builder.m_DefineCount; ++i)
    {
        if (strcmp(s_Builder.m_DefineKeys[i], p_SyntaxNode->m_String) == 0)
        {
            l_ExistingValue = &s_Builder.m_DefineValues[i];
            break;
        }
    }

    // If the define already exists, then perform an assignment operation.
    if (l_ExistingValue != NULL)
    {
        // Perform the assignment operation and store the result.
        GABUILD_Value* l_Result = GABUILD_PerformAssignmentOperation(*l_ExistingValue, l_Value, p_SyntaxNode->m_Operator);
        if (l_Result == NULL)
        {
            GABUILD_DestroyValue(l_Value);
            return NULL;
        }

        // Replace the existing value with the result of the assignment operation.
        GABUILD_DestroyValue(*l_ExistingValue);
        *l_ExistingValue = l_Result;
    }
    else
    {
        // Resize the defines arrays.
        GABUILD_ResizeDefinesArrays();

        // Allocate memory for the define key string.
        Size  l_DefineStrlen = strlen(p_SyntaxNode->m_String);
        Char* l_DefineKey = GABLE_calloc(l_DefineStrlen + 1, Char);
        GABLE_pexpect(l_DefineKey != NULL, "Failed to allocate memory for define key string");
        strncpy(l_DefineKey, p_SyntaxNode->m_String, l_DefineStrlen);

        // Point to the next available define.
        s_Builder.m_DefineKeys[s_Builder.m_DefineCount] = l_DefineKey;
        s_Builder.m_DefineValues[s_Builder.m_DefineCount] = GABUILD_CopyValue(l_Value);
        s_Builder.m_DefineCount++;
    }

    GABUILD_DestroyValue(l_Value);
    return GABUILD_CreateVoidValue();
}

static GABUILD_Value* GABUILD_EvaluateMacroDefinition (const GABUILD_Syntax* p_SyntaxNode)
{
    // Check if the macro has already been defined.
    for (Index i = 0; i < s_Builder.m_MacroCount; ++i)
    {
        if (strcmp(s_Builder.m_Macros[i].m_Name, p_SyntaxNode->m_String) == 0)
        {
            GABLE_error("Macro '%s' has already been defined.", p_SyntaxNode->m_String);
            return NULL;
        }
    }

    // Resize the macros array.
    GABUILD_ResizeMacrosArray();

    // Copy the name of the macro.
    Size  l_NameStrlen = strlen(p_SyntaxNode->m_String);
    Char* l_Name = GABLE_calloc(l_NameStrlen + 1, Char);
    GABLE_pexpect(l_Name != NULL, "Failed to allocate memory for macro name string");
    strncpy(l_Name, p_SyntaxNode->m_String, l_NameStrlen);

    // Resize the macros array.
    GABUILD_Macro* l_Macro = &s_Builder.m_Macros[s_Builder.m_MacroCount++];
    l_Macro->m_Name = l_Name;
    l_Macro->m_Block = GABUILD_CopySyntax(p_SyntaxNode->m_LeftExpr);

    return GABUILD_CreateVoidValue();
}

static GABUILD_Value* GABUILD_EvaluateMacroCall (const GABUILD_Syntax* p_SyntaxNode)
{
    // Find the macro by name.
    GABUILD_Macro* l_Macro = NULL;
    for (Index i = 0; i < s_Builder.m_MacroCount; ++i)
    {
        if (strcmp(s_Builder.m_Macros[i].m_Name, p_SyntaxNode->m_String) == 0)
        {
            l_Macro = &s_Builder.m_Macros[i];
            break;
        }
    }

    // Was the macro found?
    if (l_Macro == NULL)
    {
        GABLE_error("Macro '%s' was not found.", p_SyntaxNode->m_String);
        return NULL;
    }

    // Check the macro call stack. Is it full?
    if (s_Builder.m_MacroCallStackIndex >= GABUILD_BUILDER_CALL_STACK_SIZE)
    {
        GABLE_error("Macro call stack overflowed.");
        return NULL;
    }

    // Set up the macro call context.
    GABUILD_MacroCall* l_Call = GABUILD_CreateMacroCall(p_SyntaxNode->m_BodySize);
    s_Builder.m_MacroCallStack[s_Builder.m_MacroCallStackIndex++] = l_Call;

    // Evaluate the macro call's arguments.
    for (Index i = 0; i < p_SyntaxNode->m_BodySize; ++i)
    {
        GABUILD_Value* l_Value = GABUILD_Evaluate(p_SyntaxNode->m_Body[i]);
        if (l_Value == NULL)
        {
            GABUILD_DestroyMacroCall(l_Call);
            s_Builder.m_MacroCallStack[--s_Builder.m_MacroCallStackIndex] = NULL;
            return NULL;
        }

        l_Call->m_Arguments[i] = l_Value;
    }

    // Evaluate the macro block.
    GABUILD_Value* l_Result = GABUILD_EvaluateBlock(l_Macro->m_Block);
    if (l_Result == NULL)
    {
        GABUILD_DestroyMacroCall(l_Call);
        s_Builder.m_MacroCallStack[--s_Builder.m_MacroCallStackIndex] = NULL;
        return NULL;
    }

    // Clean up the macro call context.
    GABUILD_DestroyMacroCall(l_Call);
    s_Builder.m_MacroCallStack[--s_Builder.m_MacroCallStackIndex] = NULL;
    
    return l_Result;
}

GABUILD_Value* GABUILD_EvaluateNARGExpression (const GABUILD_Syntax* p_Syntax)
{
    // Make sure the macro call stack is not empty.
    if (s_Builder.m_MacroCallStackIndex == 0)
    {
        GABLE_error("NARG syntax outside of a macro call.");
        return NULL;
    }

    // Get the macro call context at the top of the stack.
    GABUILD_MacroCall* l_Call = s_Builder.m_MacroCallStack[s_Builder.m_MacroCallStackIndex - 1];

    // Return a number value with the number of arguments in the macro call.
    return GABUILD_CreateNumberValue((Float64) l_Call->m_ArgumentCount);
}

GABUILD_Value* GABUILD_EvaluateMacroArgument (const GABUILD_Syntax* p_Syntax)
{
    // Make sure the macro call stack is not empty.
    if (s_Builder.m_MacroCallStackIndex == 0)
    {
        GABLE_error("Macro argument syntax outside of a macro call.");
        return NULL;
    }

    // Get the macro call context at the top of the stack.
    GABUILD_MacroCall* l_Call = s_Builder.m_MacroCallStack[s_Builder.m_MacroCallStackIndex - 1];

    // Check the index of the argument. Macro arguments are 1-indexed ('\1' is the first argument).
    Index l_ArgIndex = (Index) p_Syntax->m_Number;

    // Offset the argument index by the shift offset.
    l_ArgIndex += l_Call->m_ArgumentOffset;

    if (l_ArgIndex < 1 || l_ArgIndex > l_Call->m_ArgumentCount)
    {
        GABLE_error("Macro argument index %zu out of range.", l_ArgIndex);
        return NULL;
    }

    // Return a copy of the argument value.
    return GABUILD_CopyValue(l_Call->m_Arguments[l_ArgIndex - 1]);
}

GABUILD_Value* GABUILD_EvaluateShiftStatement (const GABUILD_Syntax* p_SyntaxNode)
{
    // Make sure the macro call stack is not empty.
    if (s_Builder.m_MacroCallStackIndex == 0)
    {
        GABLE_error("Shift syntax outside of a macro call.");
        return NULL;
    }

    // Get the macro call context at the top of the stack.
    GABUILD_MacroCall* l_Call = s_Builder.m_MacroCallStack[s_Builder.m_MacroCallStackIndex - 1];

    // Evaluate the shift expression.
    GABUILD_Value* l_ShiftValue = GABUILD_Evaluate(p_SyntaxNode->m_CountExpr);
    if (l_ShiftValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a number.
    if (l_ShiftValue->m_Type != GABUILD_VT_NUMBER)
    {
        GABLE_error("Unexpected value type for shift expression in 'shift' statement.");
        GABUILD_DestroyValue(l_ShiftValue);
        return NULL;
    }

    // Shift the argument offset by the value of the shift expression.
    l_Call->m_ArgumentOffset += (Index) l_ShiftValue->m_IntegerPart;
    GABUILD_DestroyValue(l_ShiftValue);

    return GABUILD_CreateVoidValue();
}

GABUILD_Value* GABUILD_EvaluateRepeatStatement (const GABUILD_Syntax* p_SyntaxNode)
{
    // Evaluate the count expression.
    GABUILD_Value* l_CountValue = GABUILD_Evaluate(p_SyntaxNode->m_CountExpr);
    if (l_CountValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a number.
    if (l_CountValue->m_Type != GABUILD_VT_NUMBER)
    {
        GABLE_error("Unexpected value type for count expression in 'repeat' statement.");
        GABUILD_DestroyValue(l_CountValue);
        return NULL;
    }

    // Evaluate the block expression.
    GABUILD_Value* l_Result = NULL;
    Uint64 l_Count = l_CountValue->m_IntegerPart;
    GABUILD_DestroyValue(l_CountValue);

    while (l_Count > 0)
    {
        l_Result = GABUILD_EvaluateBlock(p_SyntaxNode->m_LeftExpr);
        if (l_Result == NULL)
        {
            return NULL;
        }

        GABUILD_DestroyValue(l_Result);
        l_Count--;
    }

    return GABUILD_CreateVoidValue();
}

GABUILD_Value* GABUILD_EvaluateIfStatement (const GABUILD_Syntax* p_SyntaxNode)
{
    // Evaluate the condition expression.
    GABUILD_Value* l_ConditionValue = GABUILD_Evaluate(p_SyntaxNode->m_CondExpr);
    if (l_ConditionValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a number.
    if (l_ConditionValue->m_Type != GABUILD_VT_NUMBER)
    {
        GABLE_error("Unexpected value type for condition expression in 'if' statement.");
        GABUILD_DestroyValue(l_ConditionValue);
        return NULL;
    }

    // Evaluate the block expression.
    GABUILD_Value* l_Result = NULL;
    if (l_ConditionValue->m_Number != 0)
    {
        l_Result = GABUILD_EvaluateBlock(p_SyntaxNode->m_LeftExpr);
    }
    else if (p_SyntaxNode->m_RightExpr != NULL)
    {
        l_Result = GABUILD_EvaluateBlock(p_SyntaxNode->m_RightExpr);
    }

    GABUILD_DestroyValue(l_ConditionValue);
    return l_Result;
}

GABUILD_Value* GABUILD_EvaluateIncludeStatement (const GABUILD_Syntax* p_SyntaxNode)
{
    // Evaluate the string expression.
    GABUILD_Value* l_StringValue = GABUILD_Evaluate(p_SyntaxNode->m_LeftExpr);
    if (l_StringValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a string.
    if (l_StringValue->m_Type != GABUILD_VT_STRING)
    {
        GABLE_error("Unexpected value type for string expression in 'include' statement.");
        GABUILD_DestroyValue(l_StringValue);
        return NULL;
    }
    
    // Prepare the lexer for the new file.
    GABUILD_ResetLexer();
    if (GABUILD_LexFile(l_StringValue->m_String) == false)
    {
        GABUILD_DestroyValue(l_StringValue);
        return NULL;
    }

    // Parse the lexed tokens.
    GABUILD_Syntax* l_Syntax = GABUILD_CreateSyntax(GABUILD_ST_BLOCK, GABUILD_PeekToken(0));
    if (GABUILD_Parse(l_Syntax) == false)
    {
        GABUILD_DestroySyntax(l_Syntax);
        GABUILD_DestroyValue(l_StringValue);
        return NULL;
    }

    // Evaluate the parsed syntax.
    GABUILD_Value* l_Result = GABUILD_Evaluate(l_Syntax);
    if (l_Result == NULL)
    {
        GABUILD_DestroySyntax(l_Syntax);
        GABUILD_DestroyValue(l_StringValue);
        return NULL;
    }

    GABUILD_DestroySyntax(l_Syntax);
    GABUILD_DestroyValue(l_Result);
    GABUILD_DestroyValue(l_StringValue);
    return GABUILD_CreateVoidValue();
}

GABUILD_Value* GABUILD_EvaluateIncbinStatement (const GABUILD_Syntax* p_SyntaxNode)
{
    // Evaluate the string expression.
    GABUILD_Value* l_StringValue = GABUILD_Evaluate(p_SyntaxNode->m_LeftExpr);
    if (l_StringValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a string.
    if (l_StringValue->m_Type != GABUILD_VT_STRING)
    {
        GABLE_error("Unexpected value type for string expression in 'incbin' statement.");
        GABUILD_DestroyValue(l_StringValue);
        return NULL;
    }

    // Load the file.
    if (GABUILD_DefineBinaryFile(l_StringValue->m_String) == false)
    {
        GABUILD_DestroyValue(l_StringValue);
        return NULL;
    }

    GABUILD_DestroyValue(l_StringValue);
    return GABUILD_CreateVoidValue();
}

GABUILD_Value* GABUILD_EvaluateBlock (const GABUILD_Syntax* p_SyntaxNode)
{
    // Create a new value to hold the result of the block.
    GABUILD_Value* l_Result = GABUILD_CreateVoidValue();

    // Evaluate each statement in the block.
    for (Index i = 0; i < p_SyntaxNode->m_BodySize; ++i)
    {
        GABUILD_Value* l_Value = GABUILD_Evaluate(p_SyntaxNode->m_Body[i]);
        GABUILD_DestroyValue(l_Result);

        if (l_Value == NULL)
        {
            return NULL;
        }

        l_Result = l_Value;
    }

    return l_Result;
}

GABUILD_Value* GABUILD_Evaluate (const GABUILD_Syntax* p_SyntaxNode)
{
    GABUILD_Value* l_Result = NULL;
    switch (p_SyntaxNode->m_Type)
    {   
        case GABUILD_ST_STRING:
            l_Result = GABUILD_EvaluateString(p_SyntaxNode);
            break;

        case GABUILD_ST_NUMBER:
            l_Result = GABUILD_EvaluateNumber(p_SyntaxNode);
            break;

        case GABUILD_ST_BINARY_EXP:
            l_Result = GABUILD_EvaluateBinaryExpression(p_SyntaxNode);
            break;

        case GABUILD_ST_UNARY_EXP:
            l_Result = GABUILD_EvaluateUnaryExpression(p_SyntaxNode);
            break;

        case GABUILD_ST_IDENTIFIER:
            l_Result = GABUILD_EvaluateIdentifier(p_SyntaxNode);
            break;

        case GABUILD_ST_LABEL:
            l_Result = GABUILD_EvaluateLabel(p_SyntaxNode);
            break;

        case GABUILD_ST_DATA:
            l_Result = GABUILD_EvaluateData(p_SyntaxNode);
            break;

        case GABUILD_ST_DEF:
            l_Result = GABUILD_EvaluateDefine(p_SyntaxNode);
            break;

        case GABUILD_ST_MACRO:
            l_Result = GABUILD_EvaluateMacroDefinition(p_SyntaxNode);
            break;

        case GABUILD_ST_MACRO_CALL:
            l_Result = GABUILD_EvaluateMacroCall(p_SyntaxNode);
            break;

        case GABUILD_ST_NARG:
            l_Result = GABUILD_EvaluateNARGExpression(p_SyntaxNode);
            break;

        case GABUILD_ST_ARGUMENT:
            l_Result = GABUILD_EvaluateMacroArgument(p_SyntaxNode);
            break;

        case GABUILD_ST_SHIFT:
            l_Result = GABUILD_EvaluateShiftStatement(p_SyntaxNode);
            break;

        case GABUILD_ST_REPEAT:
            l_Result = GABUILD_EvaluateRepeatStatement(p_SyntaxNode);
            break;

        case GABUILD_ST_IF:
            l_Result = GABUILD_EvaluateIfStatement(p_SyntaxNode);
            break;

        case GABUILD_ST_BLOCK:
            l_Result = GABUILD_EvaluateBlock(p_SyntaxNode);
            break;

        case GABUILD_ST_INCLUDE:
            l_Result = GABUILD_EvaluateIncludeStatement(p_SyntaxNode);
            break;

        case GABUILD_ST_INCBIN:
            l_Result = GABUILD_EvaluateIncbinStatement(p_SyntaxNode);
            break;

        default:
            GABLE_error("Unexpected syntax node type: %d.", p_SyntaxNode->m_Type);
            break;
    }

    if (l_Result == NULL)
    {
        fprintf(stderr, " - In file '%s:%zu:%zu.\n",
            p_SyntaxNode->m_Token.m_SourceFile,
            p_SyntaxNode->m_Token.m_Line,
            p_SyntaxNode->m_Token.m_Column
        );
    }

    return l_Result;
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void GABUILD_InitBuilder ()
{
    // Initialize labels.
    s_Builder.m_Labels = GABLE_malloc(GABUILD_BUILDER_INITIAL_CAPACITY, GABUILD_Label);
    GABLE_pexpect(s_Builder.m_Labels != NULL, "Failed to allocate memory for the builder's address labels array");
    s_Builder.m_LabelCapacity = GABUILD_BUILDER_INITIAL_CAPACITY;
    s_Builder.m_LabelCount = 0;

    // Initialize macros.
    s_Builder.m_Macros = GABLE_malloc(GABUILD_BUILDER_INITIAL_CAPACITY, GABUILD_Macro);
    GABLE_pexpect(s_Builder.m_Macros != NULL, "Failed to allocate memory for the builder's macros array");
    s_Builder.m_MacroCapacity = GABUILD_BUILDER_INITIAL_CAPACITY;
    s_Builder.m_MacroCount = 0;

    // Initialize defines.
    s_Builder.m_DefineValues = GABLE_malloc(GABUILD_BUILDER_INITIAL_CAPACITY, GABUILD_Value*);
    GABLE_pexpect(s_Builder.m_DefineValues != NULL, "Failed to allocate memory for the builder's define values array");
    s_Builder.m_DefineKeys = GABLE_malloc(GABUILD_BUILDER_INITIAL_CAPACITY, Char*);
    GABLE_pexpect(s_Builder.m_DefineKeys != NULL, "Failed to allocate memory for the builder's define keys array");
    s_Builder.m_DefineCapacity = GABUILD_BUILDER_INITIAL_CAPACITY;
    s_Builder.m_DefineCount = 0;
}

void GABUILD_ShutdownBuilder ()
{
    // Free defines.
    for (Size i = 0; i < s_Builder.m_DefineCount; ++i)
    {
        GABUILD_DestroyValue(s_Builder.m_DefineValues[i]);
        GABLE_free(s_Builder.m_DefineKeys[i]);
    }
    GABLE_free(s_Builder.m_DefineValues);
    GABLE_free(s_Builder.m_DefineKeys);

    // Free macros.
    for (Size i = 0; i < s_Builder.m_MacroCount; ++i)
    {
        GABLE_free(s_Builder.m_Macros[i].m_Name);
        GABUILD_DestroySyntax(s_Builder.m_Macros[i].m_Block);
    }
    GABLE_free(s_Builder.m_Macros);

    // Free labels.
    for (Size i = 0; i < s_Builder.m_LabelCount; ++i)
    {
        GABLE_free(s_Builder.m_Labels[i].m_Name);
        GABLE_free(s_Builder.m_Labels[i].m_References);
    }
    GABLE_free(s_Builder.m_Labels);

    // Free the result value.
    GABUILD_DestroyValue(s_Builder.m_Result);
}

Bool GABUILD_Build (const GABUILD_Syntax* p_SyntaxNode)
{
    // Evaluate the syntax node.
    s_Builder.m_Result = GABUILD_Evaluate(p_SyntaxNode);
    return s_Builder.m_Result != NULL;
}

Bool GABUILD_SaveBinary (const char* p_OutputPath)
{
    GABLE_assert(p_OutputPath);

    // Ensure the path is not blank.
    if (p_OutputPath[0] == '\0')
    {
        GABLE_error("Output path is blank.");
        return false;
    }

    // Attempt to open the output file for writing.
    FILE* l_File = fopen(p_OutputPath, "wb");
    if (l_File == NULL)
    {
        GABLE_perror("Failed to open output file '%s' for writing", p_OutputPath);
        return false;
    }

    // Write the output buffer to the file.
    Size l_BytesWritten = fwrite(s_Builder.m_Output, sizeof(Uint8), s_Builder.m_OutputSize, l_File);
    if (l_BytesWritten != s_Builder.m_OutputSize || ferror(l_File))
    {
        GABLE_perror("Failed to write output to file '%s'", p_OutputPath);
        fclose(l_File);
        return false;
    }

    // Close the file.
    fclose(l_File);
    return true;
}
