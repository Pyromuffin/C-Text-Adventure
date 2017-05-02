//
// Created by Kelly MacNeill on 4/30/17.
//

#include <assert.h>
#include <stdlib.h>
#include <printf.h>
#include "commands.h"
#include "utility.h"
#include "items.h"
#include "room.h"

static bool s_CommandsRegistered[kCommandCount];
static Command s_AllCommands[kCommandCount];

void RegisterCommand(CommandLabel label, Command* command)
{
    assert(!s_CommandsRegistered[label]);
    assert(command);
    s_AllCommands[label] = *command;
    s_CommandsRegistered[label] = true;
}

void ExecuteLookCommand(const Command* this, Item* subject, Item* object)
{
    if( object )
    {
        printf("%s", object->description );
    }
    else
    {
        printf( "%s", GetCurrentRoom()->roomDescription );
    }
}

void RegisterCommands() {

    Command lookCommand;
    static char* lookVerbs[] = {"look", "examine", "view", "describe" };
    lookCommand.verbs = lookVerbs;
    lookCommand.verbCount = ARRAY_COUNT(lookVerbs);
    lookCommand.parseFlags = kParseFlagImplicitObject | kParseFlagExplicitObject;
    lookCommand.execFunction = ExecuteLookCommand;
    RegisterCommand(kCommandLook, &lookCommand);

}

const Command *GetCommand(CommandLabel label)
{
    return &s_AllCommands[label];
}

