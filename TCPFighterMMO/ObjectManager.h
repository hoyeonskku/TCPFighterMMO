#pragma once
#include <unordered_map>

class Player;

class ObjectManager
{
private:
	ObjectManager() {};
	~ObjectManager() {};

	std::unordered_map<int, Player*> ObjectMap;

public:
	static ObjectManager* GetInstance(void)
	{
		static ObjectManager Sys;
		return &Sys;
	}

	Player* FindPlayer(int sessionID)
	{
		const auto& it = ObjectMap.find(sessionID);

		if (it != ObjectMap.end())
		{
			return it->second;
		}

		return nullptr;
	}

	bool DeletePlayer(int sessionID)
	{
		auto it = ObjectMap.find(sessionID);
		if (it == ObjectMap.end())
		{
			return false;
		}
		ObjectMap.erase(it);
		return true;
	}

public:
	std::unordered_map<int, Player*>& GetObjectMap() { return ObjectMap;}
};
