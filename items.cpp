//
// Created by Kelly MacNeill on 5/3/17.
//


#include "utility.h"
#include "items.h"
#include "room.h"
#include "CompileTimeStrings.h"

static const uint MAX_REFERENT_COUNT = 10000;

Referent g_AllReferents[MAX_REFERENT_COUNT];
static ReferentHandle s_NextReferentIndex = 0;
static ReferentHandle s_RoomHandles[kRoomCount];
static ReferentHandle s_VerbHandles[kCommandCount];


int GetTotalReferentCount()
{
    return s_NextReferentIndex;
}

ReferentHandle RegisterReferent(Referent *referent) {
    ReferentHandle handle = s_NextReferentIndex;
    g_AllReferents[handle] = *referent;
    s_NextReferentIndex++;

	if (referent->type & kReferentRoom)
	{
		s_RoomHandles[referent->unionValues.room] = handle;
	}

	if (referent->type & kReferentVerb)
	{
		s_VerbHandles[referent->unionValues.command] = handle;
	}


    return handle;
}

const Referent* GetReferent(ReferentHandle handle)
{
    return &g_AllReferents[handle];
}


void MakeSomeItems()
{
	Referent nvidia;
	nvidia.type = kReferentItem;
	nvidia.shortName = "Nvidia";

	nvidia.unionValues.item.description = "Nvidia geforce gtx titan x is the floop cat.\n"
		"Found near rugs, she is usually inverted - exposing her soft underbelly.\n"
		"She is a connoisseur of the inedible.\n"
		"Sneeze rating: Standard.\n";

	LIST_IDENTIFIERS(nvidia, "nvidia", "nv", "floop cat", "cat");
    RegisterReferent(&nvidia);

    Referent flavorBlast;
    flavorBlast.type = kReferentItem;
    flavorBlast.shortName = "Flavor Blast";

    flavorBlast.unionValues.item.description = "Flavor Blast is the floof cat.\n"
            "Barely smart enough to breathe, his primary function is to grow fur.\n"
            "He has a secret itchy spot under his chin.\n"
            "Sneeze rating: Severe.\n";

	LIST_IDENTIFIERS(flavorBlast, "flavor blast", "flavor cat", "flavor butt", "floof cat", "cat");
    RegisterReferent(&flavorBlast);



	Referent catHat;
	catHat.type = kReferentItem;
	catHat.shortName = "Cat Hat";
	catHat.unionValues.item.description = "I think this is a weird cat hat.\n";

	LIST_IDENTIFIERS(catHat, "cat hat", "hat");
	RegisterReferent(&catHat);
}

Referent* GetRoomReferent(RoomLabel label)
{
	return &g_AllReferents[s_RoomHandles[label]];
}


void MakeDirectionReferents()
{

	Referent northReferent;
	northReferent.type = kReferentDirection;
	northReferent.shortName = "north";
	LIST_IDENTIFIERS(northReferent, "north");
	
	northReferent.unionValues.direction = kNorth;
	RegisterReferent(&northReferent);


	Referent eastReferent;
	eastReferent.type = kReferentDirection;
	eastReferent.shortName = "east";
	LIST_IDENTIFIERS(eastReferent, "east");

	eastReferent.unionValues.direction = kEast;
	RegisterReferent(&eastReferent);


	Referent southReferent;
	southReferent.type = kReferentDirection;
	southReferent.shortName = "south";
	LIST_IDENTIFIERS(southReferent, "south");

	southReferent.unionValues.direction = kSouth;
	RegisterReferent(&southReferent);


	Referent westReferent;
	westReferent.type = kReferentDirection;
	westReferent.shortName = "west";
	LIST_IDENTIFIERS(westReferent, "west");

	westReferent.unionValues.direction = kWest;
	RegisterReferent(&westReferent);
}