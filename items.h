//
// Created by Kelly MacNeill on 5/1/17.
//

#pragma once


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
    char* name;
    char* description;
    ItemFlags flags;

}Item;

typedef struct Referent
{
    ReferentType type;
    char* name;
    union
    {
        Direction direction;
        Item* item;
        RoomLabel room;
    };

} Referent;
