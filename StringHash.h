//
// Created by Kelly MacNeill on 5/12/17.
//

#pragma once
#include <memory>
#include "utility.h"

struct DynamicIndexArray;
typedef uint Hash;

struct TokenString
{
	size_t tokenCount;
	Hash* hashes;
	size_t* tokenIndices;
	char* tokenizedString;
	bool dynamic = false;

	~TokenString()
	{
		if (dynamic)
		{
			free(hashes);
			free(tokenIndices);
			free(tokenizedString);
		}
	}

	struct Token
	{
		Hash hash;
		size_t index;
		const char *str;
	};

	Hash GetTokenHash(size_t index) const;
	Token GetToken(size_t tokenIndex) const;
	bool HasHash(Hash hash) const;
	bool HasSubstring(const TokenString* subts) const;
	bool operator == (const TokenString& r) const;

	struct TokenIterator
	{
		TokenIterator(const TokenString* ts, size_t pos)
			: currentIndex(pos), tokenString(ts) {}

		bool operator != (const TokenIterator& other) const
		{
			return currentIndex != other.currentIndex;
		}

		const Token operator* () const
		{
			return tokenString->GetToken(currentIndex);
		}

		const TokenIterator& operator++ ()
		{
			++currentIndex;
			return *this;
		}

		size_t currentIndex;
		const TokenString* tokenString;
	};
	TokenIterator begin() const 
	{
		return TokenIterator(this, 0);
	}
	TokenIterator end() const 
	{
		return TokenIterator(this, tokenCount);
	}
};

template<size_t staticTokenCount, size_t stringLength>
struct ConstantTokenString
{
	Hash hashes[staticTokenCount];
	size_t tokenIndices[staticTokenCount];
	char tokenizedString[stringLength];

	TokenString GetTokenString() const
	{
		TokenString ts;
		ts.hashes = (Hash*)&hashes[0];
		ts.tokenIndices = (size_t*)&tokenIndices[0];
		ts.tokenizedString = (char*)&tokenizedString[0];
		ts.tokenCount = staticTokenCount;

		return ts;
	}
};

constexpr Hash HashString(const char *string);
constexpr const size_t WordCount(const char *str);
std::unique_ptr<TokenString> AllocateTokenString(const char* inputString);
