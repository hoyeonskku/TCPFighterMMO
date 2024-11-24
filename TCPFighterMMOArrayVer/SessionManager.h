#pragma once
#include "Session.h"
#include "CObjectPool.h"

#define SESSIONMAXCOUNT 8000

class SessionManager
{
private:
	SessionManager()
	{
		for (int i = 0; i < SESSIONMAXCOUNT; i++)
		{
			_freeArray[i] = i;
			_sessionArray[i]._sessionIndex = i;
		}
	};
	~SessionManager() {};

	//std::unordered_map<int, Session*> _sessionMap;
	Session _sessionArray[SESSIONMAXCOUNT];
	int _freeArray[SESSIONMAXCOUNT];
	int _freeIndex = 0;

	// ���� ������ ���� ����Ʈ
	//std::list<int> _toDeletedSessionId;
	//CMemoryPool<Session, true> sessionPool = CMemoryPool<Session, true>(0);

	// ���� ���� �� ���� �ÿ� ���� �ݹ�, ���������� ����
	void (*sessionDeleteCallback)(Session*) = nullptr;
	void (*sessionCreateCallback)(Session*) = nullptr;

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

	Session* CreateSession()
	{
		Session* newSession = &_sessionArray[_freeArray[_freeIndex]];
		_freeIndex++;
		newSession->Clear();

		newSession->sessionID = _id++;
		newSession->useFlag = true;
		//if (sessionCreateCallback)
		//{
		//	sessionCreateCallback(newSession);  // �ݹ� �Լ� ȣ��
		//}
		_sessionCount++;
		return newSession;
	}

	// ���� ������ �÷��̾� ���� �ݹ��� ȣ��
	bool DeleteSession(Session* session)
	{
		//if (sessionDeleteCallback)
		//{
		//	sessionDeleteCallback(session);  // �ݹ� �Լ� ȣ��
		//}
		// ���� �ݳ�
		--_freeIndex;
		_freeArray[_freeIndex] = session->_sessionIndex;
		session->useFlag = false;
		session->deathFlag = false;
		closesocket(session->socket);
		session->socket = INVALID_SOCKET;
		_sessionCount--;
		if (_sessionCount < 0)
			DebugBreak();
		return true;
	}

	// ����Ʈ���� ������ ������ ã�� �����ϴ� �޼���
	void RemoveSessions()
	{
		for (auto& session : _sessionArray)
			if (session.deathFlag == true)
				DeleteSession(&session);
	}

public:
	//std::unordered_map<int, Session*>& GetSessionMap() { return _sessionMap; }
	Session* GetSessionByIndex(int index) { return &_sessionArray[index];  }
	Session* GetSessionArray() { return _sessionArray;  }
	// ���� ���̵� �߱޿�
	int _id = 0;
	int _sessionCount = 0;
};
