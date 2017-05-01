#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

#include "room.h"
#include "parse.h"


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
