#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

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

static Room s_AllTheRooms[kRoomCount];
static RoomLabel s_CurrentRoom = kDefaultRoom;

Room* GetRoomPtr( RoomLabel label)
{
  return &s_AllTheRooms[label];
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

void CreateTwoRooms()
{
  CreateSingleRoom(
    kBethsRoom,
    "Beth's Room",
    "This is a nice room. No cats allowed!");

  CreateSingleRoom(
    kLivingRoom,
    "The Living Room",
    "Covered in cat hair. Constant beeping.");

  ConnectRoomsTogether(kBethsRoom, kLivingRoom, kSouth);
}


void PrintRoomDescription( Room* room )
{
  printf("%s\n", room->roomDescription);
}

void PrintArrivalGreeting( Room* room )
{
  printf("You arrive in %s.\n", room->roomName);
  if( !room->visited)
  {
    PrintRoomDescription(room);
  }

  room->visited = true;
}

bool IsQuitCommand(char* command)
{
  return !strcmp(command, "quit");

}

bool IsLookCommand(char* command)
{
  return !strcmp(command, "look") || !strcmp(command,"examine");
}

Room* GetCurrentRoom()
{
  return &s_AllTheRooms[s_CurrentRoom];
}

void ParseCommand(char* command)
{
    if(IsLookCommand(command))
    {
      PrintRoomDescription(GetCurrentRoom());
      return;
    }
}


int main( int argc, char** args )
{
  char command[256] = "Fake Command!";

  CreateTwoRooms();
  PrintArrivalGreeting(GetCurrentRoom());

  while( !IsQuitCommand(command) )
  {
    printf( ">" );
    scanf("%s", command);
    ParseCommand(command);

  }

  return 0;
}
