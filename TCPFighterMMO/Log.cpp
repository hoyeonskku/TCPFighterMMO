#include "pch.h"
#include "Log.h"
#include <vector>
#include <mutex>
#include <string>

int g_iLogLevel = dfLOG_LEVEL_DEBUG;  // �ʱ� �α� ���� ����
WCHAR g_szLogBuff[1024] = { 0 };      // �α� ���� �ʱ�ȭ

std::vector<std::wstring> g_LogBuffer;

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

    // �α� �޽����� ����
    WCHAR logMessage[1024];
    swprintf(logMessage, 1024, L"[%s] [%s] %s\n", timeBuffer, logLevelStr, szString);
}

// �� �����ӿ� �� �� ȣ���Ͽ� �α׸� ���Ͽ� ����ϴ� �Լ�
void FlushLogBuffer()
{
    // ���� ���� ���� �ؽ�Ʈ ���� ����
    FILE* pFile = nullptr;
    _wfopen_s(&pFile, dfLOG_FILE_PATH, L"a, ccs=UTF-16LE");

    if (pFile)
    {
        // ���ۿ� �ִ� ��� �α� �޽����� ���Ͽ� ���
        for (const auto& log : g_LogBuffer)
        {
            fwprintf(pFile, L"%s", log.c_str());
        }
        fclose(pFile);  // ���� �ݱ�
    }

    // ���� ����
    g_LogBuffer.clear();
}
