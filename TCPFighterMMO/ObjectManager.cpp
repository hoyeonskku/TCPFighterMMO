#include "pch.h"
#include "ObjectManager.h"
#include "NetworkManager.h"
#include "TimeManager.h"
#include "Player.h"
#include "NetworkLogic.h"

void ObjectManager::Update()
{/*
	if (TimeManager::GetInstance()->CheckLogicFrameTime())*/
	{
		for (auto& pair : ObjectMap)
			pair.second->Update();
		// 로그 버퍼 지연처리 -> 시스템콜 최소화
		FlushLogBuffer();
	}
}
