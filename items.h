//
// Created by Kelly MacNeill on 5/1/17.
//
#pragma once

#include "room.h"
#include "utility.h"
#include "StringHash.h"
#include "commands.h"

typedef uint ReferentHandle;

typedef enum ItemFlags
{
    ItemFlagUsable = 1 << 0,

} ItemFlags;


typedef enum ReferentType
{
	kReferentDirection = 1 << 0,
	kReferentItem = 1 << 1,
	kReferentRoom = 1 << 2,
	kReferentVerb = 1 << 3,
}ReferentType;


typedef struct Item
{
    char* description;
    ItemFlags flags;

}Item;

struct Referent
{
	ReferentType type;
	const char* shortName;
	TokenString* identifiers;
	size_t identifierCount;

	union
	{
		Direction direction;
		Item item; // eventually move this out of here. use a handle like everything else.
		RoomLabel room;
		CommandLabel command;
	} unionValues;
};

extern const uint MAX_REFERENT_COUNT;
extern Referent g_AllReferents[];

Referent* GetRoomReferent(enum RoomLabel label);
ReferentHandle RegisterReferent(Referent* referent);
const Referent* GetReferent(ReferentHandle handle);
int GetTotalReferentCount();
void MakeDirectionReferents();
void MakeSomeItems();