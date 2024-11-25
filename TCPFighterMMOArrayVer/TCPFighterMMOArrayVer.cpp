#pragma once
#include "pch.h"
#include "ObjectManager.h"
#include "NetworkManager.h"
#include "SessionManager.h"
#include "SectorManager.h"
#include "TimeManager.h"
#include "ContentPacketProcessor.h"
#include "Player.h"

bool g_bShutdown = false;
int main()
{
	// 초기화 부분
	// 패킷 처리 함수 오버로딩을 위한 의존성 주입
	NetworkManager::GetInstance()->netInit(new ContentPacketProcessor);
	SessionManager::GetInstance()->Init(CreatePlayer, DeletePlayer);
	SectorManager::GetInstance()->Init();

	while (!g_bShutdown)
	{
		if (!TimeManager::GetInstance()->CheckLogicFrameTime())
			continue;
		// 네트워크 루프
		NetworkManager::GetInstance()->netIOProcess();

		// 컨텐츠 루프
		ObjectManager::GetInstance()->Update();
	}
	NetworkManager::GetInstance()->netCleanUp();
	return 0;
}
