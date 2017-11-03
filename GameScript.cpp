#include <stdio.h>
#include <vector>
#include "GameScript.h"
#include "items.h"
#include "cereal\archives\json.hpp"

// better change this guy whenever you add or reorder serializable members
static const uint32_t SCRIPT_VERSION = 1; 
CEREAL_CLASS_VERSION(RoomScript, SCRIPT_VERSION);

RoomScript* RoomScript::ms_roomScripts[kRoomCount];

template<RoomLabel label>
auto& GetRoomScript() { return nullptr };

#define REGISTER_SCRIPT( name )  \
static name s_##name; \
template<> \
auto& GetRoomScript<name::label>() { return s_##name; }


RoomScript* GetRoomScript(RoomLabel label)
{
	return RoomScript::ms_roomScripts[label];
}


void RoomScript::Load()
{
	/*
	//std::ifstream saveFile;
	//saveFile.open("C:\\lomg\\potato.txt");

//	cereal::JSONInputArchive iarchive(saveFile);

	for (int i = 0; i < kRoomCount; i++)
	{
		if (ms_roomScripts[i] != nullptr)
		{
			iarchive(*ms_roomScripts[i]);
		}
	}
	*/
}


class KitchenScript : public SpecificRoomScript<kRoomKitchen>
{
public:
	Serialize<bool> m_cats = false;
	virtual void OnExit(RoomLabel to) override;
};
REGISTER_SCRIPT(KitchenScript);


class BethsRoomScript : public SpecificRoomScript<kRoomBeth>
{
public:
	
	Serialize<bool> once = true;
	Serialize<bool> m_doorOpen = false;
	Serialize<bool> m_coveredInCatHair = false;

	virtual void OnEnter(RoomLabel from) override
	{
		if (m_coveredInCatHair && once)
		{
			printf("Due to your negligence, the cats have ravaged Beth's once pristine room.\n\nYou lose one sneeze point!\n");
			once = false;
		}
	}

	virtual void OnExit(RoomLabel to) override
	{
		if(!m_doorOpen)
			printf("The door to the kitchen swings open.\n\n");

		m_doorOpen = true;
	}

	virtual void PrintAdditionalDescription() override
	{
		if (m_coveredInCatHair)
			printf("Cat hair covers every exposed surface. Just being in here makes your nose itch.\n");
	}


};
REGISTER_SCRIPT(BethsRoomScript);



void KitchenScript::OnExit(RoomLabel to)
{
	
	auto& bethsScript = GetRoomScript<kRoomBeth>();

	if (bethsScript.m_doorOpen && to != kRoomBeth)
	{
		printf("Because you left the door open, the cats rush into Beth's room!\n\n");
		bethsScript.m_coveredInCatHair = true;
	}
	
}

