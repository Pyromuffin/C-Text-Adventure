//
// Created by Kelly MacNeill on 5/3/17.
//


#include "utility.h"
#include "items.h"

const uint MAX_REFERENT_COUNT = 10000;

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
static char* var##Names[] = { __VA_ARGS__ }; \
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

