//
// Created by Kelly MacNeill on 5/12/17.
//

#pragma once

#include "utility.h"
#include "IndexVector.h"

typedef uint Hash;

typedef struct TokenString
{
    DynamicIndexArray* tokenIndices;
    char* tokenizedString;
    Hash* hashes;
} TokenString;

TokenString AllocateTokenString(char* inputString, const char* debugName);
void FreeTokenString(TokenString tokenString);

Hash HashString(char * string);