#pragma once
#include <array>
#include <tuple>
#include "utility.h"
#include "StringHash.h"

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

template<size_t tupleSize, typename TupleType, size_t ... tupleIndices>
auto GetTokenStringArrayIndices(TupleType& tuple, std::index_sequence<tupleIndices ...>)
{
	std::array<TokenString, tupleSize> pointerArray = { std::get<tupleIndices>(tuple).GetTokenString() ... };
	return pointerArray;
}

template<typename TupleType>
auto GetTokenStringArray(TupleType &tuple)
{
	constexpr size_t tupleSize = std::tuple_size<TupleType>::value;
	return GetTokenStringArrayIndices<tupleSize>(tuple, std::make_index_sequence<tupleSize>());
}

constexpr Hash ConstHashString(const char *string)
{
	Hash hash = 2166136261;

	for (int i = 0; string[i] != '\0'; i++)
	{
		hash = hash ^ string[i];
		hash = hash * 16777619;
	}

	return hash;
}

template <typename StrType, size_t index, size_t ... tokenIndices, std::size_t ... indices>
constexpr decltype(auto) build_string(std::index_sequence<tokenIndices ...>, std::index_sequence<indices...>)
{
	constexpr auto str = StrType{}.strList[index];
	constexpr size_t tokenCount = sizeof...(tokenIndices);
	constexpr size_t charCount = sizeof...(indices);

	size_t tokens[tokenCount] = { 0 };
	size_t currentIndex = 0;
	size_t currentTokenIndex = 1;
	char tokenized[charCount] = { str[indices] == ' ' ? (tokens[currentTokenIndex++] = ++currentIndex, '\0') : (currentIndex++, str[indices]) ... };

	ConstantTokenString<tokenCount, charCount> constTokenString =
	{
		{ ConstHashString(&tokenized[tokens[tokenIndices]]) ... },
		{ tokens[tokenIndices] ... },
		{ tokenized[indices] ... },
	};

	return constTokenString;
}

constexpr const size_t ConstWordCount(const char *str)
{
	size_t words = 1;
	for (int i = 0; str[i] != '\0'; i++)
	{
		if (str[i] == ' ')
			words++;
	}

	return words;
}

constexpr size_t ConstStrlen(const char *str)
{
	int i = 0;
	while (str[i] != '\0')
		i++;
	return i;
}

template <typename StrType, size_t ... stringIndices>
constexpr auto build_array(std::index_sequence<stringIndices ...>)
{
	constexpr size_t wordCountArray[] = { ConstWordCount(StrType{}.strList[stringIndices]) ... };
	constexpr size_t stringLengthArray[] = { ConstStrlen(StrType{}.strList[stringIndices]) + 1 ... };

	return std::make_tuple
	(
		build_string<StrType, stringIndices>
		(
			std::make_index_sequence<wordCountArray[stringIndices]>(),
			std::make_index_sequence<stringLengthArray[stringIndices]>()
			) ...
	);
}

template <typename StrType, size_t count>
constexpr decltype(auto) make_string()
{
	return build_array<StrType>(std::make_index_sequence<count>());
}

#define MakeIdentifierTuple( ... )  \
[]{\
	const char* strList[] = {__VA_ARGS__}; \
	constexpr size_t stringCount = ARRAY_COUNT(strList); \
	struct StrType { const char* strList[stringCount] = {__VA_ARGS__} ; }; \
	return make_string<StrType, stringCount>(); \
}()

inline void SetData(Referent& ref, TokenString* identifiers, size_t identifierCount)
{
	ref.identifiers = identifiers;
	ref.identifierCount = identifierCount;
}

#define LIST_IDENTIFIERS( refName, ... ) \
static constexpr const char* refName##_strList[] = { __VA_ARGS__ }; \
constexpr size_t refName##_stringCount = ARRAY_COUNT(refName##_strList); \
struct refName##_StrType { const char* const strList[refName##_stringCount] = { __VA_ARGS__ }; }; \
static constexpr auto refName##_tuple = make_string<refName##_StrType, refName##_stringCount>(); \
static auto refName##_array = GetTokenStringArray(refName##_tuple); \
SetData(refName, refName##_array.data(), refName##_stringCount)