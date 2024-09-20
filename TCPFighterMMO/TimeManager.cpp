#include "TimeManager.h"
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "winmm.lib")

void TimeManager::TimeInit()
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
