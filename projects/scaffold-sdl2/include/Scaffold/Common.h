/**
 * @file     Scaffold/Common.h
 * @brief    Contains additional common definitions and functions to what is provided by GABLE.
 */

#pragma once

#include <GABLE/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

/*
    Note: Replace `GAME_` in the following macros and constants with the prefix you have chosen for
    your project, if you wish.
*/

/**
 * @brief The maximum number of characters in an absolute file path.
 */
#define GAME_FILEPATH_STRLEN    1024

/**
 * @brief The application window's width, height and title.
 */
#define GAME_WINDOW_WIDTH       800
#define GAME_WINDOW_HEIGHT      720
#define GAME_WINDOW_TITLE       "GABLE Application (SDL2)"

/**
 * @brief The audio sample buffer's size.
 */
#define GAME_AUDIO_SAMPLE_BUFFER_SIZE   2048

/**
 * @brief The target number of audio samples, below which new audio data should be enqueued.
 */
#define GAME_AUDIO_SAMPLE_COUNT_TARGET  65536
