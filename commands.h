//
// Created by Kelly MacNeill on 4/30/17.
//
#pragma once
#include <stdbool.h>

typedef struct Item Item;
typedef struct Command Command;

typedef bool (*ParseFunction)(char*);
typedef void (*CommandExecFunction)(const Command*, Item*, Item*);

typedef enum CommandLabel
{
    kCommandLook,
    kCommandDie,
    kCommandQuit,
    kCommandTake,

    kCommandCount,
} CommandLabel;

typedef enum ParseFlags
{
    kParseFlagImplicitObject = 1 << 0,
    kParseFlagExplicitObject = 1 << 1,
    kParseFlagSubjectAndObject =  1 << 2,

}ParseFlags;

typedef struct Command
{
    char** verbs;
    int verbCount;
    CommandExecFunction execFunction;
    ParseFlags parseFlags;
} Command;


void RegisterCommands();
void RegisterCommand(CommandLabel label, Command* command);
const Command* GetCommand(CommandLabel label);