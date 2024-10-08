#include "pch.h"
#include "Log.h"

int g_iLogLevel = dfLOG_LEVEL_DEBUG;  // �ʱ� �α� ���� ����
WCHAR g_szLogBuff[1024] = { 0 };      // �α� ���� �ʱ�ȭ

void Log(WCHAR* szString, int iLogLevel) 
{
    // ���� �ð��� �޾ƿ��� ���� ����
    time_t rawtime;
    struct tm timeinfo;
    WCHAR timeBuffer[64];

    // ���� �ð��� ���ڿ��� ��ȯ
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    wcsftime(timeBuffer, sizeof(timeBuffer) / sizeof(WCHAR), L"%Y-%m-%d %H:%M:%S", &timeinfo);

    // �α� ������ ���� ���ڿ�
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

    // ���� ���� ���� �ؽ�Ʈ ���� ����
    FILE* pFile = nullptr;
    _wfopen_s(&pFile, dfLOG_FILE_PATH, L"a, ccs=UTF-16LE");

    if (pFile)
    {
        // ���Ͽ� �ð�, �α� ����, �޽����� ���
        fwprintf(pFile, L"[%s] [%s] %s\n", timeBuffer, logLevelStr, szString);
        fclose(pFile);  // ���� �ݱ�
    }

    // �ܼ� ���
    wprintf(L"[%s] [%s] %s\n", timeBuffer, logLevelStr, szString);
}

