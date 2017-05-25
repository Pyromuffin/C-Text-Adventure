#include <assert.h>
#include <stdio.h>
#include "room.h"
#include "items.h"

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
    case kNorth:
      return kSouth;
    case kSouth:
      return kNorth;
    case kEast:
      return kWest;
    case kWest:
      return kEast;

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