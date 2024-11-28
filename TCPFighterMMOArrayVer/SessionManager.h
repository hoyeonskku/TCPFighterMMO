#pragma once
#include "Session.h"
#include "CObjectPool.h"

#define SESSIONMAXCOUNT 10300

class SessionManager
{
private:
	SessionManager()
	{
		for (int i = 0; i < SESSIONMAXCOUNT; i++)
		{
			_freeArray[i] = i;
		}
	};
	~SessionManager() {};

	//std::unordered_map<int, Session*> _sessionMap;
	Session _sessionArray[SESSIONMAXCOUNT];
	unsigned long long _freeArray[SESSIONMAXCOUNT];

	// ���� ������ ���� ����Ʈ
	//std::list<int> _toDeletedSessionId;
	//CMemoryPool<Session, true> sessionPool = CMemoryPool<Session, true>(0);

	// ���� ���� �� ���� �ÿ� ���� �ݹ�, ���������� ����
	void (*sessionDeleteCallback)(Session*) = nullptr;
	void (*sessionCreateCallback)(Session*) = nullptr;

	int _sessionCount = 0;

public:
	static SessionManager* GetInstance(void)
	{
		static SessionManager Sys;
		return &Sys;
	}

	// ���� ����, ���� ���� �ݹ��Լ��� �޾ƿ�
	void Init(void (*createCallback)(Session*), void (*deleteCallback)(Session*))
	{
		sessionCreateCallback = createCallback;
		sessionDeleteCallback = deleteCallback;
	}

	short GetIndexBySessionID(unsigned long long sessionID)
	{
		return static_cast<USHORT>((sessionID >> 48) & 0xFFFF);
	}

	Session* CreateSession()
	{
		Session* newSession = &_sessionArray[_freeArray[_sessionCount]];
		newSession->Clear(_freeArray[_sessionCount]);
		_sessionCount++;

		newSession->useFlag = true;
		if (sessionCreateCallback)
		{
			sessionCreateCallback(newSession);  // �ݹ� �Լ� ȣ��
		}
		return newSession;
	}

	// ���� ������ �÷��̾� ���� �ݹ��� ȣ��
	bool DeleteSession(Session* session)
	{
		// ���� �ݳ�
		--_sessionCount;
		_freeArray[_sessionCount] = static_cast<USHORT>((session->sessionID >> 48) & 0xFFFF);
		if (sessionDeleteCallback)
		{
			sessionDeleteCallback(session);  // �ݹ� �Լ� ȣ��
		}
		session->useFlag = false;
		session->deathFlag = false;
		closesocket(session->socket);
		session->socket = INVALID_SOCKET;
		return true;
	}

	// ����Ʈ���� ������ ������ ã�� �����ϴ� �޼���
	void RemoveSessions()
	{
		for (auto& session : _sessionArray)
			if (session.deathFlag == true)
				DeleteSession(&session);
	}
	
	int GetSessionCount() { return _sessionCount; }

public:
	//std::unordered_map<int, Session*>& GetSessionMap() { return _sessionMap; }
	Session* GetSessionByIndex(int index) { return &_sessionArray[index];  }
	Session* GetSessionArray() { return _sessionArray;  }
	// ���� ���̵� �߱޿�
	int _id = 0;

};
