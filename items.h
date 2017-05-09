//
// Created by Kelly MacNeill on 5/1/17.
//
#pragma once

#include "room.h"
#include "utility.h"

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
}ReferentType;


typedef struct Item
{
    char* description;
    ItemFlags flags;

}Item;

typedef struct Referent
{
    ReferentType type;
    const char** names;
    uint nameCount;

    union
    {
        Direction direction;
        Item* item;
        RoomLabel room;
    };
} Referent;

extern const uint MAX_REFERENT_COUNT;
extern Referent g_AllReferents[];

ReferentHandle RegisterReferent(Referent* referent);
const Referent* GetReferent(ReferentHandle handle);
int GetTotalReferentCount();
void MakeRoomReferents();
void MakeSomeItems();
