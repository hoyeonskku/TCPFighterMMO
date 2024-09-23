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

	// 세션 제거, 삭제 시의 콜백함수를 받아옴
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
	
	// 세션 삭제시 플레이어 삭제 콜백을 호출
	bool ReserveDeleteSession(Session* session)
	{

		return true;
	}


	// 리스트에서 삭제할 세션을 찾아 제거하는 메서드
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
