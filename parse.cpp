//
// Created by Kelly MacNeill on 4/30/17.
//
#include <string.h>
#include <stdio.h>

#include <vector>
#include <string>
#include <tuple>
#include <algorithm>

#include "parse.h"
#include "utility.h"
#include "IndexVector.h"
#include "StringHash.h"
#include "room.h"

//import std.core;
//import std.memory;

bool g_DebugParse, g_RawDebugParse;

DynamicIndexArray* GetAvailableReferents()
{
	// ok, so get all the direction referents, all the command referents, and the referents in the room, and probably all *discovered* room referents.

    DynamicIndexArray* referents = AllocateIndexVector(20, "available referents" );
	Room* currentRoom = GetCurrentRoomPtr();
	const DynamicIndexArray* roomItems = currentRoom->containedItemReferents;

	if (roomItems != nullptr)
	{
		for (uint i = 0; i < roomItems->length; i++)
		{
			PushIndex(referents, roomItems->handles[i]);
		}
	}

	ReferentHandle* allRooms = GetAllRoomHandles();
	ReferentHandle* allDirections = GetAllDirectionHandles();
	ReferentHandle* allCommands = GetAllVerbHandles();

	for (int i = 0; i < kCommandCount; i++)
	{
		if(allCommands[i] != kReferentUnregisteredIndex)
			PushIndex(referents, allCommands[i]);
	}

	for (int i = 0; i < Direction::kDirectionCount; i++)
	{
		ReferentHandle connectedRoom = currentRoom->connectedRooms[i];
		if ((connectedRoom != kNoRoom) && (allRooms[connectedRoom] != kReferentUnregisteredIndex))
		{
			PushIndex(referents, allRooms[connectedRoom]);
		}

		//also push the direction referents here.
		PushIndex(referents, allDirections[i]);
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
	for (uint substringIndex = 0; substringIndex < substringVector->length; substringIndex++)
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
		printf("%zu", token.index);
		for (int i = 0; i < longestShortName; i++)
		{
			printf(" ");
		}
	}
	printf("\n");

	IdentifierSequence* lastSeq = nullptr;
	char bigBuffer[1000];

	std::vector<IdentifierSequence> sequencesCopy;
	for (auto& seq : sequences)
	{
		sequencesCopy.push_back(seq);
	}

	for (int i = 0; i < sequencesCopy.size(); i++)
	{
		auto& seq = sequencesCopy[i];

		auto ref = GetReferent(seq.referent);
		size_t refNameLength = strlen(ref->shortName);

		size_t numberOfDashes = (seq.length * longestShortName) - refNameLength;
		size_t numberOfSpaces = seq.startPosition * (longestShortName + 1);

		sprintf(bigBuffer, "%s%s%s%s", std::string(numberOfSpaces, ' ').c_str(), std::string(numberOfDashes / 2, '-').c_str(), ref->shortName, std::string(numberOfDashes / 2, '-').c_str());

		auto* currentSeq = &seq;
		std::vector<IdentifierSequence*> removeList;

		for (int j = i + 1; j < sequencesCopy.size(); j++)
		{
			auto& nextSeq = sequencesCopy[j];
			bool overlapsStart = (nextSeq.startPosition >= currentSeq->startPosition) && (nextSeq.startPosition <= (currentSeq->startPosition + currentSeq->length - 1));
			bool further = nextSeq.startPosition > currentSeq->startPosition;

			if (!overlapsStart && further)
			{
				auto nextRef = GetReferent(nextSeq.referent);
				auto nextRefNameLength = strlen(nextRef->shortName);

				int nextNumberOfSpaces = nextSeq.startPosition * (longestShortName + 1);
				int nextNumberOfDashes = (nextSeq.length * longestShortName) - nextRefNameLength;

				size_t currentLength = strlen(bigBuffer);
				nextNumberOfSpaces -= currentLength;
				nextNumberOfSpaces = std::max(nextNumberOfSpaces, 0);
				sprintf(bigBuffer, "%s%s%s%s%s", bigBuffer, std::string(nextNumberOfSpaces, ' ').c_str(), std::string(nextNumberOfDashes / 2, '-').c_str(), nextRef->shortName, std::string(nextNumberOfDashes / 2, '-').c_str());

				nextSeq.referent = -1;
				currentSeq = &nextSeq;
			}
		}

		auto remover = [](const IdentifierSequence& s)
		{
			return s.referent == -1;
		};

		sequencesCopy.erase(std::remove_if(sequencesCopy.begin(), sequencesCopy.end(), remover), sequencesCopy.end());
	
		printf("%s\n",bigBuffer);
	}

	int index = 0;
	for (auto& seq : sequences)
	{
		auto ref = GetReferent(seq.referent);
		printf("sequence index: %d, start: %zu, length: %zu, referent name: %s, identifier: ", index, seq.startPosition, seq.length, ref->shortName);
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

	// assert(sequences.size() >= 1 && sequences.size() <= 3);

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

	if (result.valid)
	{
		const Command* cmd = GetCommand(result.commandLabel);
		result.valid = IsAcceptableReferentCount(cmd->parseFlags, sequences.size() - 1);
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
		if (a.length == b.length)
		{
			return a.startPosition < b.startPosition;
		}

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
		auto [disjunct, ambiguous, potentialAmbiguous] = RemoveOverlappingSequences(sequences, overlapping, sequences[i]);
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


