#include "room.h"
#include "utility.h"
#include "cereal\archives\json.hpp"

class GameState
{
private:
	static GameState instance;

public:
	RoomLabel currentRoom = kDefaultRoom;
	uint sneezePoints = 0;
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

	template<class Archive>
	void serialize(Archive & archive, const uint32_t version)
	{
		archive(CEREAL_NVP(currentRoom), CEREAL_NVP(sneezePoints));
	}

};