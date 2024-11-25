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
		SocketError = WSAGetLastError();
		_LOG(dfLOG_LEVEL_ERROR, L"socket() error, code %d", SocketError);
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

	return 0;
}

void NetworkManager::netCleanUp()
{
	WSACleanup();
}

void NetworkManager::netIOProcess()
{
	/*if (!TimeManager::GetInstance()->CheckNetworkFrameTime())
		return;*/
	// ��Ʈ��ũ ������ ����
	TimeManager::GetInstance()->CheckLogicFrameTime();
	networkFps++;

	FD_SET rSet;
	FD_SET wSet;

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	Session* sessionArray = SessionManager::GetInstance()->GetSessionArray();


	struct timeval timeout;
	timeout.tv_sec = 0;  // �� ���� ����
	timeout.tv_usec = 0; // ����ũ���� ���� ����

	SOCKADDR_IN clientaddr;
	char addr_str[INET_ADDRSTRLEN];

	int SelectRetval;
	int SelectError;

	int setIndex = 0;
	int isSetIndex = 0;

	while (setIndex != SESSIONMAXCOUNT)
	{
		FD_ZERO(&rSet);
		FD_ZERO(&wSet);

		FD_SET(listenSocket, &rSet);

		Session* setSession;
		// �¿� ���� �ϳ��� ���
		int count = 0;
		while (count++ < 63)
		{
			setSession = &sessionArray[setIndex];
			if (setSession->useFlag == true)
			{
				FD_SET(setSession->socket, &rSet);
				// ���� �����Ͱ� ���������� wSet�� ���
				if (setSession->sendBuffer->GetUseSize() != 0)
					FD_SET(setSession->socket, &wSet);
			}
			setIndex++;
			if (setIndex == SESSIONMAXCOUNT)
				break;
		}

		SelectRetval = select(0, &rSet, &wSet, nullptr, &timeout);
		if (SelectRetval == -1) 
		{
			SelectError = WSAGetLastError();
			_LOG(dfLOG_LEVEL_ERROR, L"select() error, code %d", SelectError);
			DebugBreak();
			g_bShutdown = false;
		}
		if (FD_ISSET(listenSocket, &rSet))
		{
			SelectRetval--;
			netProc_Accept();
		}
		Session* isSetSession;
		while (true)
		{			
			// ���� Ȯ���ߴٸ� ���� ������ �̵�,
			if (SelectRetval == 0)
			{
				isSetIndex = setIndex;
				break;
			}
			isSetSession = &sessionArray[isSetIndex];
			if (FD_ISSET(isSetSession->socket, &rSet))
			{
				if (isSetSession->useFlag == false)
				{
					DebugBreak();
				}
				SelectRetval--;
				netProc_Recv(isSetSession);
			}
			if (FD_ISSET(isSetSession->socket, &wSet))
			{
				if (isSetSession->useFlag == false)
				{
					DebugBreak();
				}
				SelectRetval--;
				netProc_Send(isSetSession);
			}
			isSetIndex++;

			if (isSetIndex > setIndex)
				DebugBreak();
		}

		// ���������� ����¿� �ִ� ��� accept ��û�� ����.

		if (setIndex == SESSIONMAXCOUNT)
			break;
	}

	

	SessionManager::GetInstance()->RemoveSessions();
}

void NetworkManager::netProc_Accept()
{
	char addr_str[INET_ADDRSTRLEN];
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	int acceptError;
	while (true)
	{
		client_sock = accept(listenSocket, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK) 
				// �� �̻� ó���� Ŭ���̾�Ʈ�� �����Ƿ� ���� ����
				break;

			acceptError = WSAGetLastError();
			_LOG(dfLOG_LEVEL_ERROR, L"accept() error, code %d", acceptError);
			g_bShutdown = false;
			return;
		}
		// ���� ����
		Session* session = SessionManager::GetInstance()->CreateSession();
		session->socket = client_sock;

		inet_ntop(AF_INET, &clientaddr.sin_addr, addr_str, sizeof(addr_str));
		session->port = ntohs(clientaddr.sin_port);
		strcpy_s(session->ip, sizeof(session->ip), addr_str);
	}
}

void NetworkManager::SendUnicast(unsigned long long sessionID, st_PACKET_HEADER* pHeader, CPacket* pPacket)
{

	unsigned short index = SessionManager::GetInstance()->GetIndexBySessionID(sessionID);
	Session* session = SessionManager::GetInstance()->GetSessionByIndex(index);
	if (session->useFlag == false)
		DebugBreak();
	session->sendBuffer->Enqueue((char*)pHeader, sizeof(st_PACKET_HEADER));
	if (pHeader->byCode != 0x89)
		DebugBreak();
	int size = session->sendBuffer->Enqueue(pPacket->GetBufferPtr(), (int)pHeader->bySize);
}

//void NetworkManager::SendBroadCast(unsigned long long elseSessionID, st_PACKET_HEADER* pHeader, CPacket* pPacket)
//{
//	short elseIndex = SessionManager::GetInstance()->GetIndexBySessionID(elseSessionID);
//	Session* elseSession = SessionManager::GetInstance()->GetSessionByIndex(elseIndex);
//	for (int i = 0; i < SessionManager::GetInstance()->GetSessionCount(); i++)
//	{
//		Session* session = SessionManager::GetInstance()->GetSessionByIndex(i);
//		if (session->deathFlag == true || session == elseSession)
//			continue;
//
//		session->sendBuffer->Enqueue((char*)pHeader, sizeof(st_PACKET_HEADER));
//		if (pHeader->byCode != 0x89)
//			DebugBreak();
//		session->sendBuffer->Enqueue(pPacket->GetBufferPtr(), (int)pHeader->bySize);
//	}
//}

void NetworkManager::Disconnect(unsigned long long sessionID)
{

	unsigned short index = SessionManager::GetInstance()->GetIndexBySessionID(sessionID);
	Session* session = &SessionManager::GetInstance()->GetSessionArray()[index];

	if (session->deathFlag == true)
		return;
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
		Disconnect(session->sessionID);
		return;
	}
	if (RecvRetval == SOCKET_ERROR)
	{
		RecvError = WSAGetLastError();
		// �Ʒ� �� ���� ������ / ����Ʈ����� ���� ������ ���� ���
		if (RecvError == WSAECONNABORTED)
		{
			//_LOG(dfLOG_LEVEL_ERROR, L"WSAECONNABORTED Disconnect %d.", RecvError);
			Disconnect(session->sessionID);
			return;
		}
		if (RecvError == WSAECONNRESET)
		{
			//_LOG(dfLOG_LEVEL_ERROR, L"WSAECONNRESET Disconnect %d.", RecvError);
			Disconnect(session->sessionID);
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
				Disconnect(session->sessionID);
			}

			SerializingBufferManager::GetInstance()->_cPacketPool.Free(packet);
		}
		else
			break;
	}
}

void NetworkManager::netProc_Send(Session* session)
{
	int SentBytes;
	int SendError;
	// ������ �� ������ ũ�� Ȯ��
	int dataSize = session->sendBuffer->GetUseSize();

	if (session->deathFlag == true)
		return;

	// ��踦 �ȳѴ� ���
	if (session->sendBuffer->DirectDequeueSize() >= dataSize)
	{
	// ������ �ڵ�
	/*DebugLog debugLog;
	debugLog._prevFront = session->sendBuffer->_front;
	debugLog._prevRear = session->sendBuffer->_rear;*/
		// �����͸� �� ���� ����
		SentBytes = send(session->socket, session->sendBuffer->GetFrontBufferPtr(), dataSize, 0);

		if (SentBytes == SOCKET_ERROR)
		{
			SendError = WSAGetLastError();

			// �Ʒ� �� ���� ������ / ����Ʈ����� ���� ������ ���� ���
			if (SendError == WSAECONNABORTED)
			{
				//_LOG(dfLOG_LEVEL_ERROR, L"Send Error WSAECONNABORTED Disconnect %d., sessionId : %d", SendError, session->sessionID);
				Disconnect(session->sessionID);
				return;
			}
			if (SendError == WSAECONNRESET)
			{
				//_LOG(dfLOG_LEVEL_ERROR, L"Send Error WSAECONNRESET Disconnect %d., sessionId : %d", SendError, session->sessionID);
				Disconnect(session->sessionID);
				return;
			}
			// �̿��� ��� ���� �˴ٿ�
			_LOG(dfLOG_LEVEL_SYSTEM, L"Send Error ShutDown %d.", SendError);
			DebugBreak();
			g_bShutdown = false;
		}
		session->sendBuffer->MoveFront(SentBytes);
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
				//_LOG(dfLOG_LEVEL_ERROR, L"Send Error WSAECONNABORTED Disconnect %d., sessionId : %d", SendError, session->sessionID);
				Disconnect(session->sessionID);
				return;
			}
			if (SendError == WSAECONNRESET)
			{
				//_LOG(dfLOG_LEVEL_ERROR, L"Send Error WSAECONNRESET Disconnect %d., sessionId : %d", SendError, session->sessionID);
				Disconnect(session->sessionID);
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


	return true;

}