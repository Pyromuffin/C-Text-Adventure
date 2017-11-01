//
// Created by Kelly MacNeill on 5/12/17.
//

#include "StringHash.h"
#include "IndexVector.h"
#include <assert.h>


/*
* from https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
hash = FNV_offset_basis
for each byte_of_data to be hashed
hash = hash XOR byte_of_data
hash = hash × FNV_prime
return hash
*/

static const uint FNV_OFFSET = 2166136261;
static const uint FNV_PRIME = 16777619;

constexpr Hash HashString(const char *string)
{
	Hash hash = FNV_OFFSET;

	for (int i = 0; string[i] != '\0'; i++)
	{
		hash = hash ^ string[i];
		hash = hash * FNV_PRIME;
	}

	return hash;
}

static void TokenizeString(char* dstString, size_t* dstIndices, const char* inputString)
{
	int currentTokenIndex = 1;
	dstIndices[0] = 0;

	int i;
	for (i = 0; inputString[i] != '\0'; i++)
	{
		if (inputString[i] == ' ')
		{
			dstString[i] = '\0';
			dstIndices[currentTokenIndex] = i + 1;
			currentTokenIndex++;
		}
		else
		{
			dstString[i] = inputString[i];
		}
	}

	dstString[i] = '\0';
}

constexpr const size_t WordCount(const char *str)
{
	size_t words = 1;
	for (int i = 0; str[i] != '\0'; i++)
	{
		if (str[i] == ' ')
			words++;
	}

	return words;
}

std::unique_ptr<TokenString> AllocateTokenString(const char* inputString)
{
	auto ts = std::make_unique<TokenString>();

	size_t wordCount = WordCount(inputString);
	size_t len = strlen(inputString) + 1;

	ts->dynamic = true;
	ts->hashes = (Hash*)malloc(sizeof(Hash) * wordCount);
	ts->tokenIndices = (size_t*)malloc(sizeof(size_t) *wordCount);
	ts->tokenizedString = (char*)malloc(sizeof(char) * len);
	ts->tokenCount = wordCount;

	TokenizeString(ts->tokenizedString, ts->tokenIndices, inputString);

	for (int i = 0; i < wordCount; i++)
	{
		ts->hashes[i] = HashString(&ts->tokenizedString[ts->tokenIndices[i]]);
	}

    return ts;
}

TokenString Substring(const TokenString* ts, size_t startIndex, size_t tokenCount)
{
	TokenString subTS;
	subTS.tokenCount = tokenCount;
	assert(startIndex + tokenCount <= ts->tokenCount);
	subTS.tokenIndices = &ts->tokenIndices[startIndex];
	subTS.hashes = &ts->hashes[startIndex];
	subTS.tokenizedString = &ts->tokenizedString[ts->tokenIndices[startIndex]];

	return subTS;
}

Hash TokenString::GetTokenHash(size_t index) const
{
	assert(index < tokenCount);
	return hashes[index];
}

TokenString::Token TokenString::GetToken(size_t tokenIndex) const
{
	Token token{ hashes[tokenIndex], tokenIndex, &tokenizedString[tokenIndices[tokenIndex]] };
	return token;
}

bool TokenString::HasHash(Hash hash) const
{
	for (int i = 0; i < tokenCount; i++)
	{
		if (hashes[i] == hash)
			return true;
	}

	return false;
}

bool TokenString::HasSubstring(const TokenString* subts) const
{
	if (subts->tokenCount > tokenCount)
		return false;

	size_t startingPlaces = (tokenCount - subts->tokenCount) + 1;

	for (int i = 0; i < startingPlaces; i++)
	{
		TokenString ts = Substring(this, i, subts->tokenCount);
		if (ts == *subts)
			return true;
	}

	return false;
}

int TokenString::FindSubstring(const TokenString* subts) const
{
	if (subts->tokenCount > tokenCount)
		return -1;

	size_t startingPlaces = (tokenCount - subts->tokenCount) + 1;

	for (int i = 0; i < startingPlaces; i++)
	{
		TokenString ts = Substring(this, i, subts->tokenCount);
		if (ts == *subts)
			return i;
	}

	return -1;
}


void TokenString::FindSubstrings(const TokenString* subts, DynamicIndexArray * dst) const
{
	if (subts->tokenCount > tokenCount)
		return;

	size_t startingPlaces = (tokenCount - subts->tokenCount) + 1;

	for (int i = 0; i < startingPlaces; i++)
	{
		TokenString ts = Substring(this, i, subts->tokenCount);
		if (ts == *subts)
			PushIndex(dst, i);
	}
}


bool TokenString::operator==(const TokenString& r) const
{
	if (tokenCount != r.tokenCount)
		return false;

	for (int i = 0; i < tokenCount; i++)
	{
		if (GetTokenHash(i) != r.GetTokenHash(i))
			return false;
	}

	return true;
}
