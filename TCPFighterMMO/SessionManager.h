#pragma once
#include "Session.h"
#include "CObjectPool.h"

class SessionManager
{
private:
	SessionManager() {};
	~SessionManager() {};

	std::unordered_map<int, Session*> _sessionMap;
	// 지연 삭제될 세션 리스트
	std::list<int> _toDeletedSessionId;
	CMemoryPool<Session, true> sessionPool = CMemoryPool<Session, true>(0);

	// 세션 생성 및 삭제 시에 사용될 콜백, 컨텐츠에서 정의
	void (*sessionDeleteCallback)(Session*) = nullptr;
	void (*sessionCreateCallback)(Session*) = nullptr;

public:
	static SessionManager* GetInstance(void)
	{
		static SessionManager Sys;
		return &Sys;
	}

	// 세션 생성, 삭제 시의 콜백함수를 받아옴
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
			sessionCreateCallback(newSession);  // 콜백 함수 호출
		}
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


	// 리스트에서 삭제할 세션을 찾아 제거하는 메서드
	void RemoveSessions() {
		for (auto sessionId : _toDeletedSessionId)
			DeleteSession(_sessionMap[sessionId]);
		
		_toDeletedSessionId.clear();
	}

public:
	std::unordered_map<int, Session*>& GetSessionMap() { return _sessionMap; }
	// 세션 아이디 발급용
	int _id = 0;
};
