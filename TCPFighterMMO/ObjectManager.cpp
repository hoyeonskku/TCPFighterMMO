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
		// �α� ���� ����ó�� -> �ý����� �ּ�ȭ
		FlushLogBuffer();
	}
}
