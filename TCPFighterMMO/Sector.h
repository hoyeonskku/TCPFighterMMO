#pragma once
#include "CRingBuffer.h"

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
	Sector() {}
	~Sector() {}

	int x;
	int y;
	std::unordered_set<Player*> _playerSet;
};

