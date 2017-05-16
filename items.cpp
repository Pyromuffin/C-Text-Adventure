//
// Created by Kelly MacNeill on 5/3/17.
//


#include "utility.h"
#include "items.h"

static const uint MAX_REFERENT_COUNT = 10000;

Referent g_AllReferents[MAX_REFERENT_COUNT];
static ReferentHandle s_NextReferentIndex = 0;
static ReferentHandle s_RoomHandles[kRoomCount];

int GetTotalReferentCount()
{
    return s_NextReferentIndex;
}

ReferentHandle RegisterReferent(Referent *referent) {
    ReferentHandle handle = s_NextReferentIndex;
    g_AllReferents[handle] = *referent;
    s_NextReferentIndex++;
    return handle;
}

const Referent* GetReferent(ReferentHandle handle)
{
    return &g_AllReferents[handle];
}

#define LIST_NAMES( var, ... ) \
static const char* var##Names[] = { __VA_ARGS__ }; \
var.names = var##Names; \
var.nameCount = ARRAY_COUNT(var##Names)

void MakeSomeItems()
{
    Referent nvidia;
    LIST_NAMES(nvidia, "Nvidia");
    RegisterReferent(&nvidia);

    Referent flavorBlast;
    LIST_NAMES(flavorBlast, "Flavor Blast");
    RegisterReferent(&flavorBlast);

    Referent beth;
    LIST_NAMES(beth, "Beth");
    RegisterReferent(&beth);

}

void MakeRoomReferent(RoomLabel label)
{
    Room* room = GetRoomPtr(label);
    Referent roomReferent;
    roomReferent.type = kReferentRoom;
    roomReferent.names = &room->roomName;
    roomReferent.nameCount = 1;
    roomReferent.room = label;
    RegisterReferent( &roomReferent);
}

void MakeRoomReferents()
{
    for(int i = 0; i < kRoomCount; i++)
    {
        MakeRoomReferent((RoomLabel)i);
    }
}

void MakeDirectionReferent(Direction dir)
{
    Referent directionReferent;
    directionReferent.type = kReferentDirection;
    directionReferent.names = GetDirectionStrings(dir);
    directionReferent.nameCount = 1;
    directionReferent.direction = dir;
    RegisterReferent( &directionReferent);
}

void MakeDirectionReferents()
{
    for(int i = 0; i < kDirectionCount; i++)
    {
        MakeDirectionReferent((Direction)i);
    }
}
