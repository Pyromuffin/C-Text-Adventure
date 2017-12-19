#pragma once

#include "room.h"
#include "utility.h"

class GameState
{
public:
	static GameState instance;


	RoomLabel currentRoom = kDefaultRoom;
	uint sneezePoints = 0;
	bool speedCheck = false;
	// current inventory duh

	static Room* GetCurrentRoomPtr()
	{
		return GetRoomPtr(instance.currentRoom);
	}

	static RoomLabel GetCurrentRoomLabel()
	{
		return instance.currentRoom;
	}

	static void SetCurrentRoom(RoomLabel label)
	{
		instance.currentRoom = label;
	}

	static bool GetSpeedCheck()
	{
		return instance.speedCheck;
	}

	template<class Archive>
	void serialize(Archive &archive, const unsigned int version);

};