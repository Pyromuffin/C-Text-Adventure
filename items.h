//
// Created by Kelly MacNeill on 5/1/17.
//
#pragma once

#include "utility.h"

#define kReferentUnregisteredIndex 0

enum RoomLabel : int;
enum Direction : int;
enum CommandLabel : int;
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
	const char* description;
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

    template<typename T>  void SetType(T) { }

    template<typename T>
	Referent(const char* shortName, T referentType)
	{
		SetType(referentType);
		this->shortName = shortName;
	}

private:

};

extern const uint MAX_REFERENT_COUNT;
extern Referent g_AllReferents[];

Referent* GetRoomReferent(enum RoomLabel label);
ReferentHandle RegisterReferent(Referent* referent);
const Referent* GetReferent(ReferentHandle handle);
ReferentHandle* GetAllRoomHandles();
ReferentHandle* GetAllVerbHandles();
ReferentHandle* GetAllDirectionHandles();
int GetTotalReferentCount();
void MakeDirectionReferents();
void MakeSomeItems();