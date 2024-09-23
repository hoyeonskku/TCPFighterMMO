#pragma once
#include "pch.h"
#include "NetworkManager.h"
#include "SessionManager.h"
#include "ContentPacketProcessor.h"

bool g_bShutdown = false;

int main()
{
	// 초기화 부분
	// 패킷 처리 함수 오버로딩을 위한 의존성 주입
	NetworkManager::GetInstance()->netInit(new ContentPacketProcessor);
	//SessionManager::GetInstance()->Init();

	while (!g_bShutdown)
	{
		try
		{
			NetworkManager::GetInstance()->netIOProcess();
			Update();
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
