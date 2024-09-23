#pragma once
#include "Session.h"

struct Player;

class SessionManager
{
private:
	SessionManager() {};
	~SessionManager() {};

	std::unordered_map<int, Session*> _sessionMap;
	std::list<int> _toDeletedSessionId;
	void (*sessionDeleteCallback)(Session*) = nullptr;

public:
	static SessionManager* GetInstance(void)
	{
		static SessionManager Sys;
		return &Sys;
	}

	// ���� ����, ���� ���� �ݹ��Լ��� �޾ƿ�
	//void Init(void (*deleteCallback)(Session*))
	//sessionDeleteCallback = deleteCallback;
	void Init()
	{
	}

	Session* CreateSession()
	{
		Session* newSession = new Session;

		_sessionMap[newSession->sessionID] = newSession;
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
		delete session;
		return true;
	}
	
	// ���� ������ �÷��̾� ���� �ݹ��� ȣ��
	bool ReserveDeleteSession(Session* session)
	{

		return true;
	}


	// ����Ʈ���� ������ ������ ã�� �����ϴ� �޼���
	void RemoveSessions() {
		for (auto sessionId : _toDeletedSessionId)
		{
			_sessionMap.erase(sessionId);
		}
	}

public:
	std::unordered_map<int, Session*>& GetSessionMap() { return _sessionMap; }
	std::list<int>& GetDeletedSessionList() { return _toDeletedSessionId; }
};
