//
// Created by Kelly MacNeill on 4/30/17.
//
#pragma once
#include <stdbool.h>
#include "commands.h"
#include "items.h"

// the result of a parse should be a verb and optionally a subject and/or object

typedef struct ParseResult
{
    bool valid;
    CommandLabel commandLabel;
    Item *subject;
    Item *object;
} ParseResult;

ParseResult ParseCommand(char* command);


bool IsQuitCommand(char* command);
bool IsLookCommand(char* command);
bool IsDieCommand(char* command);
