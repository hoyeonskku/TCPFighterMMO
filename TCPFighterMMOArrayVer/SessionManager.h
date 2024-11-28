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

	// 지연 삭제될 세션 리스트
	//std::list<int> _toDeletedSessionId;
	//CMemoryPool<Session, true> sessionPool = CMemoryPool<Session, true>(0);

	// 세션 생성 및 삭제 시에 사용될 콜백, 컨텐츠에서 정의
	void (*sessionDeleteCallback)(Session*) = nullptr;
	void (*sessionCreateCallback)(Session*) = nullptr;

	int _sessionCount = 0;

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
			sessionCreateCallback(newSession);  // 콜백 함수 호출
		}
		return newSession;
	}

	// 세션 삭제시 플레이어 삭제 콜백을 호출
	bool DeleteSession(Session* session)
	{
		// 세션 반납
		--_sessionCount;
		_freeArray[_sessionCount] = static_cast<USHORT>((session->sessionID >> 48) & 0xFFFF);
		if (sessionDeleteCallback)
		{
			sessionDeleteCallback(session);  // 콜백 함수 호출
		}
		session->useFlag = false;
		session->deathFlag = false;
		closesocket(session->socket);
		session->socket = INVALID_SOCKET;
		return true;
	}

	// 리스트에서 삭제할 세션을 찾아 제거하는 메서드
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
	// 세션 아이디 발급용
	int _id = 0;

};
