//
// Created by Kelly MacNeill on 4/30/17.
//
#pragma once
#include <stdbool.h>

struct Command;
struct Referent;
struct DynamicIndexArray;

typedef void (*CommandExecFunction)(const Command*, Referent*, Referent*);

typedef enum CommandLabel
{
    kCommandLook,
	kCommandExamine,
    kCommandDie,
    kCommandQuit,
    kCommandTake,
    kCommandMove,
    kCommandYes,
    kCommandNo,
	kCommandSave,
	kCommandLoad,

    kCommandCount,
    kCommandInvalid = -1,
} CommandLabel;

typedef enum ParseFlags
{
    kParseFlagImplicitObject = 1 << 0,
    kParseFlagExplicitObject = 1 << 1,
    kParseFlagSubjectAndObject =  1 << 2,

}ParseFlags;

struct Command
{
    CommandExecFunction execFunction;
    ParseFlags parseFlags;
};

extern Command g_AllCommands[];

DynamicIndexArray* getAvailableCommands();
void RegisterCommands();
void RegisterCommand(CommandLabel label, Command* command);
const Command* GetCommand(CommandLabel label);
bool IsAcceptableReferentCount(ParseFlags flags, int referentCount);
