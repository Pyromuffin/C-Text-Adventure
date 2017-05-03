//
// Created by Beth Kessler on 5/2/17.
//
#include <string.h>
#include <stdbool.h>
#include "utility.h"

const char k_whiteSpaceChars[] =
        {
                ' ',
                '\n',
        };

bool IsWhiteSpace(char c)
{
    for (int x = 0; x < ARRAY_COUNT(k_whiteSpaceChars); x++)
    {
        if (c == k_whiteSpaceChars[x])
        return true;
    }
    return false;
}

void TrimSelf(char* input) {
    int inputLen = strlen(input);

    for (int i = inputLen - 1; IsWhiteSpace(input[i]); i--)
    {
        input[i] = '\0';
    }
}