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

// ���� �Ŵ����� �����ͷ� ������ �Լ�
extern Player* CreatePlayer(Session* pSession);
extern void DeletePlayer(Session* pSession);

void DisconnectByPlayer(Player* player);