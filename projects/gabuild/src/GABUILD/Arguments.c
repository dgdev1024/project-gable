/**
 * @file     GABUILD/Arguments.c
 */

#include <GABUILD/Arguments.h>

// Static Variables ////////////////////////////////////////////////////////////////////////////////

static Int32    s_Argc = 0;
static Char**   s_Argv = NULL;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void GABUILD_CaptureArguments (Int32 p_Argc, Char** p_Argv)
{
    s_Argc = p_Argc;
    s_Argv = p_Argv;
}

void GABUILD_ReleaseArguments ()
{
    s_Argc = 0;
    s_Argv = NULL;
}

Bool GABUILD_HasArgument (const Char* p_Longform, const Char p_Shortform)
{
    // Ensure that both the longform and shortform arguments are not NULL.
    if (p_Longform == NULL || p_Shortform == '\0')
    {
        GABLE_error("Must provide a valid longform and shortform argument.");
        return false;
    }

    // Iterate through all arguments.
    for (Int32 i = 1; i < s_Argc; i++)
    {
        // Point to the current argument. Make sure it is not NULL.
        Char* l_Argument = s_Argv[i];
        if (l_Argument == NULL) { continue; }

        // Key arguments must be at least 2 characters long.
        Size l_ArgumentLength = strlen(l_Argument);
        if (l_ArgumentLength < 2) { continue; }

        // Longform key arguments start with a double-dash '--'.
        if (strstr(l_Argument, "--") == l_Argument)
        {
            // Longform key arguments must be at least 3 characters long.
            if (l_ArgumentLength < 3) { continue; }

            // Compare the longform key argument.
            if (strcmp(l_Argument + 2, p_Longform) == 0)
            {
                return true;
            }
        }

        // Shortform key arguments start with a single dash '-'.
        else if (l_Argument[0] == '-')
        {
            // Compare the shortform key argument.
            if (l_Argument[1] == p_Shortform)
            {
                return true;
            }
        }
    }

    // Argument not found.
    return false;
}

const Char* GABUILD_GetArgumentValue (const Char* p_Longform, const Char p_Shortform)
{
    // Ensure that both the longform and shortform arguments are not NULL.
    if (p_Longform == NULL || p_Shortform == '\0')
    {
        GABLE_error("Must provide a valid longform and shortform argument.");
        return NULL;
    }

    // Iterate through all arguments.
    for (Int32 i = 1; i < s_Argc; ++i)
    {
        // Point to the current argument. Make sure it is not NULL.
        Char* l_Argument = s_Argv[i];
        if (l_Argument == NULL) { continue; }

        // Key arguments must be at least 2 characters long.
        Size l_ArgumentLength = strlen(l_Argument);
        if (l_ArgumentLength < 2) { continue; }

        // Ensure that there is an argument immediately after this one.
        if (i + 1 >= s_Argc) { continue; }

        // Ensure that the argument immediately after this one is a value argument. Value arguments
        // must be at least 1 character long and must not start with a dash.
        Char* l_Value = s_Argv[i + 1];
        if (l_Value == NULL) { continue; }
        else if (l_Value[0] == '-') { continue; }

        // Longform key arguments start with a double-dash '--'.
        if (strstr(l_Argument, "--") == l_Argument)
        {
            // Longform key arguments must be at least 3 characters long.
            if (l_ArgumentLength < 3) { continue; }

            // Compare the longform key argument.
            if (strcmp(l_Argument + 2, p_Longform) == 0)
            {
                return l_Value;
            }
        }

        // Shortform key arguments start with a single dash '-'.
        else if (l_Argument[0] == '-')
        {
            // Compare the shortform key argument.
            if (l_Argument[1] == p_Shortform)
            {
                return l_Value;
            }
        }
    }

    // Argument not found.
    return NULL;
}
