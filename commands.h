//
// Created by Kelly MacNeill on 4/30/17.
//
#pragma once
#include <stdbool.h>

typedef struct Referent Referent;
typedef struct Command Command;
typedef struct IndexVector DynamicIndexArray;

typedef bool (*ParseFunction)(char*);
typedef void (*CommandExecFunction)(const Command*, Referent*, Referent*);

typedef enum CommandLabel
{
    kCommandLook,
    kCommandDie,
    kCommandQuit,
    kCommandTake,
    kCommandMove,
    kCommandYes,
    kCommandNo,
    kCommandCount,
    kCommandInvalid = -1,
} CommandLabel;

typedef enum ParseFlags
{
    kParseFlagImplicitObject = 1 << 0,
    kParseFlagExplicitObject = 1 << 1,
    kParseFlagSubjectAndObject =  1 << 2,

}ParseFlags;

typedef struct Command
{
    const char** verbs;
    int verbCount;
    CommandExecFunction execFunction;
    ParseFlags parseFlags;
} Command;

extern Command g_AllCommands[];

DynamicIndexArray* getAvailableCommands();
void RegisterCommands();
void RegisterCommand(CommandLabel label, Command* command);
const Command* GetCommand(CommandLabel label);
bool IsAcceptableReferentCount(ParseFlags flags, int referentCount);
