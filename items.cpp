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



void SetupIdentifierTokens(Referent* referent, char** identifiers)
{
    for(int i = 0; i < referent->identifierCount; i ++)
    {
        char* string = identifiers[i];
        referent->identifiers[i] = AllocateTokenString(string, referent->shortName);
    }
}

#define LIST_IDENTIFIERS( var, ... ) \
static char* var##_identifiers[] = { __VA_ARGS__ }; \
var.identifierCount = ARRAY_COUNT(var##_identifiers); \
static TokenString var##_tokenStrings[ARRAY_COUNT(var##_identifiers)]; \
SetupIdentifierTokens(&var, var##_identifiers)

void MakeSomeItems()
{
    Referent nvidia;
    nvidia.type = kReferentItem;
    nvidia.shortName = "Nvidia";

    nvidia.item.description = "Nvidia geforce gtx titan x is the floop cat.\n"
            "Found near rugs, she is usually inverted - exposing her soft underbelly.\n"
            "She is a connoisseur of the inedible.\n"
            "Sneeze rating: Standard.\n";

    LIST_IDENTIFIERS(nvidia,
                     "nvidia",
                     "nv",
                     "nvidia geforce gtx titan x",
                     "floop cat",
                     "cat");

    RegisterReferent(&nvidia);

    Referent flavorBlast;
    flavorBlast.type = kReferentItem;
    flavorBlast.shortName = "Flavor Blast";

    flavorBlast.item.description = "Flavor Blast is the floof cat.\n"
            "Barely smart enough to breathe, his primary function is to grow fur.\n"
            "He has a secret itchy spot under his chin.\n"
            "Sneeze rating: Severe.\n";

    LIST_IDENTIFIERS(flavorBlast, "flavor blast", "flavor cat", "flavor butt", "floof cat", "cat");
    RegisterReferent(&flavorBlast);
}

void MakeRoomReferent(RoomLabel label)
{
    Room* room = GetRoomPtr(label);
    Referent roomReferent;
    roomReferent.type = kReferentRoom;
    roomReferent.shortName = room->roomName;
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
