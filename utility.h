//
// Created by Kelly MacNeill on 5/1/17.
//
#pragma once
#include <stdlib.h>

#define ARRAY_COUNT(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

bool IsWhiteSpace(char c);
void TrimSelf(char* input);