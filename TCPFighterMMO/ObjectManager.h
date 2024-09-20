#pragma once
#include <list>

class ObjectManager
{
private:
	ObjectManager() {};
	~ObjectManager() {};

	std::list<class Player*> ObjectList;

public:
	static ObjectManager* GetInstance(void)
	{
		static ObjectManager Sys;
		return &Sys;
	}

public:
	std::list<Player*>& GetObjectList() { return ObjectList; }
	void RemovePlayers();
};
