//
// Created by Kelly MacNeill on 4/30/17.
//
#include <string.h>
#include <printf.h>

#include "parse.h"
#include "utility.h"
#include "IndexVector.h"
#include "items.h"

typedef struct TokenString
{
    DynamicIndexArray* tokenIndices;
    char* tokenizedString;
} TokenString;


bool IsLookCommand(char* command)
{
    return !strcmp(command, "look") || !strcmp(command,"examine");
}

bool IsDieCommand(char* command)
{
    return !strcmp(command, "die");

}

static bool TokenIsAnyOfTheseWords(char* token, const char** wordList, int wordCount )
{
    for( int i = 0; i < wordCount; i ++ )
    {
        if(!strcmp(token, wordList[i]))
            return true;
    }

    return false;
}


static bool TokensAreAnyOfTheseWords(TokenString tokenString, const char** wordList, int wordCount)
{
    // this is probably the slowest way to do this.
    ITERATE_VECTOR(token, tokenString.tokenIndices, tokenString.tokenizedString)
    {
        if(TokenIsAnyOfTheseWords(token, wordList, wordCount))
        {
            return true;
        }
    }

    return false;
}

DynamicIndexArray* TokenizeString(char* inputString)
{
    DynamicIndexArray* tokens =  AllocateIndexVector(8, "Token Vector");
    int length = strlen(inputString);

    // better hope that this is a trimmed string.
    PushIndex(tokens, 0);
    for(int i = 0; i < length; i++)
    {
        if(inputString[i] == ' ')
        {
            PushIndex(tokens, i + 1u);
            inputString[i] = '\0';
        }
    }

    return tokens;
}

CommandLabel FindVerb(TokenString tokenString)
{
    for(int i = 0; i < kCommandCount; i ++)
    {
        const Command* command = GetCommand((CommandLabel)i);
        if(TokensAreAnyOfTheseWords(tokenString, command->verbs, command->verbCount))
        {
          return (CommandLabel)i;
        }
    }

    return kCommandInvalid;
}




MAKE_STATIC_VECTOR(s_FoundReferents, 2);

DynamicIndexArray* FindReferents(TokenString tokenString, DynamicIndexArray* potentialSet)
{
    // for now just find the first two referents.
    // if two tokens, then the first is subject, second is object.
    // if one token then it's just the object.
    s_FoundReferents.length = 0;
    ITERATE_VECTOR(token, tokenString.tokenIndices, tokenString.tokenizedString)
    {
        int referentIndex = 0;
        ITERATE_VECTOR(referent, potentialSet, g_AllReferents)
        {
            if( TokenIsAnyOfTheseWords(token, referent->names, referent->nameCount))
            {
                PushIndexStatic(&s_FoundReferents, potentialSet->handles[referentIndex]);

                if( s_FoundReferents.length  == 2)
                    goto badTime;
            }
            referentIndex++;
        }
    }

    badTime:
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

    char commandCopy[256];
    strncpy(commandCopy, commandString, 256);

    DynamicIndexArray* tokens = TokenizeString(commandCopy);
    TokenString tokenString = {tokens, commandCopy};

    CommandLabel verb = FindVerb(tokenString);
    if( verb != kCommandInvalid )
    {
        const Command* command = GetCommand(verb);
        result.valid = true;
        result.commandLabel = verb;

        // if the command needs referents.
        if(command->parseFlags & (kParseFlagExplicitObject | kParseFlagSubjectAndObject))
        {
            DynamicIndexArray* availableReferents = GetAvailableReferents();
            DynamicIndexArray* foundReferents = FindReferents(tokenString, availableReferents);
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

    FreeIndexVector(tokens);
    return result;
}