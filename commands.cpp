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
#include "CompileTimeStrings.h"

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
    printf("%s\n", GetCurrentRoom()->description);
}


void ExecuteExamineCommand(const Command* me, Referent* subject, Referent* object)
{
	if (object && (object->type & kReferentItem))
	{
		printf("%s\n", object->unionValues.item.description);
	}
}



void ExecuteMoveDirection(Referent* dir)
{
    Room* currentRoom = GetCurrentRoom();
    RoomLabel targetRoom = currentRoom->connectedRooms[dir->unionValues.direction];

    if(targetRoom != kNoRoom)
    {
        MoveToRoom(targetRoom);
        PrintArrivalGreeting(targetRoom);
    }
    else
    {
        printf("I can't go %s.\n", dir->shortName);
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

    printf("I can't reach %s from here.\n", GetRoomReferent(label)->shortName);
}

void ExecuteMoveCommand(const Command* me, Referent* subject, Referent* object)
{
    assert(object);
    assert(object->type & (kReferentDirection | kReferentRoom));
    // somehow get direction out of object.
    if(object->type & kReferentDirection)
    {
        ExecuteMoveDirection(object);
    }
    else if (object->type & kReferentRoom)
    {
        ExecuteMoveRoom(object->unionValues.room);
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


void RegisterCommands()
{
    Command lookCommand;
    lookCommand.parseFlags = kParseFlagImplicitObject;
    lookCommand.execFunction = ExecuteLookCommand;
    RegisterCommand(kCommandLook, &lookCommand);

	Referent lookReferent;
	lookReferent.type = kReferentVerb;
	lookReferent.shortName = "look";
	lookReferent.unionValues.command = kCommandLook;
	LIST_IDENTIFIERS(lookReferent, "look", "look around");

	RegisterReferent(&lookReferent);


	Command examineCommand;
	examineCommand.parseFlags = kParseFlagExplicitObject;
	examineCommand.execFunction = ExecuteExamineCommand;
	RegisterCommand(kCommandExamine, &examineCommand);

	Referent examineReferent;
	examineReferent.type = kReferentVerb;
	examineReferent.shortName = "examine";
	examineReferent.unionValues.command = kCommandExamine;
	LIST_IDENTIFIERS(examineReferent, "examine", "look at", "describe", "tell me about");

	RegisterReferent(&examineReferent);



    Command moveCommand;
    moveCommand.parseFlags = kParseFlagExplicitObject;
    moveCommand.execFunction = ExecuteMoveCommand;
    RegisterCommand(kCommandMove, &moveCommand);

	Referent moveReferent;
	moveReferent.type = kReferentVerb;
	moveReferent.shortName = "move";
	moveReferent.unionValues.command = kCommandMove;
	LIST_IDENTIFIERS(moveReferent, "go", "move", "walk", "travel");

	RegisterReferent(&moveReferent);


	/*
    Command quitCommand;
	LIST_IDENTIFIERS(quitCommand, "quit", "exit");
    quitCommand.parseFlags = kParseFlagImplicitObject;
    quitCommand.execFunction = ExecuteQuitCommand;
    RegisterCommand(kCommandQuit, &quitCommand);

    Command dieCommand;
	LIST_IDENTIFIERS(dieCommand, "die");
    dieCommand.parseFlags = kParseFlagImplicitObject;
    dieCommand.execFunction = ExecuteDieCommand;
    RegisterCommand(kCommandDie, &dieCommand);

    Command yesCommand;
	LIST_IDENTIFIERS(yesCommand, "y", "yes");
    yesCommand.parseFlags = kParseFlagImplicitObject;
    yesCommand.execFunction = ExecuteYesCommand;
    RegisterCommand(kCommandYes, &yesCommand);

    Command noCommand;
	LIST_IDENTIFIERS(noCommand, "n", "no");
    noCommand.parseFlags = kParseFlagImplicitObject;
    noCommand.execFunction = ExecuteNoCommand;
    RegisterCommand(kCommandNo, &noCommand);
	*/

}

const Command *GetCommand(CommandLabel label)
{
    return &g_AllCommands[label];
}

