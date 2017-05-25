#pragma once
#include <stdbool.h>
#include "items.h"

typedef struct Referent Referent;

typedef enum Direction
{
  kNorth,
  kWest,
  kSouth,
  kEast,

  kDirectionCount, // keep me at the bottom.
} Direction;

typedef enum RoomLabel
{
    kBethsRoom,
    kLivingRoom,
    kKellysRoom,

    kRoomCount,
    // special room labels here
    kDefaultRoom = kBethsRoom,
    kNoRoom = -1,

} RoomLabel;

typedef struct Room
{
	const char* description;
	RoomLabel connectedRooms[kDirectionCount] = { kNoRoom, kNoRoom, kNoRoom, kNoRoom };
    bool visited;
} Room;

//extern RoomLabel g_CurrentRoom;
//extern Room g_AllTheRooms[kRoomCount];

void ConnectRoomsTogether(RoomLabel from, RoomLabel to, Direction dir);
Direction GetOpposingDirection( Direction dir );
Room* GetRoomPtr( RoomLabel label);
Room* GetCurrentRoom();
void MoveToRoom( RoomLabel label);


Referent* GetReferentsInRoom( RoomLabel label );
void PrintArrivalGreeting( RoomLabel label );
void PrintRoomDescription( RoomLabel label );
