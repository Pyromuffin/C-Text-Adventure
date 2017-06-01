//
// Created by Kelly MacNeill on 4/30/17.
//
#include <string.h>
#include <stdio.h>

#include "parse.h"
#include "utility.h"
#include "IndexVector.h"
#include "items.h"
#include <vector>
#include <algorithm>
#include <tuple>
#include <assert.h>

bool g_DebugParse, g_RawDebugParse;

DynamicIndexArray* GetAvailableReferents()
{
    // for now, just search allllll referents.
    // eventually this needs to do the following:
    // referents in room
    // referents in inventory
    // global or special referents
    // referents to adjacent rooms

    DynamicIndexArray* referents = AllocateIndexVector(GetTotalReferentCount(), "All Referents" );

    for(int i =0; i < GetTotalReferentCount(); i ++)
    {
        PushIndexStatic(referents, (IndexType)i);
    }

    return referents;
}

struct IdentifierSequence
{
	size_t startPosition;
	size_t length;
	ReferentHandle referent;
};

void AddIdentifierSequences(const DynamicIndexArray* substringVector, size_t tokenCount, const ReferentHandle referentHandle, std::vector<IdentifierSequence> &sequences)
{
	for (int substringIndex = 0; substringIndex < substringVector->length; substringIndex++)
	{
		IndexType substringPos = substringVector->handles[substringIndex];
		IdentifierSequence sequence;
		sequence.length = tokenCount;
		sequence.referent = referentHandle;
		sequence.startPosition = substringPos;
		sequences.push_back(sequence);
	}
}

void PrintIdentifier(const IdentifierSequence& sequence, TokenString* input)
{
	for (int i = 0; i < sequence.length; i++)
	{
		printf("%s ", input->GetToken(sequence.startPosition + i).str);
	}
}


void DebugParsing(std::vector<IdentifierSequence>& sequences, bool disjunct, bool ambiguous, IdentifierSequence* potentialAmbiguous, TokenString* input)
{
	if (sequences.size() == 0)
	{
		printf("No identifier sequences found.\n");
		return;
	}

	const char* format =
		"=====DEBUG PARSE======\n"
		"disjunct: %s\n"
		"ambiguous: %s\n";

	printf(format, disjunct ? "true" : "false", ambiguous ? "true" : "false");


	if (ambiguous)
	{
		const char* ambiguousName = GetReferent(potentialAmbiguous->referent)->shortName;
		printf("ambiguous referent: %s, identifier: ", ambiguousName);
		PrintIdentifier(*potentialAmbiguous, input);
		printf("\n");
	}


	size_t longestShortName = 0;
	for (auto& s : sequences)
	{
		size_t shortNameLength = strlen(GetReferent(s.referent)->shortName);
		if (shortNameLength > longestShortName)
			longestShortName = shortNameLength;
	}
	// add some length for two dashes
	longestShortName += 2;

	// print each token of the input string
	for (auto token : *input)
	{
		size_t tokenLength = strlen(token.str);
		bool even = ((longestShortName - tokenLength) % 2 ) == 0;
		for (int i = 0; i < (longestShortName - tokenLength)/2; i++)
		{
			printf(" ");
		}

		if(!even)
			printf(" ");

		printf("%s ", token.str);
		for (int i = 0; i < (longestShortName - tokenLength)/2; i++)
		{
			printf(" ");
		}
	}
	printf("\n");

	for (auto token : *input)
	{
		printf("%llu", token.index);
		for (int i = 0; i < longestShortName; i++)
		{
			printf(" ");
		}
	}
	printf("\n");

	IdentifierSequence* lastSeq = nullptr;
	/////////////////// fix this //////////////////////////////////////////////////////
	char bigBuffer[1000];
	for (auto& seq : sequences)
	{
		auto ref = GetReferent(seq.referent);
		size_t refNameLength = strlen(ref->shortName);

		int numberOfDashes = (seq.length * longestShortName) - refNameLength;
		int numberOfSpaces = seq.startPosition * (longestShortName + 1);

		if (lastSeq)
		{
			bool overlapsStart = (seq.startPosition >= lastSeq->startPosition) && (seq.startPosition <= (lastSeq->startPosition + lastSeq->length - 1));
			bool further = seq.startPosition > lastSeq->startPosition;

			if (!overlapsStart && further)
			{
				size_t currentLength = strlen(bigBuffer);
				numberOfSpaces -= currentLength;
				numberOfSpaces = std::max(numberOfSpaces, 0);
				sprintf(bigBuffer, "%s%s%s%s%s", bigBuffer, std::string(numberOfSpaces, ' ').c_str(), std::string(numberOfDashes / 2, '-').c_str(), ref->shortName, std::string(numberOfDashes / 2, '-').c_str());
			}
			else
			{
				printf(bigBuffer);
				printf("\n");
				lastSeq = nullptr;
			}
		}

		if (!lastSeq)
		{
			sprintf(bigBuffer, "%s%s%s%s", std::string(numberOfSpaces, ' ').c_str(), std::string(numberOfDashes / 2, '-').c_str(), ref->shortName, std::string(numberOfDashes / 2, '-').c_str());
			lastSeq = &seq;
		}

	}

	printf(bigBuffer);
	printf("\n");

	int index = 0;
	for (auto& seq : sequences)
	{
		auto ref = GetReferent(seq.referent);
		printf("sequence index: %d, start: %llu, length: %llu, referent name: %s, identifier: ", index, seq.startPosition, seq.length, ref->shortName);
		PrintIdentifier(seq, input);
		printf("\n");
		index++;
	}
}


std::tuple<bool, bool, bool> OverlapsSequence(const IdentifierSequence& sequence, const IdentifierSequence& other)
{
	size_t startIndex = sequence.startPosition;
	size_t endIndex = startIndex + sequence.length - 1;

	size_t otherStartIndex = other.startPosition;
	size_t otherEndIndex = otherStartIndex + other.length - 1;

	bool beginsWithin = (otherStartIndex >= startIndex) && (otherStartIndex <= endIndex);
	bool endsWithin = (otherEndIndex >= startIndex) && (otherEndIndex <= endIndex);

	bool overlaps = beginsWithin && endsWithin;
	bool disjunct = (beginsWithin && !endsWithin) || (!beginsWithin && endsWithin);
	bool ambiguous = (startIndex == otherStartIndex) && (endIndex == otherEndIndex);

	return { overlaps, disjunct, ambiguous };
}


std::tuple<bool, bool, IdentifierSequence*> RemoveOverlappingSequences(std::vector<IdentifierSequence>& identifiers, std::vector<IdentifierSequence*>& overlappingIds, const IdentifierSequence& sequence)
{
	bool overlaps = false, disjunct = false, ambiguous = false;

	for (auto& id : identifiers)
	{
		if (&id == &sequence)
			continue;

		std::tie(overlaps, disjunct, ambiguous) = OverlapsSequence(sequence, id);

		if (disjunct || ambiguous)
			return { disjunct, ambiguous, &id };

		if (overlaps)
			overlappingIds.push_back(&id);
	}

	for (int i = 0; i < overlappingIds.size(); i++)
	{
		IdentifierSequence* ptr = overlappingIds[overlappingIds.size() - 1 - i];
		size_t index = ptr - identifiers.data();
		// do this backwards so we dont end up with stuff that is too out of order.
		identifiers[index] = identifiers.back();
		identifiers.pop_back();
	}

	return { disjunct, ambiguous, nullptr };
}


ParseResult ResolveParsedSequences(std::vector<IdentifierSequence>& sequences)
{
	auto startSorter = [](const IdentifierSequence& a, const IdentifierSequence& b)
	{
		return a.startPosition < b.startPosition;
	};

	assert(sequences.size() >= 1 && sequences.size() <= 3);

	std::sort(sequences.begin(), sequences.end(), startSorter);

	ParseResult result;
	bool first = true;
	result.valid = false;

	for (auto& seq : sequences)
	{
		Referent* ref = &g_AllReferents[seq.referent];

		if (ref->type == kReferentVerb)
		{
			result.commandLabel = ref->unionValues.command;
			result.valid = true;
		}
		else if (first)
		{
			result.object = ref;
			first = false;
		}
		else
		{
			result.subject = result.object;
			result.object = ref;
		}
	}

	return result;
}

// ok new parsing algorithm. 
// go through every word in the input string and tag each word with the list of referents that it has.
// need to return a token size array of pointers to index vectors of referents
ParseResult ParseInputString(TokenString* inputString)
{
	size_t tokenCount = inputString->tokenCount;

	DynamicIndexArray* substringVector = AllocateIndexVector(4, "Substring Indices");
	DynamicIndexArray *availableReferents = GetAvailableReferents();

	std::vector<IdentifierSequence> sequences;

	ITERATE_VECTOR(ref, availableReferents, g_AllReferents)
	{
		const ReferentHandle referentHandle = ref - g_AllReferents;

		for (int i = 0; i < ref->identifierCount; i++)
		{
			TokenString* identifier = &ref->identifiers[i];
			ResetVetor(substringVector);
			inputString->FindSubstrings(identifier, substringVector);
			AddIdentifierSequences(substringVector, identifier->tokenCount, referentHandle, sequences);
		}
	}

	FreeIndexVector(substringVector);
	FreeIndexVector(availableReferents);

	auto lengthSorter = [](IdentifierSequence& a, IdentifierSequence& b)
	{
		return a.length > b.length;
	};

	std::sort(sequences.begin(), sequences.end(), lengthSorter);

	if (g_RawDebugParse)
	{
		printf("====RAW DEBUG===\n");
		DebugParsing(sequences, false, false, nullptr, inputString);
		g_RawDebugParse = false;
	}

	std::vector<IdentifierSequence*> overlapping;
	bool disjunct = false, ambiguous = false;
	IdentifierSequence* potentialAmbiguous = nullptr;
	for (int i = 0; i < sequences.size(); i++)
	{
		overlapping.clear();
		std::tie(disjunct, ambiguous, potentialAmbiguous) = RemoveOverlappingSequences(sequences, overlapping, sequences[i]);
		if (disjunct || ambiguous)
			break;
	}

	if (g_DebugParse)
	{
		DebugParsing(sequences, disjunct, ambiguous, potentialAmbiguous, inputString);
		g_DebugParse = false;
	}

	return ResolveParsedSequences(sequences);
}


