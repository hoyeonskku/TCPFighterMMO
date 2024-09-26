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
	TimeManager::GetInstance()->Init();

	while (!g_bShutdown)
	{
		try
		{
			NetworkManager::GetInstance()->netIOProcess();
			TimeManager::GetInstance()->SetCurrentTick();
			ObjectManager::GetInstance()->Update();
		}
		catch (const std::runtime_error& e)
		{
			std::cerr << "Caught an exception: " << e.what() << std::endl;
			DebugBreak();
		}
	}
	NetworkManager::GetInstance()->netCleanUp();
	return 0;
}
