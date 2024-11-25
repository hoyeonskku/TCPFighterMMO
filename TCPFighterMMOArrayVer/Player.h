#pragma once
#include "Sector.h"

class Session;

class Player
{
public:
	unsigned long long _sessionID;

	short x, y;
	char dir;
	char hp;
	bool moveFlag = false;
	bool useFlag = false;

	SectorPos sectorPos;
	DWORD lastProcTime;
	Session* session;
	

	void Update();

	void Clear()
	{
		moveFlag = false;
	};



	short GetIndex()
	{
		return static_cast<USHORT>((_sessionID >> 48) & 0xFFFF);
	}

	static const int direction[8][2];
	static const int sectorDir[9][2];
};



// 세션 매니저에 포인터로 전달할 함수
extern void CreatePlayer(Session* pSession);
extern void DeletePlayer(Session* pSession);