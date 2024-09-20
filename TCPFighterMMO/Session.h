#pragma once
#include "CRingBuffer.h"
#include <WinSock2.h>
#define _WINSOCKAPI_
#include <windows.h>

#define BUFFERSIZE 2048

struct Session
{
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
};