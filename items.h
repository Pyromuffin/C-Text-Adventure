//
// Created by Kelly MacNeill on 5/1/17.
//
#pragma once

#include "room.h"
#include "utility.h"

typedef uint ReferentHandle;
extern const uint MAX_REFERENT_COUNT;

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
    char** names;
    union
    {
        Direction direction;
        Item* item;
        RoomLabel room;
    };

} Referent;


ReferentHandle RegisterReferent(Referent* referent);
const Referent* GetReferent(ReferentHandle handle);
