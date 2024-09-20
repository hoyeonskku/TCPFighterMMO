#pragma once
#include <list>
#include "Session.h"

struct Player;

class SessionManager
{
private:
	SessionManager() {};
	~SessionManager() {};

	std::list<struct Session*> userSessionList;
	void (*sessionDeleteCallback)(Session*) = nullptr;

public:
	static SessionManager* GetInstance(void)
	{
		static SessionManager Sys;
		return &Sys;
	}

	// ���� ����, ���� ���� �ݹ��Լ��� �޾ƿ�
	void Init(void (*deleteCallback)(Session*))
	{
		sessionDeleteCallback = deleteCallback;
	}

	Session* CreateSession()
	{
		Session* newSession = new Session;

		userSessionList.push_back(newSession);
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

	void DelayedSessionDelete(Session* session)
	{
		session->deathFlag = true;
	}

	// ����Ʈ���� ������ ������ ã�� �����ϴ� �޼���
	void RemoveSessions() {
		auto it = userSessionList.begin();
		while (it != userSessionList.end())
		{
			if ((*it)->deathFlag)
			{
				DeleteSession(*it);  // ���� ���� �� �ݹ� ȣ��
				it = userSessionList.erase(it);  // ����Ʈ���� ���� ����
			}
			else
			{
				++it;  // ���� ��ҷ� �̵�
			}
		}
	}

public:
	std::list<Session*>& GetSessionList() { return userSessionList; }
};
