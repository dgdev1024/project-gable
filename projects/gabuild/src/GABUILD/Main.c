/**
 * @file     GABUILD/Main.c
 */

#include <GABUILD/Arguments.h>
#include <GABUILD/Lexer.h>
#include <GABUILD/Parser.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define GABUILD_VERSION "0.1.0"

// Static Functions ////////////////////////////////////////////////////////////////////////////////

static void GABUILD_AtExit ()
{
    GABUILD_ShutdownParser();
    GABUILD_ShutdownLexer();
    GABUILD_ReleaseArguments();
}

static void GABUILD_PrintVersion ()
{
    printf("GABUILD - GABLE Asset BUILDer - Version %s\n", GABUILD_VERSION);
    printf("By: Dennis Griffin\n");
}

static void GABUILD_PrintHelp (FILE* p_Stream, const Char* p_ProgramName)
{
    fprintf(p_Stream, "Usage: %s [options]\n", p_ProgramName);
    fprintf(p_Stream, "Options:\n");
    fprintf(p_Stream, "  -i, --input-file <file>    Input source file\n");
    fprintf(p_Stream, "  -o, --output-file <file>   Output binary file\n");
    fprintf(p_Stream, "  -l, --lex-only             Only perform lexical analysis\n");
    fprintf(p_Stream, "  -h, --help                 Print this help message\n");
    fprintf(p_Stream, "  -v, --version              Print version information\n");
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

int main (int argc, char** argv)
{
    atexit(GABUILD_AtExit);
    GABUILD_CaptureArguments(argc, argv);

    const Char* l_InputFile     = GABUILD_GetArgumentValue("input-file", 'i');
    const Char* l_OutputFile    = GABUILD_GetArgumentValue("output-file", 'o');
    Bool        l_LexOnly       = GABUILD_HasArgument("lex-only", 'l');
    Bool        l_Help          = GABUILD_HasArgument("help", 'h');
    Bool        l_Version       = GABUILD_HasArgument("version", 'v');

    if (l_Help == true)
    {
        GABUILD_PrintVersion();
        GABUILD_PrintHelp(stdout, argv[0]);
        return 0;
    }

    if (l_Version == true)
    {
        GABUILD_PrintVersion();
        return 0;
    }

    if (l_InputFile == NULL)
    {
        fprintf(stderr, "Error: No input file specified\n\n");
        GABUILD_PrintHelp(stderr, argv[0]);
        return 1;
    }

    if (l_OutputFile == NULL && l_LexOnly == false)
    {
        fprintf(stderr, "Error: No output file specified\n\n");
        GABUILD_PrintHelp(stderr, argv[0]);
        return 1;
    }

    GABUILD_InitLexer();
    if (GABUILD_LexFile(l_InputFile) == false)
    {
        return 1;
    }

    if (l_LexOnly == true)
    {
        GABUILD_PrintTokens();
        return 0;
    }

    GABUILD_InitParser();
    if (GABUILD_ParseTokens(NULL) == false)
    {
        return 1;
    }

    return 0;
}
