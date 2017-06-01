#include <stdio.h>
#include <stdbool.h>
#include <memory.h>

#include "room.h"
#include "parse.h"
#include "IndexVector.h"
#include "state.h"
#include "items.h"
#include "CompileTimeStrings.h"

void CreateTwoRooms()
{
	Referent bethsRoom;
	bethsRoom.type = kReferentRoom;
	bethsRoom.shortName = "Beth's room";
	LIST_IDENTIFIERS(bethsRoom, "beth's room", "beths room");
	bethsRoom.unionValues.room = kBethsRoom;
	GetRoomPtr(kBethsRoom)->description = "This is a nice room. No cats allowed!";
	RegisterReferent(&bethsRoom);

	Referent livingRoom;
	livingRoom.type = kReferentRoom;
	livingRoom.shortName = "the living room";
	LIST_IDENTIFIERS(livingRoom, "living room", "dying room", "super hyper room" );
	livingRoom.unionValues.room = kLivingRoom;
	GetRoomPtr(kLivingRoom)->description = "Covered in cat hair. Constant beeping.";
	RegisterReferent(&livingRoom);

    ConnectRoomsTogether(kBethsRoom, kLivingRoom, kSouth);
}

void VectorTest()
{
    DynamicIndexArray* v = AllocateIndexVector(1, "test 1");
    PushIndex(v, 10);

    PushIndex(v, 11);
    PushIndex(v, 12);
    PushIndex(v, 13);
    PushIndex(v, 14);

    DynamicIndexArray* potato = AllocateIndexVector(100, "Test 2");
    FreeIndexVector(v);
    FreeIndexVector(potato);
}


void Init()
{
    InitVectorTracking();
    RegisterCommands();

    CreateTwoRooms();
    MakeSomeItems();
    MakeDirectionReferents();
}

void CleanUp()
{
    CheckForVectorLeaks();
}

void SetDebugGlobals(char* commandString)
{
	size_t length = strlen(commandString);

	if ((commandString[length - 1] == '~') && (commandString[length - 2] == '~'))
	{
		g_RawDebugParse = true;
		commandString[length - 2] = '\0';
	}
	else if (commandString[length - 1] == '~')
	{
		g_DebugParse = true;
		commandString[length - 1] = '\0';
	}
}

int main(int argc, char **args)
{
    Init();

    char commandString[256] = "Fake Command!";
    PrintArrivalGreeting(kDefaultRoom);

    while (GetProgramRunningMode() == kPlaying || GetProgramRunningMode() == kDead)
    {
        printf("> ");
        fflush(stdout);

        fgets(commandString, 255, stdin);
        if (commandString[0] == '\n')
            continue;

        TrimSelf(commandString);
		SetDebugGlobals(commandString);

		auto inputTokens = AllocateTokenString(commandString);

		ParseResult result = ParseInputString(inputTokens.get());
		fflush(stdout);

        if (result.valid)
        {
            const Command *command = GetCommand(result.commandLabel);
            command->execFunction(command, result.subject, result.object);
            fflush(stdout);
        }
        else
        {
            printf("What's that !?\n");
        }
    }

    CleanUp();
    return 0;
}
