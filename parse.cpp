//
// Created by Kelly MacNeill on 4/30/17.
//
#include <string.h>
#include <stdio.h>

#include "parse.h"
#include "utility.h"
#include "IndexVector.h"
#include "items.h"




bool IsLookCommand(char* command)
{
    return !strcmp(command, "look") || !strcmp(command,"examine");
}

bool IsDieCommand(char* command)
{
    return !strcmp(command, "die");

}


CommandLabel FindVerb(TokenString *inputString)
{
    CommandLabel label = kCommandInvalid;
    DynamicIndexArray* availableCommands = getAvailableCommands();

    ITERATE_VECTOR(command, availableCommands, g_AllCommands)
    {
		for (int i = 0; i < command->identifierCount; i++)
		{
			TokenString *verbTS = &command->identifiers[i];
			if (inputString->HasSubstring(verbTS))
			{
				label = (CommandLabel)(command - g_AllCommands);
				goto badtime;
			}
		}
    }

badtime:
    FreeIndexVector(availableCommands);
    return label;
}

MAKE_STATIC_VECTOR(s_FoundReferents, 2);
DynamicIndexArray* FindReferents(TokenString* inputTokens, DynamicIndexArray* potentialSet)
{
    // for now just find the first two referents.
    // if two tokens, then the first is subject, second is object.
    // if one token then it's just the object.
    s_FoundReferents.length = 0;

	ITERATE_VECTOR(referent, potentialSet, g_AllReferents)
	{
		for (int identifierIndex = 0; identifierIndex < referent->identifierCount; identifierIndex++)
		{
			TokenString *identifier = &referent->identifiers[identifierIndex];
			if(inputTokens->HasSubstring(identifier))
			{
				PushIndexStatic(&s_FoundReferents, referent - g_AllReferents);
				if (s_FoundReferents.length == 2)
					return &s_FoundReferents;
			}
		}
	}
   
    return &s_FoundReferents;
}


DynamicIndexArray* GetAvailableReferents()
{
    // for now, just search allllll referents.
    // eventually this needs to do the following:
    // referents in room
    // referents in inventory
    // global or special referents
    // referents to adjacent rooms

    DynamicIndexArray* referents = AllocateIndexVector(GetTotalReferentCount(), "All Referents" );

    for(int i =0; i < GetTotalReferentCount(); i ++)
    {
        PushIndexStatic(referents, (IndexType)i);
    }

    return referents;
}


ParseResult ParseCommand(char* commandString)
{
    ParseResult result;
    result.object = NULL;
    result.subject = NULL;
    result.valid = false;

	std::unique_ptr<TokenString> inputTokens = AllocateTokenString(commandString);
    CommandLabel verb = FindVerb(inputTokens.get());

    if( verb != kCommandInvalid )
    {
        const Command* command = GetCommand(verb);
        result.valid = true;
        result.commandLabel = verb;

        // if the command needs referents.
        if(command->parseFlags & (kParseFlagExplicitObject | kParseFlagSubjectAndObject))
        {
            DynamicIndexArray* availableReferents = GetAvailableReferents();
            DynamicIndexArray* foundReferents = FindReferents(inputTokens.get(), availableReferents);
            if(!IsAcceptableReferentCount(command->parseFlags, foundReferents->length))
            {
                result.valid = false;
            }
            else
            {
                if(foundReferents->length == 1)
                {
                    result.object = &g_AllReferents[foundReferents->handles[0]];
                }
                else if ( foundReferents->length == 2)
                {
                    result.subject = &g_AllReferents[foundReferents->handles[0]];
                    result.object = &g_AllReferents[foundReferents->handles[1]];
                }
            }

            FreeIndexVector(availableReferents);
        }
    }

    return result;
}