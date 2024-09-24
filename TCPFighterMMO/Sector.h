#pragma once
#include "CRingBuffer.h"

#define RINGBUFFERSIZE 20000
class Player;

struct SectorPos
{
	SectorPos() {};
	SectorPos(int y, int x) : y(y), x(x) {};
	short y;
	short x;
};

class Sector
{
public:
	Sector()
	{
	}
	~Sector()
	{
	}

	int x;
	int y;
	std::unordered_map<int, Player*> _playerMap;
};

