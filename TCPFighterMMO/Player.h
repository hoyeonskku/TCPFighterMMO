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

// ���� �Ŵ����� �����ͷ� ������ �Լ�
extern void CreatePlayer(Session* pSession);
extern void DeletePlayer(Session* pSession);