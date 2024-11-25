#include "pch.h"
#include "ObjectManager.h"
#include "NetworkManager.h"
#include "TimeManager.h"
#include "Player.h"
#include "NetworkLogic.h"

void ObjectManager::Update()
{
	for (int i = 0; i < PLAYERMAXCOUNT; i++)
	{
		if (_playerArray[i].useFlag == true)
			_playerArray[i].Update();
	}
	// 로그 버퍼 지연처리 -> 시스템콜 최소화
	FlushLogBuffer();
}
