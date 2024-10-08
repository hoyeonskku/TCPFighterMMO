#pragma once
#include "Session.h"
#include "CObjectPool.h"

class SessionManager
{
private:
	SessionManager() {};
	~SessionManager() {};

	std::unordered_map<int, Session*> _sessionMap;
	std::list<int> _toDeletedSessionId;
	CMemoryPool<Session, true> sessionPool = CMemoryPool<Session, true>(0);
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
		Session* newSession = sessionPool.Alloc();
		newSession->Clear();

		newSession->sessionID = _id++;
		_sessionMap.insert(std::make_pair( newSession->sessionID, newSession));
		if (sessionCreateCallback)
		{
			sessionCreateCallback(newSession);  // �ݹ� �Լ� ȣ��
		}
		return newSession;
	}

	// ���� ������ �÷��̾� ���� �ݹ��� ȣ��
	bool DeleteSession(Session* session)
	{
		if (sessionDeleteCallback)
		{
			sessionDeleteCallback(session);  // �ݹ� �Լ� ȣ��
		}
		// ���� ����
		_sessionMap.erase(session->sessionID);	
		closesocket(session->socket);
		sessionPool.Free(session);
		return true;
	}
	
	bool ReserveDeleteSession(Session* session)
	{
		_toDeletedSessionId.push_back(session->sessionID);
		return true;
	}


	// ����Ʈ���� ������ ������ ã�� �����ϴ� �޼���
	void RemoveSessions() {
		for (auto sessionId : _toDeletedSessionId)
			DeleteSession(_sessionMap[sessionId]);
		
		_toDeletedSessionId.clear();
	}

public:
	std::unordered_map<int, Session*>& GetSessionMap() { return _sessionMap; }
	int _id = 0;
};
