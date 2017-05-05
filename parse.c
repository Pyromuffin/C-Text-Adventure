//
// Created by Kelly MacNeill on 4/30/17.
//
#include <string.h>
#include <printf.h>

#include "parse.h"
#include "utility.h"
#include "IndexVector.h"
#include "items.h"

bool IsQuitCommand(char* command)
{
    return !strcmp(command, "quit");
}

bool IsLookCommand(char* command)
{
    return !strcmp(command, "look") || !strcmp(command,"examine");
}

bool IsDieCommand(char* command)
{
    return !strcmp(command, "die");

}

static bool StringHasAnyOfTheseWords(char* inputStr, const char** wordList, int wordCount)
{
    // this is probably the slowest way to do this.
    char copyBuffer[256];
    strncpy(copyBuffer, inputStr, ARRAY_COUNT(copyBuffer));

    char* token = strtok(copyBuffer, " ");
    while( token != NULL)
    {
        for( int i = 0; i < wordCount; i ++)
        {
            if(!strcmp(token, wordList[i]))
            {
                return true;
            }
        }

        token = strtok(NULL, " ");
    }

    return false;
}


CommandLabel FindVerb( char* commandString )
{
    for(int i = 0; i < kCommandCount; i ++)
    {
        const Command* command = GetCommand((CommandLabel)i);
        if( StringHasAnyOfTheseWords(commandString, command->verbs, command->verbCount))
        {
          return (CommandLabel)i;
        }
    }

    return kCommandInvalid;
}

IndexVector* FindReferents(char* commandString, IndexVector* potentialSet)
{
    //@todo implement me!
    return NULL;
}

ParseResult ParseCommand(char* commandString)
{
    ParseResult result;
    result.object = NULL;
    result.subject = NULL;
    result.valid = false;

    CommandLabel verb = FindVerb(commandString);
    if( verb != kCommandInvalid )
    {
        const Command* command = GetCommand(verb);
        result.valid = true;

    }

    return result;
}