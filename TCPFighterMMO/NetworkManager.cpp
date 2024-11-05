#include "pch.h"
#include "NetworkManager.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <winsock2.h>
#define _WINSOCKAPI_
#include <windows.h>
#include <ws2tcpip.h>
#include "PacketDefine.h"
#include "Session.h"
#include "netPacketProcs.h"
#include "MakePacket.h"
#include "SessionManager.h"
#include "TimeManager.h"
#include "ObjectManager.h"
#include "SerializingBuffer.h"
#include "SerializingBufferManager.h"

extern bool g_bShutdown;

NetworkManager* NetworkManager::GetInstance()
{
	static NetworkManager instance; // �̱��� �ν��Ͻ� ����
	return &instance;
}

int NetworkManager::netInit(IPacketProcessor* customProcessor)
{
	// ��Ŷ ���μ��� ���
	packetProcessor = customProcessor;

	int WSAStartUpRetval;
	int SocketRetval;
	int IoctlsocketRetval;
	int BindRetval;
	int ListenRetval;
	int WSAStartUpError;
	int SocketError;
	int IoctlsocketError;
	int BindError;
	int ListenError;
	WSADATA wsaData;

	WSAStartUpRetval = ::WSAStartup(MAKEWORD(2, 2), OUT & wsaData);
	if (WSAStartUpRetval != 0)
	{
		WSAStartUpError = WSAGetLastError();
		_LOG(dfLOG_LEVEL_ERROR, L"WSAStartup() error, code %d", WSAStartUpError);
		g_bShutdown = false;
		return 0;
	}

	listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"socket Error");
		g_bShutdown = false;
		return 0;
	}

	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(SERVERPORT);

	BindRetval = ::bind(listenSocket, (SOCKADDR*)(&myAddress), sizeof(myAddress));
	if (BindRetval == SOCKET_ERROR)
	{
		BindError = WSAGetLastError();
		_LOG(dfLOG_LEVEL_ERROR, L"bind() error, code %d", BindError);
		g_bShutdown = false;
		return 0;
	}

	u_long on = 1;
	IoctlsocketRetval = ::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET;

	if (IoctlsocketRetval == SOCKET_ERROR)
	{
		IoctlsocketError = WSAGetLastError();
		_LOG(dfLOG_LEVEL_ERROR, L"ioctlsocket() error, code %d", IoctlsocketError);
		g_bShutdown = false;
		return 0;
	}

	ListenRetval = ::listen(listenSocket, SOMAXCONN_HINT(65535));
	//ListenRetval = ::listen(listenSocket, SOMAXCONN);
	if (ListenRetval == SOCKET_ERROR)
	{
		ListenError = WSAGetLastError();
		_LOG(dfLOG_LEVEL_ERROR, L"listen() error, code %d", ListenError);
		g_bShutdown = false;
		return 0;
	}
}

void NetworkManager::netCleanUp()
{
	WSACleanup();
}

void NetworkManager::netIOProcess()
{
	// ��Ʈ��ũ ������ ����
	networkFps++;

	FD_SET rSet;
	FD_SET wSet;

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	std::unordered_map<int, Session*>& sessionList = SessionManager::GetInstance()->GetSessionMap();

	int sessionListSize = sessionList.size();

	struct timeval timeout;
	timeout.tv_sec = 0;  // �� ���� ����
	timeout.tv_usec = 0; // ����ũ���� ���� ����

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	char addr_str[INET_ADDRSTRLEN];

	int SelectRetval;
	int SelectError;

	auto setIter = sessionList.begin();
	auto isSetIter = sessionList.begin();
	// 64�� ���Ǿ� ��ȸ
	while (sessionListSize >= 64)
	{
		FD_ZERO(&rSet);
		FD_ZERO(&wSet);
		// �¿� ���� �ϳ��� ���
		for (int i = 0; i < 64; i++)
		{
			FD_SET((*setIter).second->socket, &rSet);
			// ���� �����Ͱ� ���������� wSet�� ���
			if ((*setIter).second->sendBuffer->GetUseSize() != 0)
				FD_SET((*setIter).second->socket, &wSet);
			setIter++;
		}
		sessionListSize -= 64;
		SelectRetval = select(0, &rSet, &wSet, nullptr, &timeout);
		if (SelectRetval == -1)
		{
			SelectError = WSAGetLastError();
			_LOG(dfLOG_LEVEL_ERROR, L"select() error, code %d", SelectError);
			DebugBreak();
			g_bShutdown = false;
		}
		// ������ ���� ��ȸ
		for (int j = 0; j < 64; j++)
		{
			if (FD_ISSET((*isSetIter).second->socket, &rSet))
			{
				SelectRetval--;
				netProc_Recv((*isSetIter).second);
			}
			if (FD_ISSET((*isSetIter).second->socket, &wSet))
			{
				SelectRetval--;
				if ((*isSetIter).second->deathFlag != true)
				{
					netProc_Send((*isSetIter).second);
				}
			}
			isSetIter++;
			// ���� Ȯ���ߴٸ� ���� ������ �̵�, ���ͷ����� ���� ����
			if (SelectRetval == 0)
			{
				isSetIter = setIter;
				break;
			}
		}
	}
	// ������ ����(if�� �˻縦 ���ֱ� ���� ������ ������ ������ ���� �õ�
	FD_ZERO(&rSet);
	FD_ZERO(&wSet);
	
	for (int i = 0; i < sessionListSize; i++)
	{
		FD_SET((*setIter).second->socket, &rSet);

		if ((*setIter).second->sendBuffer->GetUseSize() != 0)
			FD_SET((*setIter).second->socket, &wSet);
		setIter++;
	}
	FD_SET(listenSocket, &rSet);

	SelectRetval = select(0, &rSet, &wSet, nullptr, &timeout);
	if (SelectRetval == -1)
	{
		SelectError = WSAGetLastError();
		_LOG(dfLOG_LEVEL_ERROR, L"select() error, code %d", SelectError);
		DebugBreak();
		g_bShutdown = false;
	}
	// ������ ���� ��ȸ
	int readCount = 0;
	int writeCount = 0;
	for (int j = 0; j < sessionListSize; j++)
	{
		if (FD_ISSET((*isSetIter).second->socket, &rSet))
		{
			SelectRetval--;
			readCount++;
			netProc_Recv((*isSetIter).second);
		}

		if (FD_ISSET((*isSetIter).second->socket, &wSet))
		{
			SelectRetval--;
			writeCount++;
			if ((*isSetIter).second->deathFlag != true)
			{
				netProc_Send((*isSetIter).second);
			}
		}
		isSetIter++;
	}
	// ���������� ����¿� �ִ� ��� accept ��û�� ����.
	if (FD_ISSET(listenSocket, &rSet))
	{
		SelectRetval--;
		netProc_Accept();
	}

	SessionManager::GetInstance()->RemoveSessions();
}

void NetworkManager::netProc_Accept()
{
	char addr_str[INET_ADDRSTRLEN];
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	int acceptError;
	while (true)
	{
		SOCKET clientSock = accept(listenSocket, (SOCKADDR*)&clientaddr, &addrlen);
		if (clientSock == INVALID_SOCKET)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK) 
				// �� �̻� ó���� Ŭ���̾�Ʈ�� �����Ƿ� ���� ����
				break;
			std::cout << "accept() error" << std::endl;
			std::cout << WSAGetLastError() << std::endl;
			DebugBreak();
			g_bShutdown = false;
			return;
		}
		// ���� ����
		Session* session = SessionManager::GetInstance()->CreateSession();
		session->socket = clientSock;

		inet_ntop(AF_INET, &clientaddr.sin_addr, addr_str, sizeof(addr_str));
		session->port = ntohs(clientaddr.sin_port);
		strcpy_s(session->ip, sizeof(session->ip), addr_str);
	}
}

void NetworkManager::SendUnicast(Session* session, st_PACKET_HEADER* pHeader, CPacket* pPacket)
{
	session->sendBuffer->Enqueue((char*)pHeader, sizeof(st_PACKET_HEADER));
	if (pHeader->byCode != 0x89)
		DebugBreak();
	int size = session->sendBuffer->Enqueue(pPacket->GetBufferPtr(), (int)pHeader->bySize);
}

void NetworkManager::SendBroadCast(Session* elseSession, st_PACKET_HEADER* pHeader, CPacket* pPacket)
{
	for (auto& pair : SessionManager::GetInstance()->GetSessionMap())
	{
		if (pair.second->deathFlag == true || pair.second == elseSession)
			continue;

		pair.second->sendBuffer->Enqueue((char*)pHeader, sizeof(st_PACKET_HEADER));
		if (pHeader->byCode != 0x89)
			DebugBreak();
		pair.second->sendBuffer->Enqueue(pPacket->GetBufferPtr(), (int)pHeader->bySize);
	}
}

void NetworkManager::Disconnect(Session* session)
{
	if (session->deathFlag == true)
		return;
	// ���� ������ ���� �¿� ���
	SessionManager::GetInstance()->ReserveDeleteSession(session);
	session->deathFlag = true;
}

void NetworkManager::netProc_Recv(Session* session)
{
	int RecvRetval;
	int RecvError;

	// ������ �ڵ�
	/*DebugLog debugLog;
	debugLog._prevFront = session->recvBuffer->_front;
	debugLog._prevRear = session->recvBuffer->_rear;*/



	// ��°�� ���ú�	
	RecvRetval = recv(session->socket, session->recvBuffer->GetRearBufferPtr(), session->recvBuffer->DirectEnqueueSize(), 0);
	// Select�� ������ ���� ���Ͽ��� ���ú갡 0 -> ���� ���� ����
	if (RecvRetval == 0)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"Recv() 0 Disconnect.");
		Disconnect(session);
		return;
	}
	if (RecvRetval == SOCKET_ERROR)
	{
		RecvError = WSAGetLastError();
		// �Ʒ� �� ���� ������ / ����Ʈ����� ���� ������ ���� ���
		if (RecvError == WSAECONNABORTED)
		{
			_LOG(dfLOG_LEVEL_ERROR, L"WSAECONNABORTED Disconnect %d.", RecvError);
			Disconnect(session);
			return;
		}
		if (RecvError == WSAECONNRESET)
		{
			_LOG(dfLOG_LEVEL_ERROR, L"WSAECONNRESET Disconnect %d.", RecvError);
			Disconnect(session);
			return;
		}
		// ������ ���� üũ�� ���� ���� �˴ٿ�
		_LOG(dfLOG_LEVEL_SYSTEM, L"Recv Error ShutDown %d.", RecvError);
		DebugBreak();
		g_bShutdown = false;
	}

	session->recvBuffer->MoveRear(RecvRetval);
	// ������ �ڵ�
	/*debugLog._currentFront = session->recvBuffer->_front;
	debugLog._currentRear = session->recvBuffer->_rear;
	debugLog._moveRearValue = RecvRetval;
	session->recvBuffer->_debugLogList.push_back(debugLog);*/

	while (true)
	{
		// �����ŭ ���� �� ������
		if (session->recvBuffer->GetUseSize() >= sizeof(st_PACKET_HEADER))
		{
			st_PACKET_HEADER header;

			int ret = session->recvBuffer->Peek((char*)&header, sizeof(st_PACKET_HEADER));

			if (session->recvBuffer->GetUseSize() < header.bySize + sizeof(st_PACKET_HEADER))
				break;
			session->recvBuffer->MoveFront(sizeof(st_PACKET_HEADER));

			CPacket* packet = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
			packet->Clear();

			char* packetData = packet->GetBufferPtr();
			// ������ �ڵ�
			/*DebugLog debugLog1;
			debugLog1._prevFront = session->recvBuffer->_front;
			debugLog1._prevRear = session->recvBuffer->_rear;*/
			// ������� �������� ���� �۴ٸ�
			if (session->recvBuffer->DirectDequeueSize() < header.bySize)
			{
				int freeSize = session->recvBuffer->DirectDequeueSize();
				int remainLength = header.bySize - freeSize;
				memcpy(packetData, session->recvBuffer->GetFrontBufferPtr(), freeSize);
				session->recvBuffer->MoveFront(freeSize);

				memcpy((packetData)+freeSize, session->recvBuffer->GetFrontBufferPtr(), remainLength);
				session->recvBuffer->MoveFront(remainLength);
			}
			// ����� ���� �� ������
			else
			{
				memcpy(packetData, session->recvBuffer->GetFrontBufferPtr(), header.bySize);
				session->recvBuffer->MoveFront(header.bySize);
			}

			packet->MoveWritePos(header.bySize);
			// ������ �ڵ�
			/*debugLog1._currentFront = session->recvBuffer->_front;
			debugLog1._currentRear = session->recvBuffer->_rear;
			debugLog1._moveFrontValue = header.bySize;
			session->recvBuffer->_debugLogList.push_back(debugLog1);*/
			// ��Ŷ ó��
			if (!NetworkManager::GetInstance()->ProcessPacket(session, header.byType, packet))
			{
				_LOG(dfLOG_LEVEL_ERROR, L"ProcessPacket Error, sessionId : %d", session->sessionID);
				Disconnect(session);
			}

			SerializingBufferManager::GetInstance()->_cPacketPool.Free(packet);
		}
		else
			break;
	}
}

void NetworkManager::netProc_Send(Session* session)
{
	int SendRetval;
	int SendError;
	// ������ �� ������ ũ�� Ȯ��
	int dataSize = session->sendBuffer->GetUseSize();

	if (session->deathFlag == true)
		return;

	// ��踦 �ȳѴ� ���
	if (session->sendBuffer->DirectDequeueSize() >= dataSize)
	{
	// ������ �ڵ�
	DebugLog debugLog;
	debugLog._prevFront = session->sendBuffer->_front;
	debugLog._prevRear = session->sendBuffer->_rear;
		// �����͸� �� ���� ����
		int sentBytes = send(session->socket, session->sendBuffer->GetFrontBufferPtr(), dataSize, 0);

		if (sentBytes == SOCKET_ERROR)
		{
			SendError = WSAGetLastError();

			// �Ʒ� �� ���� ������ / ����Ʈ����� ���� ������ ���� ���
			if (SendError == WSAECONNABORTED)
			{
				_LOG(dfLOG_LEVEL_ERROR, L"Send Error WSAECONNABORTED Disconnect %d., sessionId : %d", SendError, session->sessionID);
				Disconnect(session);
				return;
			}
			if (SendError == WSAECONNRESET)
			{
				_LOG(dfLOG_LEVEL_ERROR, L"Send Error WSAECONNRESET Disconnect %d., sessionId : %d", SendError, session->sessionID);
				Disconnect(session);
				return;
			}
			// �̿��� ��� ���� �˴ٿ�
			_LOG(dfLOG_LEVEL_SYSTEM, L"Send Error ShutDown %d.", SendError);
			DebugBreak();
			g_bShutdown = false;
		}
		session->sendBuffer->MoveFront(sentBytes);
		// ������ �ڵ�
		/*debugLog._currentFront = session->sendBuffer->_front;
		debugLog._currentRear = session->sendBuffer->_rear;
		debugLog._moveFrontValue = sentBytes;
		session->sendBuffer->_debugLogList.push_back(debugLog);*/
	}
	else
	{
		// �������� ��踦 �Ѿ�� ���
			// ������ �ڵ�
		/*DebugLog debugLog;
		debugLog._prevFront = session->sendBuffer->_front;
		debugLog._prevRear = session->sendBuffer->_rear;*/

		int sentBytes = send(session->socket, session->sendBuffer->GetFrontBufferPtr(), session->sendBuffer->DirectDequeueSize(), 0);
		// ���� ���� -> select �𵨿��� ���� �����

		if (sentBytes == SOCKET_ERROR)
		{
			SendError = WSAGetLastError();

			// �Ʒ� �� ���� ������ / ����Ʈ����� ���� ������ ���� ���
			if (SendError == WSAECONNABORTED)
			{
				_LOG(dfLOG_LEVEL_ERROR, L"Send Error WSAECONNABORTED Disconnect %d., sessionId : %d", SendError, session->sessionID);
				Disconnect(session);
				return;
			}
			if (SendError == WSAECONNRESET)
			{
				_LOG(dfLOG_LEVEL_ERROR, L"Send Error WSAECONNRESET Disconnect %d., sessionId : %d", SendError, session->sessionID);
				Disconnect(session);
				return;
			}
			// �̿��� ��� ���� �˴ٿ�
			_LOG(dfLOG_LEVEL_SYSTEM, L"Send Error ShutDown %d.", SendError);
			DebugBreak();
			g_bShutdown = false;
		}
		session->sendBuffer->MoveFront(sentBytes);
		//// ������ �ڵ�
		/*debugLog._currentFront = session->sendBuffer->_front;
		debugLog._currentRear = session->sendBuffer->_rear;
		debugLog._moveFrontValue = sentBytes;
		session->sendBuffer->_debugLogList.push_back(debugLog);*/
	}
}

bool NetworkManager::ProcessPacket(Session* session, char packetType, CPacket* packetData)
{
	if (packetProcessor != nullptr) {
		return packetProcessor->ProcessPacket(session, packetType, packetData);
	}
	// ��Ŷ ���μ����� �������� ���� ���
	_LOG(dfLOG_LEVEL_SYSTEM, L"ProcessPacket is not Set.");
	return false;
}