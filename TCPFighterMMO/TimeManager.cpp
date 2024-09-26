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

bool TimeManager::CheckLogicFrameTime()
{
	unsigned int currentTick = timeGetTime();
	if (currentTick > nextTick)
	{
		nextTick += logicFrameTime;
		return true;
	}
	return false;
}
bool TimeManager::CheckNetworkFrameTime()
{
	unsigned int currentTick = timeGetTime();
	if (currentTick > nextTick)
	{
		nextTick += networkFrameTime;
		return true;
	}
	return false;
}

void TimeManager::SetCurrentTick()
{
	currentTick = timeGetTime();
}

unsigned int TimeManager::GetCurrentTick()
{
	return currentTick;
}
