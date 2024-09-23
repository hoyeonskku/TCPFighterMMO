#pragma once
#include "pch.h"

int g_iLogLevel;
WCHAR g_szLogBuff[1024];

#define dfLOG_LEVEL_DEBUG 0
#define dfLOG_LEVEL_ERROR 1
#define dfLOG_LEVEL_SYSTEM 2

void Log(WCHAR* szString, int iLogLevel)
{
	wprintf(L"%s \n");
}

#define _LOG(LogLevel, fmt, ...)								   \
do {															   \
	if (g_iLogLevel <= LogLevel)								   \
	{															   \
		wsprintf(g_szLogBuff, fmt, ##__VA_ARGS__);				   \
		Log(g_szLogBuff, LogLevel);								   \
	}															   \
} while (0)														   