#include <stdio.h>


#include "GameScript.h"


static RoomScript* s_RoomScripts[kRoomCount];


RoomScript* GetRoomScript(RoomLabel label)
{
	return s_RoomScripts[label];
}


void RegisterRoomScript(RoomLabel label, RoomScript* roomScript)
{
	s_RoomScripts[label] = roomScript;
}



#define MAKE_CONSTRUCTOR(type) \
type(RoomLabel label) \
{ \
	RegisterRoomScript(label, this); \
}\
static RoomLabel ms_label


template<RoomLabel label>
auto& GetRoomScriptSpecifically() { static_assert(false, "Unregistered Script"); }

#define REGISTER_SCRIPT(myType, label) \
static myType s_##myType = myType(label); \
template<> \
auto& GetRoomScriptSpecifically<label>() \
{ \
	return s_##myType; \
}\
RoomLabel myType::ms_label = label


template<typename T>
T& GetRoomScriptSpecifically()
{
	return *reinterpret_cast<T*>(s_RoomScripts[T::ms_label]);
}


class KitchenScript : public RoomScript
{
public:
	MAKE_CONSTRUCTOR(KitchenScript);
	bool m_cats = false;
	virtual void OnExit(RoomLabel to) override;
};
REGISTER_SCRIPT(KitchenScript, kRoomKitchen);


class BethsRoomScript : public RoomScript
{
public:
	MAKE_CONSTRUCTOR(BethsRoomScript);

	bool m_doorOpen = false;
	bool m_coveredInCatHair = false;

	virtual void OnEnter(RoomLabel from) override
	{
		static bool once = true;
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
REGISTER_SCRIPT(BethsRoomScript, kRoomBeth);


void KitchenScript::OnExit(RoomLabel to)
{
	auto& bethsScript = GetRoomScriptSpecifically<kRoomBeth>();

	if (bethsScript.m_doorOpen && to != kRoomBeth)
	{
		printf("Because you left the door open, the cats rush into Beth's room!\n\n");
		bethsScript.m_coveredInCatHair = true;
	}
}

