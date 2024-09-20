#include "pch.h"
#include "ObjectManager.h"

void ObjectManager::RemovePlayers()
{
	for (auto it = ObjectList.begin(); it != ObjectList.end();)
	{
		if ((*it)->deathFlag == true)
		{
			delete* it;
			it = ObjectList.erase(it);
		}
		else
			++it;
	}
}
