//
// Created by Kelly MacNeill on 4/30/17.
//
#include <string.h>

#include "parse.h"

bool IsQuitCommand(char* command)
{
    return !strcmp(command, "quit");

}

bool IsLookCommand(char* command)
{
    return !strcmp(command, "look") || !strcmp(command,"examine");
}

