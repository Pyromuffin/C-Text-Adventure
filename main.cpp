#include <stdio.h>
#include <stdbool.h>
#include <memory.h>

#include "room.h"
#include "parse.h"
#include "IndexVector.h"
#include "state.h"
#include "items.h"
#include "CompileTimeStrings.h"


void Init()
{
    InitVectorTracking();
    RegisterCommands();

    MakeRooms();
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
			printf("\n");
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