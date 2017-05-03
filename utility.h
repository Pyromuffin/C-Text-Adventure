//
// Created by Kelly MacNeill on 5/1/17.
//
#pragma once
#include <stdlib.h>

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

bool IsWhiteSpace(char c);
void TrimSelf(char* input);
