/**
 * @file     GABUILD/Lexer.c
 */

#include <GABUILD/Lexer.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define GABUILD_LEXER_CAPACITY 16

// Lexer Context ///////////////////////////////////////////////////////////////////////////////////

static struct
{
    Char**                  m_IncludeFiles;
    Count                   m_IncludeFileCount;
    Count                   m_IncludeFileCapacity;

    GABUILD_Token*          m_Tokens;
    Count                   m_TokenCount;
    Count                   m_TokenCapacity;
    Count                   m_TokenPointer;

    const Char*             m_CurrentFile;
    Count                   m_CurrentLine;
    Count                   m_CurrentColumn;

    int                     m_Char;
} s_Lexer = {
    .m_IncludeFiles         = NULL,
    .m_IncludeFileCount     = 0,
    .m_IncludeFileCapacity  = 0,

    .m_Tokens               = NULL,
    .m_TokenCount           = 0,
    .m_TokenCapacity        = 0,
    .m_TokenPointer         = 0,

    .m_CurrentFile          = NULL,
    .m_CurrentLine          = 0,
    .m_CurrentColumn        = 0,

    .m_Char                 = 0
};

// Static Functions - Include File Management //////////////////////////////////////////////////////

static void GABUILD_ResizeIncludeFiles ()
{
    // Only resize if the capacity is about to be reached.
    if (s_Lexer.m_IncludeFileCount + 1 >= s_Lexer.m_IncludeFileCapacity)
    {

        // Calculate the new capacity, then reallocate the array with the new capacity.
        Size l_NewCapacity          = s_Lexer.m_IncludeFileCapacity * 2;
        Char** l_NewIncludeFiles    = GABLE_realloc(s_Lexer.m_IncludeFiles, l_NewCapacity, Char*);
        GABLE_pexpect(l_NewIncludeFiles, "Failed to resize include files array");

        // Update the include files array and capacity.
        s_Lexer.m_IncludeFiles          = l_NewIncludeFiles;
        s_Lexer.m_IncludeFileCapacity   = l_NewCapacity;

    }
}

static Bool GABUILD_AddIncludeFile (const Char* p_Relative, Char** p_Absolute)
{
    // Resolve `p_Relative` to an absolute path.
    Char* l_Absolute = realpath(p_Relative, NULL);
    if (l_Absolute == NULL)
    {
        if (errno == ENOENT)
        {
            GABLE_error("Include file '%s' not found.", p_Relative);
        }
        else
        {
            GABLE_perror("Failed to resolve relative filename '%s'", p_Relative);
        }

        return false;
    }

    // Check if the include file has already been included.
    for (Index i = 0; i < s_Lexer.m_IncludeFileCount; ++i)
    {
        if (strncmp(s_Lexer.m_IncludeFiles[i], l_Absolute, PATH_MAX) == 0)
        {
            GABLE_free(l_Absolute);
            return true;
        }
    }

    // Resize the include files array, then add the new include file.
    GABUILD_ResizeIncludeFiles();
    s_Lexer.m_IncludeFiles[s_Lexer.m_IncludeFileCount++] = l_Absolute;

    // Supply the absolute path to the caller, then return success.
    *p_Absolute = l_Absolute;
    return true;
}

static void GABUILD_FreeIncludeFiles ()
{
    // Free each include file, then free the include files array.
    for (Index i = 0; i < s_Lexer.m_IncludeFileCount; ++i)
    {
        GABLE_free(s_Lexer.m_IncludeFiles[i]);
    }

    GABLE_free(s_Lexer.m_IncludeFiles);
    s_Lexer.m_IncludeFiles         = NULL;
    s_Lexer.m_IncludeFileCount     = 0;
    s_Lexer.m_IncludeFileCapacity  = 0;
}

// Static Functions - Token Collection and Management //////////////////////////////////////////////

static void GABUILD_ResizeTokens ()
{
    // Only resize if the capacity is about to be reached.
    if (s_Lexer.m_TokenCount + 1 >= s_Lexer.m_TokenCapacity)
    {

        // Calculate the new capacity, then reallocate the array with the new capacity.
        Size l_NewCapacity          = s_Lexer.m_TokenCapacity * 2;
        GABUILD_Token* l_NewTokens  = GABLE_realloc(s_Lexer.m_Tokens, l_NewCapacity, GABUILD_Token);
        GABLE_pexpect(l_NewTokens, "Failed to resize tokens array");

        // Zero out the new tokens.
        memset(
            l_NewTokens + s_Lexer.m_TokenCapacity,
            0x00,
            (l_NewCapacity - s_Lexer.m_TokenCapacity) * sizeof(GABUILD_Token)
        );

        // Update the tokens array and capacity.
        s_Lexer.m_Tokens         = l_NewTokens;
        s_Lexer.m_TokenCapacity  = l_NewCapacity;

    }
}

static Bool GABUILD_InsertToken (GABUILD_TokenType p_Type, const Char* p_Lexeme)
{
    GABLE_assert(p_Lexeme != NULL);

    // Resize the tokens array, then add the new token.
    GABUILD_ResizeTokens();
    GABUILD_Token* l_Token = &s_Lexer.m_Tokens[s_Lexer.m_TokenCount++];
    l_Token->m_Type         = p_Type;
    l_Token->m_SourceFile   = s_Lexer.m_CurrentFile;
    l_Token->m_Line         = s_Lexer.m_CurrentLine;
    l_Token->m_Column       = s_Lexer.m_CurrentColumn;

    // Check to see if a non-empty lexeme was provided.
    if (p_Lexeme[0] != '\0')
    {
        // If the token's string buffer has not already been allocated, allocate it.
        if (l_Token->m_Lexeme == NULL)
        {
            l_Token->m_Lexeme = GABLE_malloc(GABUILD_TOKEN_MAX_LENGTH, Char);
            GABLE_pexpect(l_Token->m_Lexeme, "Failed to allocate token lexeme buffer");
        }

        // Get the length of the lexeme, then copy it into the token's string buffer.
        Size l_Length = strlen(p_Lexeme);
        strncpy(l_Token->m_Lexeme, p_Lexeme, l_Length);
        l_Token->m_Lexeme[l_Length] = '\0';
    }

    return true;
}

static void GABUILD_FreeTokens ()
{
    // Iterate over each token, if it has a lexeme buffer, free it.
    for (Index i = 0; i < s_Lexer.m_TokenCount; ++i)
    {
        GABLE_free(s_Lexer.m_Tokens[i].m_Lexeme);
    }

    // Free the tokens array.
    GABLE_free(s_Lexer.m_Tokens);
    s_Lexer.m_Tokens        = NULL;
    s_Lexer.m_TokenCount    = 0;
    s_Lexer.m_TokenCapacity = 0;
    s_Lexer.m_TokenPointer  = 0;
}

// Static Functions - Lexing ///////////////////////////////////////////////////////////////////////

static Bool GABUILD_LexSymbol (FILE* p_File)
{
    Int32 l_Peek1 = 0, l_Peek2 = 0;

    switch (s_Lexer.m_Char)
    {
        case '+':
            // '+' = Plus, '+=' = Assign Plus, '++' = Increment
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_PLUS, "+="); }
            else if (l_Peek1 == '+') { return GABUILD_InsertToken(GABUILD_TOKEN_INCREMENT, "++"); }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_PLUS, "+");
        case '-':
            // '-' = Minus, '-=' = Assign Minus, '--' = Decrement
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_MINUS, "-="); }
            else if (l_Peek1 == '-') { return GABUILD_InsertToken(GABUILD_TOKEN_DECREMENT, "--"); }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_MINUS, "-");
        case '*':
            // '*' = Multiply, '**' = Exponent, '*=' = Assign Multiply, '**=' = Assign Exponent
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '*')
            {
                l_Peek2 = fgetc(p_File);

                if (l_Peek2 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_EXPONENT, "**="); }
                else { ungetc(l_Peek2, p_File); }

                return GABUILD_InsertToken(GABUILD_TOKEN_EXPONENT, "**");
            }
            else if (l_Peek1 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_MULTIPLY, "*="); }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_MULTIPLY, "*");
        case '/':
            // '/' = Divide, '/=' = Assign Divide
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_DIVIDE, "/="); }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_DIVIDE, "/");
        case '%':
            // '%' = Modulo, '%=' = Assign Modulo
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_MODULO, "%%="); }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_MODULO, "%%");
        case '&':
            // '&' = Bitwise And, '&=' = Assign Bitwise And, '&&' = Logical And
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '&') { return GABUILD_InsertToken(GABUILD_TOKEN_LOGICAL_AND, "&&"); }
            else if (l_Peek1 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_BITWISE_AND, "&="); }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_BITWISE_AND, "&");
        case '|':
            // '|' = Bitwise Or, '|=' = Assign Bitwise Or, '||' = Logical Or
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '|') { return GABUILD_InsertToken(GABUILD_TOKEN_LOGICAL_OR, "||"); }
            else if (l_Peek1 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_BITWISE_OR, "|="); }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_BITWISE_OR, "|");
        case '^':
            // '^' = Bitwise Xor, '^=' = Assign Bitwise Xor
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_BITWISE_XOR, "^="); }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_BITWISE_XOR, "^");
        case '~':
            // '~' = Bitwise Not
            return GABUILD_InsertToken(GABUILD_TOKEN_BITWISE_NOT, "~");
        case '<':
            // '<' = Compare Less, '<<' = Bitwise Shift Left, '<=' = Compare Less Equal, '<<=' = Assign Bitwise Shift Left
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '<')
            {
                l_Peek2 = fgetc(p_File);

                if (l_Peek2 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_BITWISE_SHIFT_LEFT, "<<="); }
                else { ungetc(l_Peek2, p_File); }

                return GABUILD_InsertToken(GABUILD_TOKEN_BITWISE_SHIFT_LEFT, "<<");
            }
            else if (l_Peek1 == '=')
            {
                return GABUILD_InsertToken(GABUILD_TOKEN_COMPARE_LESS_EQUAL, "<=");
            }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_COMPARE_LESS, "<");
        case '>':
            // '>' = Compare Greater, '>>' = Bitwise Shift Right, '>=' = Compare Greater Equal, '>>=' = Assign Bitwise Shift Right
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '>')
            {
                l_Peek2 = fgetc(p_File);

                if (l_Peek2 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_BITWISE_SHIFT_RIGHT, ">>="); }
                else { ungetc(l_Peek2, p_File); }

                return GABUILD_InsertToken(GABUILD_TOKEN_BITWISE_SHIFT_RIGHT, ">>");
            }
            else if (l_Peek1 == '=')
            {
                return GABUILD_InsertToken(GABUILD_TOKEN_COMPARE_GREATER_EQUAL, ">=");
            }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_COMPARE_GREATER, ">");
        case '=':
            // '=' = Assign Equal, '==' = Compare Equal
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_COMPARE_EQUAL, "=="); }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_ASSIGN_EQUAL, "=");
        case '!':
            // '!' = Logical Not, '!=' = Compare Not Equal
            l_Peek1 = fgetc(p_File);

            if (l_Peek1 == '=') { return GABUILD_InsertToken(GABUILD_TOKEN_COMPARE_NOT_EQUAL, "!="); }
            else { ungetc(l_Peek1, p_File); }

            return GABUILD_InsertToken(GABUILD_TOKEN_LOGICAL_NOT, "!");
        case '(':
            // '(' = Open Parenthesis
            return GABUILD_InsertToken(GABUILD_TOKEN_PARENTHESIS_OPEN, "(");
        case ')':
            // ')' = Close Parenthesis
            return GABUILD_InsertToken(GABUILD_TOKEN_PARENTHESIS_CLOSE, ")");
        case '[':
            // '[' = Open Bracket
            return GABUILD_InsertToken(GABUILD_TOKEN_BRACKET_OPEN, "[");
        case ']':
            // ']' = Close Bracket
            return GABUILD_InsertToken(GABUILD_TOKEN_BRACKET_CLOSE, "]");
        case '{':
            // '{' = Open Brace
            return GABUILD_InsertToken(GABUILD_TOKEN_BRACE_OPEN, "{");
        case '}':
            // '}' = Close Brace
            return GABUILD_InsertToken(GABUILD_TOKEN_BRACE_CLOSE, "}");
        case ',':
            // ',' = Comma
            return GABUILD_InsertToken(GABUILD_TOKEN_COMMA, ",");
        case ':':
            // ':' = Colon
            return GABUILD_InsertToken(GABUILD_TOKEN_COLON, ":");
        case '.':
            // '.' = Dot
            return GABUILD_InsertToken(GABUILD_TOKEN_PERIOD, ".");
        case '?':
            // '?' = Question Mark
            return GABUILD_InsertToken(GABUILD_TOKEN_QUESTION, "?");
        case '#':
            // '#' = Pound Sign
            return GABUILD_InsertToken(GABUILD_TOKEN_POUND, "#");
        default:
            GABLE_error("Unexpected character '%c' at line %zu, column %zu.", s_Lexer.m_Char, s_Lexer.m_CurrentLine, s_Lexer.m_CurrentColumn);
            return false;
    }
}

static Bool GABUILD_LexIdentifier (FILE* p_File)
{
    // Keep a string buffer for the identifier, and a counter for the current length.
    Char l_Buffer[GABUILD_TOKEN_MAX_LENGTH];
    Index l_Length = 0;

    // Also, keep a separate buffer for the identifier in uppercase.
    Char l_Upper[GABUILD_TOKEN_MAX_LENGTH];

    // Add characters to the buffer until a non-alphanumeric, non-underscore character is found.
    while (isalnum(s_Lexer.m_Char) || s_Lexer.m_Char == '_' || s_Lexer.m_Char == '#' || s_Lexer.m_Char == '.')
    {
        // Check if the buffer is full. If so, return an error.
        if (l_Length >= GABUILD_TOKEN_MAX_LENGTH)
        {
            GABLE_error("Identifier exceeds maximum length of %d characters.", GABUILD_TOKEN_MAX_LENGTH);
            return false;
        }

        // Add the character to the buffer.
        l_Buffer[l_Length]  = (Char) s_Lexer.m_Char;
        l_Upper[l_Length]   = (Char) toupper(s_Lexer.m_Char);
        l_Length++;

        // Read the next character from the file.
        s_Lexer.m_Char = fgetc(p_File);

        // Update the current column.
        s_Lexer.m_CurrentColumn++;
    }

    // Put the non-alphanumeric or non-underscore character back into the file stream.
    ungetc(s_Lexer.m_Char, p_File);
    s_Lexer.m_CurrentColumn--;

    // Add a null terminator to the buffers.
    l_Buffer[l_Length] = '\0';
    l_Upper[l_Length]  = '\0';

    // Check if the identifier is a keyword.
    const GABUILD_Keyword* l_Keyword = GABUILD_LookupKeyword(l_Upper);
    if (l_Keyword->m_Type != GABUILD_KT_NONE)
    {
        // Insert the keyword token, then attach the keyword to the token.
        GABUILD_InsertToken(GABUILD_TOKEN_KEYWORD, l_Buffer);
        s_Lexer.m_Tokens[s_Lexer.m_TokenCount - 1].m_Keyword = l_Keyword;

        return true;
    }
    else
    {
        return GABUILD_InsertToken(GABUILD_TOKEN_IDENTIFIER, l_Buffer);
    }
}

static Bool GABUILD_LexString (FILE* p_File)
{
    // Advance past the opening double quote.
    s_Lexer.m_Char = fgetc(p_File);
    s_Lexer.m_CurrentColumn++;

    // Keep a string buffer for the string, and a counter for the current length.
    Char l_Buffer[GABUILD_TOKEN_MAX_LENGTH];
    Index l_Length = 0;

    // Add characters to the buffer until a closing double quote is found.
    while (s_Lexer.m_Char != '"')
    {
        // Check if the buffer is full. If so, return an error.
        if (l_Length >= GABUILD_TOKEN_MAX_LENGTH)
        {
            GABLE_error("String exceeds maximum length of %d characters.", GABUILD_TOKEN_MAX_LENGTH);
            return false;
        }
        
        // Check for an escape character.
        if (s_Lexer.m_Char == '\\')
        {
            // Read the next character from the file.
            s_Lexer.m_Char = fgetc(p_File);
            s_Lexer.m_CurrentColumn++;

            // Check for the escape character.
            switch (s_Lexer.m_Char)
            {
                case '0': l_Buffer[l_Length] = '\0'; break;
                case 'a': l_Buffer[l_Length] = '\a'; break;
                case 'b': l_Buffer[l_Length] = '\b'; break;
                case 'f': l_Buffer[l_Length] = '\f'; break;
                case 'n': l_Buffer[l_Length] = '\n'; break;
                case 'r': l_Buffer[l_Length] = '\r'; break;
                case 't': l_Buffer[l_Length] = '\t'; break;
                case 'v': l_Buffer[l_Length] = '\v'; break;
                case '\\': l_Buffer[l_Length] = '\\'; break;
                case '"': l_Buffer[l_Length] = '"'; break;
                case '?': l_Buffer[l_Length] = '?'; break;
                default:
                    GABLE_error("Invalid escape character '\\%c'.", s_Lexer.m_Char);
                    return false;
            }
        }
        else
        {
            // The character is not an escape character.
            l_Buffer[l_Length] = (Char) s_Lexer.m_Char;
        }

        // Read the next character from the file.
        s_Lexer.m_Char = fgetc(p_File);
        s_Lexer.m_CurrentColumn++;

        // Update the current length.
        l_Length++;
    }

    // Add a null terminator to the buffer.
    l_Buffer[l_Length] = '\0';

    // Insert the string token.
    return GABUILD_InsertToken(GABUILD_TOKEN_STRING, l_Buffer);
}

static Bool GABUILD_LexCharacter (FILE* p_File)
{
    // Advance past the opening single quote.
    s_Lexer.m_Char = fgetc(p_File);
    s_Lexer.m_CurrentColumn++;
    
    // Keep exactly one character for the character token.
    Char l_Character = 0;

    // Check to see if the current character is an escape character.
    if (s_Lexer.m_Char == '\\')
    {
        // Read the next character from the file.
        s_Lexer.m_Char = fgetc(p_File);
        s_Lexer.m_CurrentColumn++;

        // Check for the escape character.
        switch (s_Lexer.m_Char)
        {
            case '0': l_Character = '\0'; break;
            case 'a': l_Character = '\a'; break;
            case 'b': l_Character = '\b'; break;
            case 'f': l_Character = '\f'; break;
            case 'n': l_Character = '\n'; break;
            case 'r': l_Character = '\r'; break;
            case 't': l_Character = '\t'; break;
            case 'v': l_Character = '\v'; break;
            case '\\': l_Character = '\\'; break;
            case '\'': l_Character = '\''; break;
            case '"': l_Character = '"'; break;
            case '?': l_Character = '?'; break;
            default:
                GABLE_error("Invalid escape character '\\%c'.", s_Lexer.m_Char);
                return false;
        }
    }
    else
    {
        // The character is not an escape character.
        l_Character = (Char) s_Lexer.m_Char;
    }
    
    // Read the next character from the file. It should be a closing single quote.
    s_Lexer.m_Char = fgetc(p_File);
    s_Lexer.m_CurrentColumn++;
    if (s_Lexer.m_Char != '\'')
    {
        GABLE_error("Expected closing single quote after character literal.");
        return false;
    }

    // Insert the character token.
    Char l_Buffer[2] = { l_Character, '\0' };
    return GABUILD_InsertToken(GABUILD_TOKEN_CHARACTER, l_Buffer);
}

static Bool GABUILD_LexBinary (FILE* p_File)
{
    Bool l_IsPercent = s_Lexer.m_Char == '%';

    // Advance past the 'b' or '%' character.
    s_Lexer.m_Char = fgetc(p_File);
    s_Lexer.m_CurrentColumn++;

    // Keep a string buffer for the binary number, and a counter for the current length.
    Char l_Buffer[GABUILD_TOKEN_MAX_LENGTH];
    Index l_Length = 0;

    // Add characters to the buffer until a non-binary character is found.
    while (s_Lexer.m_Char == '0' || s_Lexer.m_Char == '1')
    {
        // Check if the buffer is full. If so, return an error.
        if (l_Length >= GABUILD_TOKEN_MAX_LENGTH)
        {
            GABLE_error("Binary number exceeds maximum length of %d characters.", GABUILD_TOKEN_MAX_LENGTH);
            return false;
        }

        // Add the character to the buffer.
        l_Buffer[l_Length] = (Char) s_Lexer.m_Char;
        l_Length++;

        // Read the next character from the file.
        s_Lexer.m_Char = fgetc(p_File);

        // Update the current column.
        s_Lexer.m_CurrentColumn++;
    }

    // Make sure the binary number is not empty.
    if (l_Length == 0)
    {
        if (l_IsPercent == true)
        {
            ungetc(s_Lexer.m_Char, p_File);
            s_Lexer.m_Char = '%';
            s_Lexer.m_CurrentColumn--;
            return GABUILD_LexSymbol(p_File);
        }

        GABLE_error("Expected binary number after '0b' prefix.");
        return false;
    }

    // Put the non-binary character back into the file stream.
    ungetc(s_Lexer.m_Char, p_File);

    // Add a null terminator to the buffer.
    l_Buffer[l_Length] = '\0';

    // Insert the binary number token.
    return GABUILD_InsertToken(GABUILD_TOKEN_BINARY, l_Buffer);
}

static Bool GABUILD_LexOctal (FILE* p_File)
{
    Bool l_IsAmpersand = s_Lexer.m_Char == '&';

    // Advance past the 'o' or '&' character.
    s_Lexer.m_Char = fgetc(p_File);
    s_Lexer.m_CurrentColumn++;

    // Keep a string buffer for the octal number, and a counter for the current length.
    Char l_Buffer[GABUILD_TOKEN_MAX_LENGTH];
    Index l_Length = 0;

    // Add characters to the buffer until a non-octal character is found.
    while (s_Lexer.m_Char >= '0' && s_Lexer.m_Char <= '7')
    {
        // Check if the buffer is full. If so, return an error.
        if (l_Length >= GABUILD_TOKEN_MAX_LENGTH)
        {
            GABLE_error("Octal number exceeds maximum length of %d characters.", GABUILD_TOKEN_MAX_LENGTH);
            return false;
        }

        // Add the character to the buffer.
        l_Buffer[l_Length] = (Char) s_Lexer.m_Char;
        l_Length++;

        // Read the next character from the file.
        s_Lexer.m_Char = fgetc(p_File);

        // Update the current column.
        s_Lexer.m_CurrentColumn++;
    }

    // Make sure the octal number is not empty.
    if (l_Length == 0)
    {
        if (l_IsAmpersand == true)
        {
            ungetc(s_Lexer.m_Char, p_File);
            s_Lexer.m_Char = '&';
            s_Lexer.m_CurrentColumn--;
            return GABUILD_LexSymbol(p_File);
        }

        GABLE_error("Expected octal number after '0o' prefix.");
        return false;
    }

    // Put the non-octal character back into the file stream.
    ungetc(s_Lexer.m_Char, p_File);

    // Add a null terminator to the buffer.
    l_Buffer[l_Length] = '\0';

    // Insert the octal number token.
    return GABUILD_InsertToken(GABUILD_TOKEN_OCTAL, l_Buffer);
}

static Bool GABUILD_LexHexadecimal (FILE* p_File)
{
    Bool l_IsDollar = s_Lexer.m_Char == '$';

    // Advance past the 'x' or '$' character.
    s_Lexer.m_Char = fgetc(p_File);
    s_Lexer.m_CurrentColumn++;

    // Keep a string buffer for the hexadecimal number, and a counter for the current length.
    Char l_Buffer[GABUILD_TOKEN_MAX_LENGTH];
    Index l_Length = 0;

    // Add characters to the buffer until a non-hexadecimal character is found.
    while (isxdigit(s_Lexer.m_Char))
    {
        // Check if the buffer is full. If so, return an error.
        if (l_Length >= GABUILD_TOKEN_MAX_LENGTH)
        {
            GABLE_error("Hexadecimal number exceeds maximum length of %d characters.", GABUILD_TOKEN_MAX_LENGTH);
            return false;
        }

        // Add the character to the buffer.
        l_Buffer[l_Length] = (Char) s_Lexer.m_Char;
        l_Length++;

        // Read the next character from the file.
        s_Lexer.m_Char = fgetc(p_File);

        // Update the current column.
        s_Lexer.m_CurrentColumn++;
    }

    // Make sure the hexadecimal number is not empty.
    if (l_Length == 0)
    {
        GABLE_error("Expected hexadecimal number after '0x' or '$' prefix.");
        return false;
    }

    // Put the non-hexadecimal character back into the file stream.
    ungetc(s_Lexer.m_Char, p_File);

    // Add a null terminator to the buffer.
    l_Buffer[l_Length] = '\0';

    // Insert the hexadecimal number token.
    return GABUILD_InsertToken(GABUILD_TOKEN_HEXADECIMAL, l_Buffer);
}

static Bool GABUILD_LexNumber (FILE* p_File)
{
    // If the first digit is zero, then check the next character. We may be dealing with a binary,
    // octal, or hexadecimal number.
    if (s_Lexer.m_Char == '0')
    {
        // Read the next character from the file.
        s_Lexer.m_Char = fgetc(p_File);
        s_Lexer.m_CurrentColumn++;

        // Check for a binary, octal, or hexadecimal number.
        if (s_Lexer.m_Char == 'b' || s_Lexer.m_Char == 'B') { return GABUILD_LexBinary(p_File); }
        if (s_Lexer.m_Char == 'o' || s_Lexer.m_Char == 'O') { return GABUILD_LexOctal(p_File); }
        if (s_Lexer.m_Char == 'x' || s_Lexer.m_Char == 'X') { return GABUILD_LexHexadecimal(p_File); }

        // Put the non-binary, non-octal, non-hexadecimal character back into the file stream.
        ungetc(s_Lexer.m_Char, p_File);
        s_Lexer.m_Char = '0';
        s_Lexer.m_CurrentColumn--;
    }

    // Keep a string buffer for the number, and a counter for the current length.
    Char l_Buffer[GABUILD_TOKEN_MAX_LENGTH];
    Index l_Length = 0;

    // Keep a flag for whether a decimal point has been found.
    Bool l_Decimal = false;

    // Add characters to the buffer until a non-numeric character is found.
    while (isdigit(s_Lexer.m_Char) || s_Lexer.m_Char == '.')
    {
        // Check if the buffer is full. If so, return an error.
        if (l_Length >= GABUILD_TOKEN_MAX_LENGTH)
        {
            GABLE_error("Number exceeds maximum length of %d characters.", GABUILD_TOKEN_MAX_LENGTH);
            return false;
        }

        // Check for a decimal point.
        if (s_Lexer.m_Char == '.')
        {
            // Check if a decimal point has already been found. If so, return an error.
            if (l_Decimal)
            {
                GABLE_error("Number contains multiple decimal points.");
                return false;
            }

            l_Decimal = true;
        }

        // Add the character to the buffer.
        l_Buffer[l_Length] = (Char) s_Lexer.m_Char;
        l_Length++;

        // Read the next character from the file.
        s_Lexer.m_Char = fgetc(p_File);

        // Update the current column.
        s_Lexer.m_CurrentColumn++;
    }

    // Put the non-numeric character back into the file stream.
    ungetc(s_Lexer.m_Char, p_File);
    s_Lexer.m_CurrentColumn--;

    // Add a null terminator to the buffer.
    l_Buffer[l_Length] = '\0';

    // Insert the number token.
    return GABUILD_InsertToken(GABUILD_TOKEN_NUMBER, l_Buffer);
}

static Bool GABUILD_LexArgument (FILE* p_File)
{
    // This works the same as `GABUILD_LexNumber`, only that it allows only integers.
    // Keep a string buffer for the number, and a counter for the current length.
    Char l_Buffer[GABUILD_TOKEN_MAX_LENGTH];
    Index l_Length = 0;

    // Advance past the `@` or '\' character.
    s_Lexer.m_Char = fgetc(p_File);
    s_Lexer.m_CurrentColumn++;

    // Add characters to the buffer until a non-numeric character is found.
    while (isdigit(s_Lexer.m_Char))
    {
        // Check if the buffer is full. If so, return an error.
        if (l_Length >= GABUILD_TOKEN_MAX_LENGTH)
        {
            GABLE_error("Argument exceeds maximum length of %d characters.", GABUILD_TOKEN_MAX_LENGTH);
            return false;
        }

        // Add the character to the buffer.
        l_Buffer[l_Length] = (Char) s_Lexer.m_Char;
        l_Length++;

        // Read the next character from the file.
        s_Lexer.m_Char = fgetc(p_File);

        // Update the current column.
        s_Lexer.m_CurrentColumn++;
    }

    // Put the non-numeric character back into the file stream.
    ungetc(s_Lexer.m_Char, p_File);
    s_Lexer.m_CurrentColumn--;

    // Add a null terminator to the buffer.
    l_Buffer[l_Length] = '\0';

    // Insert the argument token.
    return GABUILD_InsertToken(GABUILD_TOKEN_ARGUMENT, l_Buffer);
}

static Bool GABUILD_Lex (FILE* p_File)
{
    Bool l_Comment = false;     // Whether the lexer is currently in a comment.

    while (true)
    {
        // Read the next character from the file.
        s_Lexer.m_Char = fgetc(p_File);

        // Check for the end of the file.
        if (s_Lexer.m_Char == EOF)
        {
            return GABUILD_InsertToken(GABUILD_TOKEN_EOF, "");
        }

        // Update the current column.
        s_Lexer.m_CurrentColumn++;

        // Check for a newline character. If found, update the current line and column. Also,
        // if the lexer is currently in a comment, reset the comment flag.
        if (s_Lexer.m_Char == '\n')
        {
            s_Lexer.m_CurrentLine++;
            s_Lexer.m_CurrentColumn = 0;
            l_Comment = false;
            GABUILD_InsertToken(GABUILD_TOKEN_NEWLINE, "");
            continue;
        }

        // Check for whitespace characters. If found, skip them.
        if (isspace(s_Lexer.m_Char))
        {
            continue;
        }

        // Check for the start of a comment, indicated by a semicolon (';').)
        if (s_Lexer.m_Char == ';')
        {
            l_Comment = true;
            continue;
        }

        // Lex the next token based on the current character.
        //
        // - Identifiers begin with an alphabetic character or an underscore.
        // - Strings begin with a double quote ('"').
        // - Characters begin with a single quote ('\'').
        // - Binary numbers begin with '0b' or '0B'.
        // - Octal numbers begin with '0o' or '0O'.
        // - Hexadecimal numbers begin with '0x' or '0X'.
        // - Numbers begin with a digit, unless that digit is a zero preceeded by one of the above
        //   prefixes.
        // - Symbols are any other character.
        Bool l_Good = false;
        if (isalpha(s_Lexer.m_Char) || s_Lexer.m_Char == '_' || s_Lexer.m_Char == '.')   
            { l_Good = GABUILD_LexIdentifier(p_File); }
        else if (s_Lexer.m_Char == '"')                             
            { l_Good = GABUILD_LexString(p_File); }
        else if (s_Lexer.m_Char == '\'')                            
            { l_Good = GABUILD_LexCharacter(p_File); }
        else if (s_Lexer.m_Char == '@' || s_Lexer.m_Char == '\\')   
            { l_Good = GABUILD_LexArgument(p_File); }
        else if (isdigit(s_Lexer.m_Char))                           
            { l_Good = GABUILD_LexNumber(p_File); }
        else if (s_Lexer.m_Char == '$')                             
            { l_Good = GABUILD_LexHexadecimal(p_File); }
        else if (s_Lexer.m_Char == '&')                             
            { l_Good = GABUILD_LexOctal(p_File); }
        else if (s_Lexer.m_Char == '%')                             
            { l_Good = GABUILD_LexBinary(p_File); }
        else                                                        
            { l_Good = GABUILD_LexSymbol(p_File); }

        if (l_Good == false)
        {
            return false;
        }
    }
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void GABUILD_InitLexer ()
{
    // Allocate the include files and tokens arrays.
    s_Lexer.m_IncludeFiles = GABLE_malloc(GABUILD_LEXER_CAPACITY, Char*);
    GABLE_pexpect(s_Lexer.m_IncludeFiles, "Failed to allocate include files array");

    s_Lexer.m_Tokens = GABLE_calloc(GABUILD_LEXER_CAPACITY, GABUILD_Token);
    GABLE_pexpect(s_Lexer.m_Tokens, "Failed to allocate tokens array");

    // Initialize the include files and tokens arrays.
    s_Lexer.m_IncludeFileCount     = 0;
    s_Lexer.m_IncludeFileCapacity  = GABUILD_LEXER_CAPACITY;
    s_Lexer.m_TokenCount           = 0;
    s_Lexer.m_TokenCapacity        = GABUILD_LEXER_CAPACITY;
    s_Lexer.m_TokenPointer         = 0;
}

void GABUILD_ShutdownLexer ()
{
    // Free the include files and tokens arrays.
    GABUILD_FreeIncludeFiles();
    GABUILD_FreeTokens();
}

Bool GABUILD_LexFile (const Char* p_FilePath)
{
    if (p_FilePath == NULL || p_FilePath[0] == '\0')
    {
        GABLE_error("File path string is NULL or blank.");
        return false;
    }
    
    // Resolve the file path to an absolute path.
    Char* l_ResolvedFilePath = NULL;
    Bool  l_Resolved = GABUILD_AddIncludeFile(p_FilePath, &l_ResolvedFilePath);
    if (l_Resolved == false)
    {
        return false;
    }
    else if (l_Resolved == true && l_ResolvedFilePath == NULL)
    {
        // File already included.
        return true;
    }

    // Open the file for reading.
    FILE* l_File = fopen(l_ResolvedFilePath, "r");
    if (l_File == NULL)
    {
        GABLE_perror("Failed to open file '%s' for reading", l_ResolvedFilePath);
        return false;
    }

    // Prepare the lexer context for lexing the file.
    s_Lexer.m_CurrentFile   = l_ResolvedFilePath;
    s_Lexer.m_CurrentLine   = 1;
    s_Lexer.m_CurrentColumn = 1;

    // Lex the file.
    Bool l_Lexed = GABUILD_Lex(l_File);
    if (l_Lexed == false)
    {
        GABLE_error("Failed to lex file '%s'.", l_ResolvedFilePath);
    }

    // Close the file and return the result of the lexing operation.
    fclose(l_File);
    return l_Lexed;
}

Bool GABUILD_HasMoreTokens ()
{
    return
        s_Lexer.m_TokenPointer < s_Lexer.m_TokenCount &&
        s_Lexer.m_Tokens[s_Lexer.m_TokenPointer].m_Type != GABUILD_TOKEN_EOF;
}

const GABUILD_Token* GABUILD_AdvanceToken ()
{
    if (GABUILD_HasMoreTokens())
    {
        return &s_Lexer.m_Tokens[s_Lexer.m_TokenPointer++];
    }
    else
    {
        return &s_Lexer.m_Tokens[s_Lexer.m_TokenCount - 1];
    }
}

const GABUILD_Token* GABUILD_AdvanceTokenIfType (GABUILD_TokenType p_Type)
{
    if (
        GABUILD_HasMoreTokens() && 
        s_Lexer.m_Tokens[s_Lexer.m_TokenPointer].m_Type == p_Type
    )
    {
        return &s_Lexer.m_Tokens[s_Lexer.m_TokenPointer++];
    }
    else
    {
        return NULL;
    }
}

const GABUILD_Token* GABUILD_AdvanceTokenIfKeyword (GABUILD_KeywordType p_Type)
{
    if (
        GABUILD_HasMoreTokens() && 
        s_Lexer.m_Tokens[s_Lexer.m_TokenPointer].m_Type == GABUILD_TOKEN_KEYWORD &&
        s_Lexer.m_Tokens[s_Lexer.m_TokenPointer].m_Keyword != NULL &&
        s_Lexer.m_Tokens[s_Lexer.m_TokenPointer].m_Keyword->m_Type == p_Type
    )
    {
        return &s_Lexer.m_Tokens[s_Lexer.m_TokenPointer++];
    }
    else
    {
        return NULL;
    }
}

const GABUILD_Token* GABUILD_PeekToken (Index p_Offset)
{
    if (s_Lexer.m_TokenPointer + p_Offset < s_Lexer.m_TokenCount)
    {
        return &s_Lexer.m_Tokens[s_Lexer.m_TokenPointer + p_Offset];
    }
    else
    {
        return &s_Lexer.m_Tokens[s_Lexer.m_TokenCount - 1];
    }
}

void GABUILD_PrintTokens ()
{
    for (Index i = 0; i < s_Lexer.m_TokenCount; ++i)
    {
        GABUILD_PrintToken(&s_Lexer.m_Tokens[i]);
    }
}

void GABUILD_ResetLexer ()
{
    s_Lexer.m_TokenPointer = 0;
    s_Lexer.m_TokenCount   = 0;
}
