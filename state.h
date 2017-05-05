//
// Created by Beth Kessler on 5/4/17.
//

#pragma once

#include <stdbool.h>

typedef enum ProgramRunningMode
{
    kPlaying,
    kQuitting,
} ProgramRunningMode;

typedef struct ProgramState
{
    ProgramRunningMode currentState;
} ProgramState;

void SetProgramRunningMode(ProgramRunningMode newRunningMode);
ProgramRunningMode GetProgramRunningMode();