//
// Created by Kelly MacNeill on 5/1/17.
//
#pragma once

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

typedef unsigned int uint;
typedef uint IndexType;
typedef IndexType ReferentHandle;
typedef uint Hash;

bool IsWhiteSpace(char c);
void TrimSelf(char* input);
