/**
 * @file     Scaffold/Application.h
 * @brief    Contains the main application context structure and functions.
 */

#pragma once

#include <Scaffold/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief Forward declaration of the application context structure.
 * 
 * @note Replace the `Game_` prefix in this structure and functions in this header and its
 *       corresponding source file with the prefix you have chosen for your project, if you wish.
 */
typedef struct Game_Application Game_Application;

// Function Declarations ///////////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a new application context.
 * 
 * @return A pointer to the newly created application context.
 */
Game_Application* Game_CreateApplication ();

/**
 * @brief Destroys an application context.
 * 
 * @param p_App The application context to destroy.
 */
void Game_DestroyApplication (Game_Application* p_App);

/**
 * @brief Prepares, then starts the application loop.
 * 
 * @param p_App The application context to run.
 */
void Game_StartApplication (Game_Application* p_App);
