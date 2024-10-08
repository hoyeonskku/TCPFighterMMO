#pragma once
#include <cstdio>
#include <ctime>

// �α� ���� ����
#define dfLOG_LEVEL_DEBUG 0
#define dfLOG_LEVEL_ERROR 1
#define dfLOG_LEVEL_SYSTEM 2

extern int g_iLogLevel;
extern WCHAR g_szLogBuff[1024];
// �α� ���� ���
#define dfLOG_FILE_PATH L"serverLog.txt"

// �α� �Լ� ����
void Log(WCHAR* szString, int iLogLevel);

// ��ũ�� ����
#define _LOG(LogLevel, fmt, ...)								   \
do {															   \
	if (g_iLogLevel <= LogLevel)								   \
	{															   \
		wsprintf(g_szLogBuff, fmt, ##__VA_ARGS__);				   \
		Log(g_szLogBuff, LogLevel);								   \
	}															   \
} while (0)
