/**
 * @file Hello/Application.h
 */

#pragma once

#include <GABLE/GABLE.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

typedef struct Hello_Application Hello_Application;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

Hello_Application* Hello_CreateApplication ();
void Hello_DestroyApplication (Hello_Application* p_App);
void Hello_StartApplication (Hello_Application* p_App);
