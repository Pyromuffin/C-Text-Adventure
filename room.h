#pragma once
#include <stdbool.h>

struct Referent;

enum Direction
{
	kDirectionLidaline,
	kDirectionAft,
	kDirectionSpoutward,
	kDirectionSnorth,
	kDirectionHandlebound,
	kDirectionWhistlewise,

	kDirectionCount, // keep me at the bottom.
};

enum RoomLabel
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
    bool visited;
};

//extern RoomLabel g_CurrentRoom;
//extern Room g_AllTheRooms[kRoomCount];

void ConnectRoomsTogether(RoomLabel from, RoomLabel to, Direction dir);
Direction GetOpposingDirection( Direction dir );
Room* GetRoomPtr( RoomLabel label);
Room* GetCurrentRoom();
void MoveToRoom( RoomLabel label);


Referent* GetReferentsInRoom( RoomLabel label );
void PrintArrivalGreeting( RoomLabel label );
void MakeRooms();
void PrintRoomDescription(RoomLabel label);
