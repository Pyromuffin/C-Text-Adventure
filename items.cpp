//
// Created by Kelly MacNeill on 5/3/17.
//


#include "utility.h"
#include "items.h"
#include "room.h"
#include "StringHash.h"
#include "CompileTimeStrings.h"
#include "commands.h"

const uint MAX_REFERENT_COUNT = 10000;

Referent g_AllReferents[MAX_REFERENT_COUNT];
static ReferentHandle s_NextReferentIndex = 1;
static ReferentHandle s_RoomHandles[kRoomCount];
static ReferentHandle s_VerbHandles[kCommandCount];
static ReferentHandle s_DirectionHandles[kDirectionCount];


ReferentHandle* GetAllRoomHandles()
{
	return &s_RoomHandles[0];
}

ReferentHandle* GetAllVerbHandles()
{
	return &s_VerbHandles[0];
}

ReferentHandle* GetAllDirectionHandles()
{
	return &s_DirectionHandles[0];
}

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
	else if (referent->type & kReferentVerb)
	{
		s_VerbHandles[referent->unionValues.command] = handle;
	}
	else if (referent->type & kReferentDirection)
	{
		s_DirectionHandles[referent->unionValues.direction] = handle;
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
	Referent lidReferent;
	lidReferent.type = kReferentDirection;
	lidReferent.shortName = "alid";
	LIST_IDENTIFIERS(lidReferent, "lid", "alid");
	lidReferent.unionValues.direction = kDirectionAlid;
	RegisterReferent(&lidReferent);


	Referent snorthReferent;
	snorthReferent.type = kReferentDirection;
	snorthReferent.shortName = "snorth";
	LIST_IDENTIFIERS(snorthReferent, "sn", "snorth");
	snorthReferent.unionValues.direction = kDirectionSnorth;
	RegisterReferent(&snorthReferent);


	Referent aftReferent;
	aftReferent.type = kReferentDirection;
	aftReferent.shortName = "aft";
	LIST_IDENTIFIERS(aftReferent, "aft");
	aftReferent.unionValues.direction = kDirectionAft;
	RegisterReferent(&aftReferent);


	Referent handleboundReferent;
	handleboundReferent.type = kReferentDirection;
	handleboundReferent.shortName = "handlebound";
	LIST_IDENTIFIERS(handleboundReferent, "hb", "handlebound");
	handleboundReferent.unionValues.direction = kDirectionHandlebound;
	RegisterReferent(&handleboundReferent);


	Referent spoutwardReferent;
	spoutwardReferent.type = kReferentDirection;
	spoutwardReferent.shortName = "spoutward";
	LIST_IDENTIFIERS(spoutwardReferent, "sw", "spoutward");
	spoutwardReferent.unionValues.direction = kDirectionSpoutward;
	RegisterReferent(&spoutwardReferent);


	Referent whistlewiseReferent;
	whistlewiseReferent.type = kReferentDirection;
	whistlewiseReferent.shortName = "whistlewise";
	LIST_IDENTIFIERS(whistlewiseReferent, "ww", "whistlewise");
	whistlewiseReferent.unionValues.direction = kDirectionWhistlewise;
	RegisterReferent(&whistlewiseReferent);
}



// template specializations

template<> void Referent::SetType<Item>(Item thing)					{ type = kReferentItem; unionValues.item = thing; }
template<> void Referent::SetType<Direction>(Direction thing)			{ type = kReferentDirection; unionValues.direction = thing; }
template<> void Referent::SetType<RoomLabel>(RoomLabel thing)			{ type = kReferentRoom; unionValues.room = thing; }
template<> void Referent::SetType<CommandLabel>(CommandLabel thing)	{ type = kReferentVerb; unionValues.command = thing; ; }