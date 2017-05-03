#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

#include "room.h"
#include "parse.h"
#include "utility.h"

bool g_StillAlive = true;

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




int main( int argc, char** args )
{
    char commandString[256] = "Fake Command!";
    char terminator[1];
    CreateTwoRooms();
    PrintArrivalGreeting(kDefaultRoom);
    RegisterCommands();

  while( !IsQuitCommand(commandString) && g_StillAlive )
  {
      printf( "> " );
      fflush(stdout);

      fgets(commandString, 255, stdin);
      if(commandString[0] == '\n')
          continue;

      TrimSelf(commandString);

      ParseResult result = ParseCommand(commandString);
      if( result.valid )
      {
        const Command* command = GetCommand(result.commandLabel);
        command->execFunction(command, result.subject, result.object);
        fflush(stdout);
      }
      else
      {
          printf("I don't know what is going on.\n");
      }
  }

  return 0;
}
