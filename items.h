//
// Created by Kelly MacNeill on 5/1/17.
//
#pragma once

#include "utility.h"

enum RoomLabel;
enum Direction;
enum CommandLabel;
struct TokenString;

enum ItemFlags
{
    ItemFlagUsable			 = 1 << 0,
	ItemFlagPickup			 = 1 << 1,
	ItemFlagContainer		 = 1 << 2,
	ItemFlagImplictLocation	 = 1 << 3,
};


enum ReferentType
{
	kReferentDirection = 1 << 0,
	kReferentItem = 1 << 1,
	kReferentRoom = 1 << 2,
	kReferentVerb = 1 << 3,
};


struct Item
{
	char* description;
	ItemFlags flags;
};



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

	Referent() {};
	template<typename T>
	
	Referent(const char* shortName, T referentType)
	{
		SetType(referentType);
		this->shortName = shortName;
	}

private:
	template<typename T>  void SetType(T) { static_assert(false); }
	template<> void SetType<Item>(Item thing)					{ type = kReferentItem; unionValues.item = thing; }
	template<> void SetType<Direction>(Direction thing)			{ type = kReferentDirection; unionValues.direction = thing; }
	template<> void SetType<RoomLabel>(RoomLabel thing)			{ type = kReferentRoom; unionValues.room = thing; }
	template<> void SetType<CommandLabel>(CommandLabel thing)	{ type = kReferentVerb; unionValues.command = thing; ; }
};

extern const uint MAX_REFERENT_COUNT;
extern Referent g_AllReferents[];

Referent* GetRoomReferent(enum RoomLabel label);
ReferentHandle RegisterReferent(Referent* referent);
const Referent* GetReferent(ReferentHandle handle);
int GetTotalReferentCount();
void MakeDirectionReferents();
void MakeSomeItems();