#pragma once
//#include <unordered_map>
#include "Player.h"
#include "CObjectPool.h"
#include "SessionManager.h"

#define PLAYERMAXCOUNT 8000

class ObjectManager
{
private:
	ObjectManager() 
	{
		for (int i = 0; i < PLAYERMAXCOUNT; i++)
		{
			_freeArray[i] = i;
		}
	};
	~ObjectManager() {};

	//std::unordered_map<int, Player*> ObjectMap;

	//std::unordered_map<int, Session*> _sessionMap;
	Player _playerArray[PLAYERMAXCOUNT];
	int _freeArray[PLAYERMAXCOUNT];
	int _playerCount = 0;

public:
	static ObjectManager* GetInstance(void)
	{
		static ObjectManager Sys;
		return &Sys;
	}

	Player* CreatePlayer(unsigned long long sessionID)
	{
		unsigned short index = SessionManager::GetInstance()->GetIndexBySessionID(sessionID);
		_playerArray[index].useFlag = true;
		_playerArray[index].Clear();
		return &_playerArray[index];
	}

	Player* GetPlayer(unsigned long long sessionID)
	{
		unsigned short index = SessionManager::GetInstance()->GetIndexBySessionID(sessionID);
		return &_playerArray[index];
	}

	bool DeletePlayer(unsigned long long sessionID)
	{
		unsigned short index = SessionManager::GetInstance()->GetIndexBySessionID(sessionID);
		_playerArray[index].useFlag = false;
		return true;
	}

	void Update();

	//CMemoryPool<Player, true> playerPool = CMemoryPool<Player, true>(0);
public:
	//std::unordered_map<int, Player*>& GetObjectMap() { return ObjectMap; }
};
