#include "pch.h"
#include "TimeManager.h"
#include "NetworkManager.h"
#include "NetworkLogic.h"
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "winmm.lib")

TimeManager::TimeManager()
{
	timeBeginPeriod(1);
	frameTick = timeGetTime();
	nextTick = frameTick + logicFrameTime;
}

bool TimeManager::CheckLogicFrameTime()
{
	currentTick = timeGetTime();
	if (currentTick > nextTick)
	{
		fps++;
		if (currentTick - frameTick >= 1000)
		{
			// 서버 모니터 출력
			{
				int networkFPS = NetworkManager::GetInstance()->networkFps;
				int syncCount = g_SyncCount;

				std::cout << "======================================" << std::endl;
				std::cout << "networkFPS : " << networkFPS << std::endl;
				std::cout << "contentsFPS : " << fps << std::endl;
				std::cout << "syncCount : " << g_SyncCount << std::endl;
				std::cout << "======================================\n" << std::endl;
			}
			frameTick = nextTick;
			// 프레임 초기화
			fps = 0;
			NetworkManager::GetInstance()->networkFps = 0;
		}
		nextTick += logicFrameTime;
		return true;
	}
	return false;
}

unsigned int TimeManager::GetCurrentTick()
{
	return currentTick;
}
