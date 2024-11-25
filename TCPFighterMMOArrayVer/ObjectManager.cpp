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
	// �α� ���� ����ó�� -> �ý����� �ּ�ȭ
	FlushLogBuffer();
}
