//
// Created by Beth Kessler on 5/4/17.
//

#include "state.h"

static ProgramState s_TheProgram;

void SetProgramRunningMode(ProgramRunningMode newRunningMode)
{
    s_TheProgram.currentState = newRunningMode;
}

ProgramRunningMode GetProgramRunningMode()
{
    return s_TheProgram.currentState;
}