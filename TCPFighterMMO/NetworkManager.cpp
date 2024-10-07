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

extern bool g_bShutdown;

NetworkManager* NetworkManager::GetInstance()
{
	static NetworkManager instance; // �̱��� �ν��Ͻ� ����
	return &instance;
}

int NetworkManager::netInit(IPacketProcessor* customProcessor)
{
	packetProcessor = customProcessor;
	int WSAStartUpRetval;
	int SocketRetval;
	int IoctlsocketRetval;
	int BindRetval;
	int ListenRetval;
	WSADATA wsaData;

	WSAStartUpRetval = ::WSAStartup(MAKEWORD(2, 2), OUT & wsaData);
	if (WSAStartUpRetval != 0)
	{
		printf("WSAStartup Error!");
		std::cout << WSAGetLastError();
		g_bShutdown = false;
		return 0;
	}
	else
		printf("WSAStartup #\n");

	listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket Error!\n");
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
		printf("bind Error!\n");
		std::cout << WSAGetLastError();
		g_bShutdown = false;
		return 0;
	}
	else
		printf("bind OK # Port:20000\n");

	u_long on = 1;
	IoctlsocketRetval = ::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET;

	if (IoctlsocketRetval == SOCKET_ERROR)
	{
		printf("ioctlsocket Error!\n");
		g_bShutdown = false;
		return 0;
	}

	ListenRetval = ::listen(listenSocket, SOMAXCONN_HINT(65535));
	//ListenRetval = ::listen(listenSocket, SOMAXCONN);
	if (ListenRetval == SOCKET_ERROR)
	{
		printf("listen Error!\n");
		g_bShutdown = false;
		return 0;
	}
	else
		printf("listen OK #\n");

	return 0;
}

void NetworkManager::netCleanUp()
{
	WSACleanup();
}

void NetworkManager::netIOProcess()
{
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

	auto setIter = sessionList.begin();
	auto isSetIter = sessionList.begin();
	while (sessionListSize >= 64)
	{
		FD_ZERO(&rSet);
		FD_ZERO(&wSet);
		for (int i = 0; i < 64; i++)
		{
			FD_SET((*setIter).second->socket, &rSet);

			if ((*setIter).second->sendBuffer->GetUseSize() != 0)
				FD_SET((*setIter).second->socket, &wSet);
			setIter++;
		}
		sessionListSize -= 64;
		SelectRetval = select(0, &rSet, &wSet, nullptr, &timeout);
		if (SelectRetval == -1)
		{
			DWORD error = WSAGetLastError();
			if (error == 10022)
				break;
			std::cout << "select() error, code: " << WSAGetLastError() << std::endl;
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
			if (SelectRetval == 0)
				break;
		}
	}

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
		DWORD error = WSAGetLastError();
		std::cout << "select() error, code: " << WSAGetLastError() << std::endl;
		DebugBreak();
		g_bShutdown = false;
	}
	// ������ ���� ��ȸ
	for (int j = 0; j < sessionListSize; j++)
	{
		if (isSetIter == sessionList.end())
			break;

		if ((*isSetIter).second->socket == listenSocket)
		{	
			SelectRetval--;
			isSetIter++;
			continue;
		}

		if (FD_ISSET((*isSetIter).second->socket, &rSet))
		{
			SelectRetval--;
			netProc_Recv((*isSetIter).second);
		}

		if (FD_ISSET((*isSetIter).second->socket, &wSet))
		{
			if ((*isSetIter).second->deathFlag != true)
			{
				SelectRetval--;
				netProc_Send((*isSetIter).second);
			}
		}
		isSetIter++;
		if (SelectRetval == 0)
			break;
	}

	// ���������� ����¿� �ִ� ��� accept ��û�� ����.
	if (FD_ISSET(listenSocket, &rSet))
		netProc_Accept();

	SessionManager::GetInstance()->RemoveSessions();
}

void NetworkManager::netProc_Accept()
{
	char addr_str[INET_ADDRSTRLEN];
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	//while (true)
	//{
	//	SOCKET clientSock = accept(listenSocket, (SOCKADDR*)&clientaddr, &addrlen);
	//	if (clientSock == INVALID_SOCKET)
	//	{
	//		if (WSAGetLastError() == WSAEWOULDBLOCK) 
	//			// �� �̻� ó���� Ŭ���̾�Ʈ�� �����Ƿ� ���� ����
	//			break;
	//		std::cout << "accept() error" << std::endl;
	//		std::cout << WSAGetLastError() << std::endl;
	//		DebugBreak();
	//		g_bShutdown = false;
	//		return;
	//	}
	//	// ���� ����
	//	Session* session = SessionManager::GetInstance()->CreateSession();
	//	session->socket = clientSock;

	//	inet_ntop(AF_INET, &clientaddr.sin_addr, addr_str, sizeof(addr_str));
	//	session->port = ntohs(clientaddr.sin_port);
	//	strcpy_s(session->ip, sizeof(session->ip), addr_str);
	//}
	SOCKET clientSock = accept(listenSocket, (SOCKADDR*)&clientaddr, &addrlen);
	if (clientSock == INVALID_SOCKET)
	{
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

void NetworkManager::SendUnicast(Session* session, st_PACKET_HEADER* pHeader, CPacket* pPacket)
{
	session->sendBuffer->Enqueue((char*)pHeader, sizeof(st_PACKET_HEADER));
	int size = session->sendBuffer->Enqueue(pPacket->GetBufferPtr(), (int)pHeader->bySize);

#ifdef _DEBUG
	if (size != pHeader->bySize)
		DebugBreak();
#endif // _DEBUG

}

void NetworkManager::SendBroadCast(Session* elseSession, st_PACKET_HEADER* pHeader, CPacket* pPacket)
{
	for (auto& pair : SessionManager::GetInstance()->GetSessionMap())
	{
		if (pair.second->deathFlag == true || pair.second == elseSession)
			continue;

		pair.second->sendBuffer->Enqueue((char*)pHeader, sizeof(st_PACKET_HEADER));
		pair.second->sendBuffer->Enqueue(pPacket->GetBufferPtr(), (int)pHeader->bySize);
	}
}

void NetworkManager::Disconnect(Session* session)
{
	if (session->deathFlag == true)
		return;

	SessionManager::GetInstance()->ReserveDeleteSession(session);
	session->deathFlag = true;
}

void NetworkManager::netProc_Recv(Session* session)
{
	int RecvRetval;
	int RecvError;

	// ��°�� ���ú�	
	RecvRetval = recv(session->socket, session->recvBuffer->GetRearBufferPtr(), session->recvBuffer->DirectEnqueueSize(), 0);
	// Select�� ������ ���� ���Ͽ��� ���ú갡 0 -> 
	if (RecvRetval == 0)
	{
		Disconnect(session);
		return;
	}
	if (RecvRetval == SOCKET_ERROR)
	{
		RecvError = WSAGetLastError();
		// �Ʒ� �� ���� ������ / ����Ʈ����� ���� ������ ���� ���
		if (RecvError == WSAECONNABORTED)
		{
			Disconnect(session);
			return;
		}
		if (RecvError == WSAECONNRESET)
		{
			Disconnect(session);
			return;
		}
		// ������ ���� üũ�� ���� ���� �˴ٿ�
		DebugBreak();
	}
	session->recvBuffer->MoveRear(RecvRetval);
	while (true)
	{
		// �����ŭ ���� �� ������
		if (session->recvBuffer->GetUseSize() >= sizeof(st_PACKET_HEADER))
		{
			st_PACKET_HEADER header;

			int ret = session->recvBuffer->Peek((char*)&header, sizeof(st_PACKET_HEADER));

			if (session->recvBuffer->GetUseSize() < header.bySize + sizeof(st_PACKET_HEADER))
				break;

			char* packetData = new char[header.bySize];
			session->recvBuffer->MoveFront(sizeof(st_PACKET_HEADER));

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
			// ��Ŷ ó��
			CPacket packet;
			packet.PutData(packetData, header.bySize);
			if (!NetworkManager::GetInstance()->ProcessPacket(session, header.byType, &packet))
			{
				Disconnect(session);
			}
			delete[] packetData;
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
		// �����͸� �� ���� ����
		int sentBytes = send(session->socket, session->sendBuffer->GetFrontBufferPtr(), dataSize, 0);

		if (sentBytes == SOCKET_ERROR)
		{
			SendError = WSAGetLastError();

			// �Ʒ� �� ���� ������ / ����Ʈ����� ���� ������ ���� ���
			if (SendError == WSAECONNABORTED)
			{
				Disconnect(session);
				return;
			}
			if (SendError == WSAECONNRESET)
			{
				Disconnect(session);
				return;
			}
			// �̿��� ��� ���� �˴ٿ�
			DebugBreak();
		}
		session->sendBuffer->MoveFront(sentBytes);
	}
	else
	{
		// �������� ��踦 �Ѿ�� ���
		int sentBytes = send(session->socket, session->sendBuffer->GetFrontBufferPtr(), session->sendBuffer->DirectDequeueSize(), 0);
		// ���� ���� -> select �𵨿��� ���� �����

		if (sentBytes == SOCKET_ERROR)
		{
			SendError = WSAGetLastError();

			// �Ʒ� �� ���� ������ / ����Ʈ����� ���� ������ ���� ���
			if (SendError == WSAECONNABORTED)
			{
				Disconnect(session);
				return;
			}
			if (SendError == WSAECONNRESET)
			{
				Disconnect(session);
				return;
			}
			// �̿��� ��� ���� �˴ٿ�
			DebugBreak();
		}
		session->sendBuffer->MoveFront(sentBytes);
	}
}

bool NetworkManager::ProcessPacket(Session* session, char packetType, CPacket* packetData)
{
	if (packetProcessor != nullptr) {
		return packetProcessor->ProcessPacket(session, packetType, packetData);
	}
	// ��Ŷ ���μ����� �������� ���� ���
	return false;
}