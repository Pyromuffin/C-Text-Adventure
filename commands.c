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

void RegisterCommands() {

    Command lookCommand;
    static char* lookVerbs[] = {"look", "examine", "view", "describe" };
    lookCommand.verbs = lookVerbs;
    lookCommand.verbCount = ARRAY_COUNT(lookVerbs);
    lookCommand.parseFlags = kParseFlagImplicitObject | kParseFlagExplicitObject;
    lookCommand.execFunction = ExecuteLookCommand;
    RegisterCommand(kCommandLook, &lookCommand);

    Command moveCommand;
    static char* moveVerbs[] = {"go", "move", "walk", "travel" };
    moveCommand.verbs = moveVerbs;
    moveCommand.verbCount = ARRAY_COUNT(moveVerbs);
    moveCommand.parseFlags = kParseFlagExplicitObject;
    moveCommand.execFunction = ExecuteMoveCommand;
    RegisterCommand(kCommandMove, &moveCommand);




}

const Command *GetCommand(CommandLabel label)
{
    return &s_AllCommands[label];
}

