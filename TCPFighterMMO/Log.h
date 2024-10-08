#pragma once
#include <cstdio>
#include <ctime>

// 로그 레벨 정의
#define dfLOG_LEVEL_DEBUG 0
#define dfLOG_LEVEL_ERROR 1
#define dfLOG_LEVEL_SYSTEM 2

extern int g_iLogLevel;
extern WCHAR g_szLogBuff[1024];
// 로그 파일 경로
#define dfLOG_FILE_PATH L"serverLog.txt"

// 로그 함수 선언
void Log(WCHAR* szString, int iLogLevel);

// 매크로 정의
#define _LOG(LogLevel, fmt, ...)								   \
do {															   \
	if (g_iLogLevel <= LogLevel)								   \
	{															   \
		wsprintf(g_szLogBuff, fmt, ##__VA_ARGS__);				   \
		Log(g_szLogBuff, LogLevel);								   \
	}															   \
} while (0)
