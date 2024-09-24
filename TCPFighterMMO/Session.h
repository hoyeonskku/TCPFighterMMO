#pragma once
#include "CRingBuffer.h"

#define BUFFERSIZE 20000

class Session
{
public:
	Session()
	{
		sendBuffer = new CRingBuffer(BUFFERSIZE);
		recvBuffer = new CRingBuffer(BUFFERSIZE);
	}
	~Session()
	{
		delete sendBuffer;
		delete recvBuffer;
	}
	SOCKET socket;
	unsigned short port;
	char ip[16];
	CRingBuffer* sendBuffer;
	CRingBuffer* recvBuffer;
	bool deathFlag = false;
	DWORD dwLastRecvTime;
	DWORD sessionID;
};