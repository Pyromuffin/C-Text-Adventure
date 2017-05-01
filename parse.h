//
// Created by Kelly MacNeill on 4/30/17.
//

#ifndef C_TEXT_ADVENTURE_PARSE_H
#define C_TEXT_ADVENTURE_PARSE_H

#endif //C_TEXT_ADVENTURE_PARSE_H


#include <stdbool.h>

extern bool g_StillAlive;
bool IsQuitCommand(char* command);
bool IsLookCommand(char* command);
bool IsDieCommand(char* command);
