#pragma once
#include <stdbool.h>
#include "IndexVector.h"

struct Referent;

enum Direction : int
{
	kDirectionAlid,
	kDirectionAft,
	kDirectionSpoutward,
	kDirectionSnorth,
	kDirectionHandlebound,
	kDirectionWhistlewise,

	kDirectionCount, // keep me at the bottom.
};

enum RoomLabel : int
{
	// act one: before the universe
	kRoomPrecreation,

	// act two: mirror apartment
	kRoomBeth,
	kRoomKitchen,
	kRoomLiving,
	kRoomBathroom,
	kRoomCello,
	kRoomRoommate,
	kRoomMirrorRefrigerator,

	// cat three: kettle
	kRoomKettleTunnel,
	kRoomKettleRefrigerator,
	kRoomKettleLobby,
	kRoomKettleMarket,

	kRoomCount,
	// special room labels here
	kDefaultRoom = kRoomBeth,
	kNoRoom = -1,

};

struct Room
{
	const char* description;
	RoomLabel connectedRooms[kDirectionCount] = { kNoRoom, kNoRoom, kNoRoom, kNoRoom, kNoRoom, kNoRoom };
	DynamicIndexArray* containedItemReferents = nullptr;
    bool visited = false;
};

void ConnectRoomsTogether(RoomLabel from, RoomLabel to, Direction dir);
Direction GetOpposingDirection( Direction dir );
Room* GetRoomPtr( RoomLabel label);
Room* GetCurrentRoomPtr();
RoomLabel GetCurrentRoomLabel();
void SetCurrentRoom(RoomLabel label);


const DynamicIndexArray* GetReferentsInRoom( RoomLabel label );
void PrintArrivalGreeting( RoomLabel label );
void MakeRooms();
void PrintRoomDescription(RoomLabel label);
void AddItemToRoom(RoomLabel label, ReferentHandle referentIndex);
