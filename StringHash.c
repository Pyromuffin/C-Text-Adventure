//
// Created by Kelly MacNeill on 5/12/17.
//

#include "StringHash.h"

static const uint FNV_OFFSET = 2166136261;
static const uint FNV_PRIME = 16777619;

/*
 * from https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
   hash = FNV_offset_basis
   for each byte_of_data to be hashed
        hash = hash XOR byte_of_data
        hash = hash × FNV_prime
   return hash
 */


Hash HashString(char *string)
{
    Hash hash = FNV_OFFSET;

    for(int i = 0; string[i] != '\0'; i++)
    {
        hash = hash ^ string[i];
        hash = hash * FNV_PRIME;
    }

    return hash;
}

void MakeTokenStringHashes(TokenString tokenString)
{
    int hashCount = 0;
    ITERATE_VECTOR(token, tokenString.tokenIndices, tokenString.tokenizedString)
    {
        Hash hash = HashString(token);
        tokenString.hashes[i] = hash;
    }
}

static DynamicIndexArray* TokenizeString(char* string, const char* debugName)
{
    DynamicIndexArray* tokens =  AllocateIndexVector(3, debugName);

    // better hope that this is a trimmed string.
    PushIndex(tokens, 0);
    for(int i = 0; inputString[i] != '\0'; i++)
    {
        if(inputString[i] == ' ')
        {
            PushIndex(tokens, i + 1u);
            inputString[i] = '\0';
        }
    }

    return tokens;
}

TokenString AllocateTokenString(char* inputString, const char* debugName)
{
    TokenString ts;
    DynamicIndexArray* vector = TokenizeString(inputString, debugName);
    ts.tokenizedString = inputString;
    ts.tokenIndices = vector;
    ts.hashes = malloc(sizeof(Hash) * vector->length);
    MakeTokenStringHashes(ts);

    return ts;
}

void FreeTokenString(TokenString tokenString)
{
    free(tokenString.hashes);
    FreeIndexVector(tokenString.tokenIndices);
}