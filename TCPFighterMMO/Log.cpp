#include "pch.h"
#include "Log.h"
#include <vector>
#include <mutex>
#include <string>

int g_iLogLevel = dfLOG_LEVEL_DEBUG;  // 초기 로그 레벨 설정
WCHAR g_szLogBuff[1024] = { 0 };      // 로그 버퍼 초기화

std::vector<std::wstring> g_LogBuffer;

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

    // 로그 메시지를 생성
    WCHAR logMessage[1024];
    swprintf(logMessage, 1024, L"[%s] [%s] %s\n", timeBuffer, logLevelStr, szString);
}

// 한 프레임에 한 번 호출하여 로그를 파일에 기록하는 함수
void FlushLogBuffer()
{
    // 파일 쓰기 모드로 텍스트 파일 열기
    FILE* pFile = nullptr;
    _wfopen_s(&pFile, dfLOG_FILE_PATH, L"a, ccs=UTF-16LE");

    if (pFile)
    {
        // 버퍼에 있는 모든 로그 메시지를 파일에 기록
        for (const auto& log : g_LogBuffer)
        {
            fwprintf(pFile, L"%s", log.c_str());
        }
        fclose(pFile);  // 파일 닫기
    }

    // 버퍼 비우기
    g_LogBuffer.clear();
}
