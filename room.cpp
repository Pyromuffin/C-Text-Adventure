#include <assert.h>
#include <stdio.h>
#include "room.h"
#include "items.h"
#include "CompileTimeStrings.h"

Room g_AllTheRooms[kRoomCount];
RoomLabel g_CurrentRoom = kDefaultRoom;

Room* GetCurrentRoom()
{
  return &g_AllTheRooms[g_CurrentRoom];
}

Room* GetRoomPtr( RoomLabel label)
{
  return &g_AllTheRooms[label];
}

Direction GetOpposingDirection( Direction dir )
{
  switch (dir)
  {
	case kDirectionLidaline:
		return kDirectionSnorth;
		break;
	case kDirectionAft:
		return kDirectionHandlebound;
		break;
	case kDirectionSpoutward:
		return kDirectionWhistlewise;
		break;
	case kDirectionSnorth:
		return kDirectionLidaline;
		break;
	case kDirectionHandlebound:
		return kDirectionAft;
		break;
	case kDirectionWhistlewise:
		return kDirectionSpoutward;
		break;
	case kDirectionCount:
		assert(false);
  }

  return kDirectionCount;
}

void ConnectRoomsTogether(RoomLabel from, RoomLabel to, Direction dir)
{
  Room* fromRoom = GetRoomPtr(from);
  Room* toRoom = GetRoomPtr(to);

  assert( fromRoom->connectedRooms[dir] == kNoRoom);
  fromRoom->connectedRooms[dir] = to;

  assert( toRoom->connectedRooms[GetOpposingDirection(dir)] == kNoRoom );
  toRoom->connectedRooms[GetOpposingDirection(dir)] = from;
}

void MoveToRoom(RoomLabel label)
{
    g_CurrentRoom = label;
}

void PrintRoomDescription( RoomLabel label )
{
    Room* room = GetRoomPtr(label);
    printf("%s\n", room->description);
}

void PrintArrivalGreeting( RoomLabel label )
{
    Room* room = GetRoomPtr(label);
	
    printf("You arrive in %s.\n", GetRoomReferent(label)->shortName);
    if( !room->visited)
    {
        PrintRoomDescription(label);
    }

    room->visited = true;
}

void InitRooms()
{
	for (int i = 0; i < kRoomCount; i++)
	{
		g_AllTheRooms[i].visited = false;

		for (int j = 0; j < kDirectionCount; j++)
		{
			g_AllTheRooms[i].connectedRooms[j] = kNoRoom;
		}
	}
}


void MakeRooms()
{
	Referent precreation;
	precreation.type = kReferentRoom;
	precreation.shortName = "nowhere";
	LIST_IDENTIFIERS(precreation, "nowhere");
	precreation.unionValues.room = kRoomPrecreation;
	GetRoomPtr(kRoomPrecreation)->description = "There is nothing here.";
	RegisterReferent(&precreation);

	Referent bethsRoom;
	bethsRoom.type = kReferentRoom;
	bethsRoom.shortName = "Beth's room";
	LIST_IDENTIFIERS(bethsRoom, "beth's room", "beths room");
	bethsRoom.unionValues.room = kRoomBeth;
	GetRoomPtr(kRoomBeth)->description = "This is a nice room. A little cramped.\n" 
		"No cats allowed.\n"
		"To the aft is the kitchen.";
	RegisterReferent(&bethsRoom);

	ConnectRoomsTogether(kRoomBeth, kRoomKitchen, kDirectionAft);

	Referent kitchen;
	kitchen.type = kReferentRoom;
	kitchen.shortName = "the kitchen";
	LIST_IDENTIFIERS(kitchen, "kitchen");
	kitchen.unionValues.room = kRoomKitchen;
	GetRoomPtr(kRoomKitchen)->description = "The kitchen is the heart of the apartment. A free standing shelf is piled high with tea.\n"
		"A table waits in the center of room. There's a sink next to the stove and a cabinet above.\n" 
		"The Refrigerator remains unmentioned.\n"
		"Handlebound is Beth's room, whistlewise leads to the bathroom, the cello room is lidaline, and the living room lies snorth.";
	RegisterReferent(&kitchen);

	ConnectRoomsTogether(kRoomKitchen, kRoomBathroom, kDirectionWhistlewise);
	ConnectRoomsTogether(kRoomKitchen, kRoomCello, kDirectionLidaline);
	ConnectRoomsTogether(kRoomKitchen, kRoomLiving, kDirectionSnorth);

	Referent bathroom;
	bathroom.type = kReferentRoom;
	bathroom.shortName = "the bathroom";
	LIST_IDENTIFIERS(bathroom, "bathroom");
	bathroom.unionValues.room = kRoomBathroom;
	GetRoomPtr(kRoomBathroom)->description = "The bathroom is your sanctuary. You would spend most your time in here if you thought no one would notice. The cats rarely attempt to infiltrate the bathroom.\n"
		"No cats allowed.\n"
		"There's a toilet with adjacent TP, a sink, a shower, and a mirror.\n"
		"Spoutward is the kitchen.";
	RegisterReferent(&bathroom);

	Referent celloRoom;
	celloRoom.type = kReferentRoom;
	celloRoom.shortName = "the cello room";
	LIST_IDENTIFIERS(celloRoom, "cello room");
	celloRoom.unionValues.room = kRoomCello;
	GetRoomPtr(kRoomCello)->description = "The oddly-named cello room is where you would keep your cello, if you had one.\n"
		"Effectively a utility closet, there's pretty much just stuff in here that you don't want cluttering up the rest of the apartment.\n"
		"You hear faint wailing accompanied by what sound like very quiet dance music.\n"
		"There's a vacuum cleaner, a pile of broken cat toys, a trash can, a deflated air bed.\n"
		"The kitchen is snorth.";
	RegisterReferent(&celloRoom);


	Referent livingRoom;
	livingRoom.type = kReferentRoom;
	livingRoom.shortName = "the living room";
	LIST_IDENTIFIERS(livingRoom, "living room");
	livingRoom.unionValues.room = kRoomLiving;
	GetRoomPtr(kRoomLiving)->description = "Not really a separate room from the kitchen; it's just a few steps snorth of the kitchen table.\n"
		"Covered in cat hair, it would be a nice place to relax while drinking tea.\n"
		"There's a couch near the wall.\n"
		"The kitchen is lidaline, and aft is your weird room mate's weird room mate room.";
	RegisterReferent(&livingRoom);

	ConnectRoomsTogether(kRoomLiving, kRoomRoommate, kDirectionAft);

	Referent roomMateRoom;
	roomMateRoom.type = kReferentRoom;
	roomMateRoom.shortName = "the weird weird roommate room";
	LIST_IDENTIFIERS(roomMateRoom, "weird room", "room mate's room", "roommate's room", "room mates room", "roommates room");
	roomMateRoom.unionValues.room = kRoomRoommate;
	GetRoomPtr(kRoomRoommate)->description = "ARE YOU YOUR WEIRD ROOMMATE? THIS IS A FORBIDDEN ZONE. EXPERIENCE REMORSE.\n"
		"THE LIVING ROOM IS HANDLEBOUND."; // commands must be entered in capitals when you're in this room.
	RegisterReferent(&roomMateRoom);

}