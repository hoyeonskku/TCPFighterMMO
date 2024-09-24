#pragma once
#include <unordered_map>
#include "Player.h"


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

	bool AddPlayer(Player* player)
	{
		auto it = ObjectMap.find(player->sessionID);
		if (it != ObjectMap.end())
		{
			return false;
		}
		ObjectMap.insert(std::make_pair(player->sessionID, player));
		return true;
	}

	bool DeletePlayer(Player* player)
	{
		auto it = ObjectMap.find(player->sessionID);
		if (it == ObjectMap.end())
		{
			return false;
		}
		ObjectMap.erase(it);
		return true;
	}

	void Update();

public:
	std::unordered_map<int, Player*>& GetObjectMap() { return ObjectMap; }
};
