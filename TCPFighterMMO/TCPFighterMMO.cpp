#pragma once
#include "pch.h"
#include "NetworkManager.h"
#include "SessionManager.h"

bool g_bShutdown = false;

int main()
{
	// 초기화 부분
	// 패킷 처리 함수 오버로딩을 위한 의존성 주입
	/*NetworkManager::GetInstance()->netInit(new ContentPacketProcessor);
	SessionManager::GetInstance()->Init(ClearUser);*/

	while (!g_bShutdown)
	{
		NetworkManager::GetInstance()->netIOProcess();
	}
}
