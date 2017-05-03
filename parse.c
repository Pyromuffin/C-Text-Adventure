//
// Created by Kelly MacNeill on 4/30/17.
//
#include <string.h>

#include "parse.h"
#include "utility.h"

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

ParseResult ParseCommand(char* commandString)
{
    ParseResult result;

    for(int i = 0; i < kCommandCount; i ++)
    {
        const Command* command = GetCommand((CommandLabel)i);
        if( StringHasAnyOfTheseWords(commandString, command->verbs, command->verbCount))
        {
            result.valid = true;
            result.commandLabel = (CommandLabel)i;
            return result;
        }
    }

    result.valid = false;
    return result;
}