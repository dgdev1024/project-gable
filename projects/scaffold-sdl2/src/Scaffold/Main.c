/**
 * @file Scaffold/Main.c
 */

#include <Scaffold/Application.h>

// Static Variables ////////////////////////////////////////////////////////////////////////////////

static Game_Application* s_App = NULL;

// Static Functions ////////////////////////////////////////////////////////////////////////////////

static void Game_AtExit ()
{
    Game_DestroyApplication(s_App);
}

// Main Function ///////////////////////////////////////////////////////////////////////////////////

int main ()
{
    // The application makes heavy use of the `exit` function when an error occurs, so we need to
    // ensure that the application context is destroyed before the program exits.
    atexit(Game_AtExit);

    // Create, then start the application.
    s_App = Game_CreateApplication();
    Game_StartApplication(s_App);

    return 0;
}
