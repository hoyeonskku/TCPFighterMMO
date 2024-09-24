#pragma once
#include "CRingBuffer.h"

#define RINGBUFFERSIZE 20000
class Player;

struct SectorPos
{
	short x;
	short y;
};

class Sector
{
public:
	Sector()
	{
		contentsBuffer = new CRingBuffer(RINGBUFFERSIZE);
		lifeCycleBuffer = new CRingBuffer(RINGBUFFERSIZE);
	}
	~Sector()
	{
		delete contentsBuffer;
		delete lifeCycleBuffer;
	}
	CRingBuffer* contentsBuffer;
	CRingBuffer* lifeCycleBuffer;

	int x;
	int y;
	std::unordered_map<int, Player*> _playerMap;
	std::list<Player*> _createBufferMap;
	std::list<Player*> _deleteBufferMap;
};

