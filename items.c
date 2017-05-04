//
// Created by Kelly MacNeill on 5/3/17.
//


#include "utility.h"
#include "items.h"

const uint MAX_REFERENT_COUNT = 10000;

static Referent s_AllReferents[MAX_REFERENT_COUNT];
static ReferentHandle s_NextReferentIndex = 0;
static ReferentHandle s_RoomHandles[kRoomCount];

ReferentHandle RegisterReferent(Referent *referent) {
    ReferentHandle handle = s_NextReferentIndex;
    s_AllReferents[handle] = *referent;
    s_NextReferentIndex++;
    return handle;
}

const Referent* GetReferent(ReferentHandle handle)
{
    return &s_AllReferents[handle];
}

void MakeSomeItems()
{



}

