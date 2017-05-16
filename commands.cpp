//
// Created by Kelly MacNeill on 4/30/17.
//

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "commands.h"
#include "utility.h"
#include "items.h"
#include "room.h"
#include "state.h"
#include "IndexVector.h"

static bool s_CommandsRegistered[kCommandCount];
Command g_AllCommands[kCommandCount];

DynamicIndexArray* getAvailableCommands()
{
    DynamicIndexArray* commands = AllocateIndexVector(kCommandCount, "AvailableCommands");
    if (GetProgramRunningMode() == kDead) {
        PushIndex(commands, kCommandYes);
        PushIndex(commands, kCommandNo);
    }
    else if (GetProgramRunningMode() == kPlaying)
    {
        PushIndex(commands, kCommandLook);
        PushIndex(commands, kCommandMove);
        PushIndex(commands, kCommandQuit);
        PushIndex(commands, kCommandTake);
        PushIndex(commands, kCommandDie);
    }
    return commands;
}

void RegisterCommand(CommandLabel label, Command* command)
{
    assert(!s_CommandsRegistered[label]);
    assert(command);
    g_AllCommands[label] = *command;
    s_CommandsRegistered[label] = true;
}

bool IsAcceptableReferentCount(ParseFlags flags, int referentCount)
{
    if(referentCount == 0)
    {
        return flags & kParseFlagImplicitObject;
    }

    if(referentCount == 1)
    {
        return flags & kParseFlagExplicitObject;
    }

    if(referentCount == 2)
    {
        return flags & kParseFlagSubjectAndObject;
    }

    return false; // too many referents ?
}



void ExecuteLookCommand(const Command* me, Referent* subject, Referent* object)
{
    if( object && (object->type & kReferentItem) )
    {
        printf("%s\n", object->item->description);
    }
    else
    {
        printf("%s\n", GetCurrentRoom()->roomDescription);
    }
}


void ExecuteMoveDirection(Direction dir)
{
    Room* currentRoom = GetCurrentRoom();
    RoomLabel targetRoom = currentRoom->connectedRooms[dir];

    if(targetRoom != kNoRoom)
    {
        MoveToRoom(targetRoom);
        PrintArrivalGreeting(targetRoom);
    }
    else
    {
        printf("I can't go %s.\n", GetDirectionStrings(dir)[0]);
    }
}

void ExecuteMoveRoom(RoomLabel label)
{
    Room* current = GetCurrentRoom();

    for(int i =0; i < kDirectionCount; i ++ )
    {
        if( current->connectedRooms[i] == label)
        {
            MoveToRoom(label);
            PrintArrivalGreeting(label);
            return;
        }
    }

    printf("I can't reach %s from here.\n", GetRoomPtr(label)->roomName);
}

void ExecuteMoveCommand(const Command* me, Referent* subject, Referent* object)
{
    assert(object);
    assert(object->type & (kReferentDirection | kReferentRoom));
    // somehow get direction out of object.
    if(object->type & kReferentDirection)
    {
        ExecuteMoveDirection(object->direction);
    }
    else if (object->type & kReferentRoom)
    {
        ExecuteMoveRoom(object->room);
    }
}

void ExecuteQuitCommand(const Command* me, Referent* subject, Referent* object)
{
    printf("Goodbye.\n");
    SetProgramRunningMode(kQuitting);
}

void ExecuteDieCommand(const Command* me, Referent* subject, Referent* object)
{
    SetProgramRunningMode(kDead);
    printf("You have died.\nOh well. Would you like to start a new game? (y/n)\n");
}

void ExecuteYesCommand(const Command* me, Referent* subject, Referent* object)
{
    SetProgramRunningMode(kPlaying);
}

void ExecuteNoCommand(const Command* me, Referent* subject, Referent* object)
{
    ExecuteQuitCommand(NULL, NULL, NULL);
}

#define LIST_VERBS( cmd, ... ) \
static const char* cmd##Verbs[] = { __VA_ARGS__ }; \
cmd.verbs = cmd##Verbs; \
cmd.verbCount = ARRAY_COUNT(cmd##Verbs);

void RegisterCommands() {

    Command lookCommand;
    LIST_VERBS(lookCommand, "look", "examine", "view", "describe" );
    //static const char* lookVerbs[] = {  "look", "examine", "view", "describe" };
    //lookCommand.verbs = lookVerbs;
    //lookCommand.verbCount = ARRAY_COUNT(lookVerbs);
    lookCommand.parseFlags = (ParseFlags)(kParseFlagImplicitObject | kParseFlagExplicitObject);
    lookCommand.execFunction = ExecuteLookCommand;
    RegisterCommand(kCommandLook, &lookCommand);

    Command moveCommand;
    LIST_VERBS(moveCommand, "go", "move", "walk", "travel" );
    moveCommand.parseFlags = kParseFlagExplicitObject;
    moveCommand.execFunction = ExecuteMoveCommand;
    RegisterCommand(kCommandMove, &moveCommand);

    Command quitCommand;
    LIST_VERBS(quitCommand, "quit", "exit");
    quitCommand.parseFlags = kParseFlagImplicitObject;
    quitCommand.execFunction = ExecuteQuitCommand;
    RegisterCommand(kCommandQuit, &quitCommand);

    Command dieCommand;
    LIST_VERBS(dieCommand, "die");
    dieCommand.parseFlags = kParseFlagImplicitObject;
    dieCommand.execFunction = ExecuteDieCommand;
    RegisterCommand(kCommandDie, &dieCommand);

    Command yesCommand;
    LIST_VERBS(yesCommand, "y", "yes");
    yesCommand.parseFlags = kParseFlagImplicitObject;
    yesCommand.execFunction = ExecuteYesCommand;
    RegisterCommand(kCommandYes, &yesCommand);

    Command noCommand;
    LIST_VERBS(noCommand, "n", "no");
    noCommand.parseFlags = kParseFlagImplicitObject;
    noCommand.execFunction = ExecuteNoCommand;
    RegisterCommand(kCommandNo, &noCommand);


}

const Command *GetCommand(CommandLabel label)
{
    return &g_AllCommands[label];
}

