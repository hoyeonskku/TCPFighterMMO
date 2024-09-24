#include "pch.h"
#include "ObjectManager.h"
#include "TimeManager.h"
#include "Player.h"

void ObjectManager::Update()
{
	if (TimeManager::GetInstance()->CheckLogicFrameTime())
	{
		for (auto& pair : ObjectMap)
			pair.second->Update();
	}
}
