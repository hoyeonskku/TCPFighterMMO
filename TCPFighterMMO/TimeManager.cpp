#include "pch.h"
#include "TimeManager.h"
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
	currentTick = timeGetTime();
	if (currentTick > nextTick)
	{
		//fps++;
		//if (currentTick - frameTick >= 1000)
		//{
		//	//std::cout << fps << std::endl;
		//	frameTick = currentTick;
		//	fps = 0;
		//}
		nextTick += logicFrameTime;
		return true;
	}
	return false;
}

unsigned int TimeManager::GetCurrentTick()
{
	return currentTick;
}
