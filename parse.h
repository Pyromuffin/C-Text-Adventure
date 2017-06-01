//
// Created by Kelly MacNeill on 4/30/17.
//
#pragma once
#include "commands.h"
#include "items.h"

extern bool g_DebugParse, g_RawDebugParse;

struct ParseResult
{
    bool valid;
    CommandLabel commandLabel;
    Referent *subject;
    Referent *object;
};

ParseResult ParseInputString(TokenString* inputString);