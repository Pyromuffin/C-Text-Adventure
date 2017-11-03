#pragma once
#include "room.h"
#include <assert.h>
#include <vector>

enum RoomLabel : int;

class RoomScript;
RoomScript* GetRoomScript(RoomLabel label);

class RoomScript
{
protected:
	std::vector<bool*> m_serializableBools;

public:
	virtual void OnEnter(RoomLabel from) {};
	virtual void OnExit(RoomLabel to) {};
	virtual void PrintAdditionalDescription() {};

	static RoomScript* ms_roomScripts[kRoomCount];

	template<class Archive>
	void serialize(Archive & archive, const uint32_t version)
	{
		for (auto& var : m_serializableBools)
		{
			archive(*var);
		}
	}

	template<typename T>
	std::vector<T*>& GetSerialArray() {};

	template<>
	std::vector<bool*>& GetSerialArray<bool>() { return m_serializableBools; }

	static void Load();
};


template<typename T, RoomLabel label>
class Serialize
{
	T thing;

public:
	T & operator=(const T& other) // copy assignment
	{
		return thing = other;
	};

	operator T() const { return thing; }

	Serialize(const T& other)
	{
		thing = other;

		RoomScript& p = GetRoomScript<label>();
		p.GetSerialArray<T>().push_back(&thing);
	}

	Serialize()
	{
		RoomScript& p = GetRoomScript<label>();
		p.GetSerialArray<T>().push_back(&thing);
	};
};


template<RoomLabel label>
class SpecificRoomScript : public RoomScript
{
public:
	constexpr static RoomLabel label = label;

	virtual void OnEnter(RoomLabel from) {};
	virtual void OnExit(RoomLabel to) {};
	virtual void PrintAdditionalDescription() {};

	template<typename T>
	using Serialize = Serialize<T, label>;

	Serialize<bool> visited = false;

protected:
	SpecificRoomScript()
	{
		ms_roomScripts[label] = this;
	}
};




