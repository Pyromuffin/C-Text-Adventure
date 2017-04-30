#include <assert.h>
#include "room.h"

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
      return kDirectionCount;
  }
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

void CreateSingleRoom( RoomLabel label, char* roomName, char* roomDescription )
{
  Room* room = GetRoomPtr(label);
  room->roomName = roomName;
  room->roomDescription = roomDescription;
  room->visited = false;
  for(int i= 0; i < kDirectionCount; i ++ )
  {
    room->connectedRooms[i] = kNoRoom;
  }
}
