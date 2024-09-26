#pragma once
#include "Sector.h"

class Session;

class Player
{
public:
	Session* session = nullptr;
	DWORD sessionID;
	short x, y;
	char dir;
	char hp;
	bool moveFlag = false;

	SectorPos sectorPos;
	DWORD lastProcTime;

	void Update();

	void Clear(){};
};

// ���� �Ŵ����� �����ͷ� ������ �Լ�
extern void CreatePlayer(Session* pSession);
extern void DeletePlayer(Session* pSession);