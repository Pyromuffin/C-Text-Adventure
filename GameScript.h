#pragma once
#include "room.h"
#include <assert.h>

enum RoomLabel : int;
class RoomScript;

void RegisterRoomScript(RoomLabel label, RoomScript* roomScript);
RoomScript* GetRoomScript(RoomLabel label);

class RoomScript
{
public:
	virtual void OnEnter(RoomLabel from) {};
	virtual void OnExit(RoomLabel to) {};
	virtual void PrintAdditionalDescription() {};
};

