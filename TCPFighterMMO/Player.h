#pragma once
struct Session;

class Player
{
public:
	Session* session = nullptr;
	DWORD sessionID;
	short x, y;
	char dir;
	char hp;
	bool moveFlag = false;
};

// 세션 매니저에 포인터로 전달할 함수
extern void CreatePlayer(Session* pSession);
extern void DeletePlayer(Session* pSession);