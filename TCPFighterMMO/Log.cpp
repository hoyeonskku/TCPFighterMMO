#include "pch.h"
#include "Log.h"

int g_iLogLevel = dfLOG_LEVEL_DEBUG;  // 초기 로그 레벨 설정
WCHAR g_szLogBuff[1024] = { 0 };      // 로그 버퍼 초기화

void Log(WCHAR* szString, int iLogLevel) 
{
    // 현재 시간을 받아오기 위한 변수
    time_t rawtime;
    struct tm timeinfo;
    WCHAR timeBuffer[64];

    // 현재 시간을 문자열로 변환
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    wcsftime(timeBuffer, sizeof(timeBuffer) / sizeof(WCHAR), L"%Y-%m-%d %H:%M:%S", &timeinfo);

    // 로그 레벨에 따른 문자열
    const WCHAR* logLevelStr = L"";
    switch (iLogLevel)
    {
    case dfLOG_LEVEL_DEBUG:
        logLevelStr = L"DEBUG";
        break;
    case dfLOG_LEVEL_ERROR:
        logLevelStr = L"ERROR";
        break;
    case dfLOG_LEVEL_SYSTEM:
        logLevelStr = L"SYSTEM";
        break;
    }

    // 파일 쓰기 모드로 텍스트 파일 열기
    FILE* pFile = nullptr;
    _wfopen_s(&pFile, dfLOG_FILE_PATH, L"a, ccs=UTF-16LE");

    if (pFile)
    {
        // 파일에 시간, 로그 레벨, 메시지를 기록
        fwprintf(pFile, L"[%s] [%s] %s\n", timeBuffer, logLevelStr, szString);
        fclose(pFile);  // 파일 닫기
    }

    // 콘솔 출력
    wprintf(L"[%s] [%s] %s\n", timeBuffer, logLevelStr, szString);
}

