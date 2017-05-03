#pragma once
#include <stdbool.h>

typedef enum Direction
{
  kNorth,
  kWest,
  kSouth,
  kEast,

  kDirectionCount, // keep me at the bottom.
} Direction;

const char* directionStrings[kDirectionCount] =
        {
                "north",
                "west",
                "south",
                "east",
        };

typedef enum RoomLabel
{
  kBethsRoom,
  kLivingRoom,

  kRoomCount,
// special room labels here
  kDefaultRoom = kBethsRoom,
  kNoRoom,

} RoomLabel;

typedef struct Room
{
  char* roomName;
  char* roomDescription;
  RoomLabel connectedRooms[kDirectionCount];
  bool visited;

} Room;

//extern RoomLabel g_CurrentRoom;
//extern Room g_AllTheRooms[kRoomCount];

void ConnectRoomsTogether(RoomLabel from, RoomLabel to, Direction dir);
void CreateSingleRoom( RoomLabel label, char* roomName, char* roomDescription );
Direction GetOpposingDirection( Direction dir );
Room* GetRoomPtr( RoomLabel label);
Room* GetCurrentRoom();
void MoveToRoom( RoomLabel label);

void PrintArrivalGreeting( RoomLabel label );
void PrintRoomDescription( RoomLabel label );
const char* GetDirectionString(Direction dir) { return directionStrings[dir]; };