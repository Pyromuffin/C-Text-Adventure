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
#include "state.h"

static bool s_CommandsRegistered[kCommandCount];
static Command s_AllCommands[kCommandCount];

void RegisterCommand(CommandLabel label, Command* command)
{
    assert(!s_CommandsRegistered[label]);
    assert(command);
    s_AllCommands[label] = *command;
    s_CommandsRegistered[label] = true;
}

void ExecuteLookCommand(const Command* this, Referent* subject, Referent* object)
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
        printf("I can't go %s.\n", GetDirectionString(dir));
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

void ExecuteMoveCommand(const Command* this, Referent* subject, Referent* object)
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

void ExecuteQuitCommand(const Command* this, Referent* subject, Referent* object)
{
    printf("Goodbye.\n");
    SetProgramRunningMode(kQuitting);
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
    lookCommand.parseFlags = kParseFlagImplicitObject | kParseFlagExplicitObject;
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


}

const Command *GetCommand(CommandLabel label)
{
    return &s_AllCommands[label];
}

