#pragma once
struct Session;

class Player
{
public:
	Session* session = nullptr;
	DWORD sessionId;
	short x, y;
	char dir;
	char hp;
	bool moveFlag = false;
	bool deathFlag = false;
};

// 세션 매니저에 포인터로 전달할 함수
extern Player* CreatePlayer(Session* pSession);
extern void DeletePlayer(Session* pSession);

void DisconnectByPlayer(Player* player);