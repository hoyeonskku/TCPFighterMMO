#include "pch.h"
#include "TimeManager.h"
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "winmm.lib")

void TimeManager::Init()
{
	timeBeginPeriod(1);
	nextTick = timeGetTime();
}

bool TimeManager::CheckFrameTime()
{
	unsigned int currentTick = timeGetTime();
	if (currentTick > nextTick)
	{
		nextTick += frameTime;
		return true;
	}
	return false;
}
