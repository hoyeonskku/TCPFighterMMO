#include "pch.h"
#include "TimeManager.h"
#include "NetworkManager.h"
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "winmm.lib")

void TimeManager::Init()
{
	timeBeginPeriod(1);
	nextTick = timeGetTime();
	frameTick = nextTick;
}

bool TimeManager::CheckLogicFrameTime()
{
	prevTick = currentTick;
	currentTick = timeGetTime();
	//std::cout << currentTick - prevTick << std::endl;
	if (currentTick > nextTick)
	{
		currentFrameCount++;
		fps++;
		if (currentTick - frameTick >= 1000)
		{
			std::cout << fps << std::endl;
			int networkFps = NetworkManager::GetInstance()->networkFps;
			std::cout << networkFps << std::endl;
			NetworkManager::GetInstance()->networkFps = 0;
			
			frameTick = currentTick;
			fps = 0;
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
