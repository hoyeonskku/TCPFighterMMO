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

	// 세션 제거, 삭제 시의 콜백함수를 받아옴
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

	// 세션 삭제시 플레이어 삭제 콜백을 호출
	bool DeleteSession(Session* session)
	{
		if (sessionDeleteCallback)
		{
			sessionDeleteCallback(session);  // 콜백 함수 호출
		}
		// 세션 삭제
		delete session;
		return true;
	}

	void DelayedSessionDelete(Session* session)
	{
		session->deathFlag = true;
	}

	// 리스트에서 삭제할 세션을 찾아 제거하는 메서드
	void RemoveSessions() {
		auto it = userSessionList.begin();
		while (it != userSessionList.end())
		{
			if ((*it)->deathFlag)
			{
				DeleteSession(*it);  // 세션 삭제 및 콜백 호출
				it = userSessionList.erase(it);  // 리스트에서 세션 제거
			}
			else
			{
				++it;  // 다음 요소로 이동
			}
		}
	}

public:
	std::list<Session*>& GetSessionList() { return userSessionList; }
};
