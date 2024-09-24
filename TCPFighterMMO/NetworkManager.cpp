#include "pch.h"
#include "NetworkManager.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <winsock2.h>
#define _WINSOCKAPI_
#include <windows.h>
#include <ws2tcpip.h>
#include "PacketDEfine.h"
#include "Session.h"
#include "netPacketProcs.h"
#include "MakePacket.h"
#include "SessionManager.h"
#include "ObjectManager.h"
#include "SerializingBuffer.h"

extern bool g_bShutdown;

NetworkManager* NetworkManager::GetInstance()
{
	static NetworkManager instance; // 싱글톤 인스턴스 생성
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
		printf("bind OK # Port:6000\n");

	u_long on = 1;
	IoctlsocketRetval = ::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET;

	if (IoctlsocketRetval == SOCKET_ERROR)
	{
		printf("ioctlsocket Error!\n");
		g_bShutdown = false;
		return 0;
	}

	//ListenRetval = ::listen(listenSocket, SOMAXCONN_HINT(65535));
	ListenRetval = ::listen(listenSocket, SOMAXCONN);
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
	std::unordered_map<int, Session*> sessionList = SessionManager::GetInstance()->GetSessionMap();

	int sessionListSize = sessionList.size();

	struct timeval timeout;
	timeout.tv_sec = 0;  // 초 단위 설정
	timeout.tv_usec = 0; // 마이크로초 단위 설정

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	char addr_str[INET_ADDRSTRLEN];

	int SelectRetval;

	auto it = sessionList.begin();
	auto iter = sessionList.begin();
	while (sessionListSize > 0)
	{
		FD_ZERO(&rSet);
		FD_ZERO(&wSet);
		for (int i = 0; i < min(64, sessionListSize); i++)
		{
			FD_SET((*iter).second->socket, &rSet);
			FD_SET((*iter).second->socket, &wSet);
			iter++;
		}
		sessionListSize -= 64;
		SelectRetval = select(0, &rSet, &wSet, nullptr, &timeout);
		if (SelectRetval == -1)
		{
			std::cout << "select() error, code: " << WSAGetLastError() << std::endl;
			DebugBreak();
			g_bShutdown = false;
		}
		// 나머지 세션 순회
		for (int j = 0; j < 64; j++)
		{
			if (it == sessionList.end())
				break;
			if ((*it).second->socket == listenSocket)
			{
				it++;
				continue;
			}

			if ((*it).second->deathFlag)
			{
				it++;
				continue;
			}
			if (FD_ISSET((*it).second->socket, &rSet))
			{
				SelectRetval--;
				netProc_Recv((*it).second);
			}
			if (FD_ISSET((*it).second->socket, &wSet) && (*it).second->sendBuffer->GetUseSize() != 0)
			{
				SelectRetval--;
				if ((*it).second->socket != INVALID_SOCKET)
					netProc_Send((*it).second);
			}
			if (SelectRetval == 0)
				break;
			it++;
		}
	}

	FD_ZERO(&rSet);
	FD_SET(listenSocket, &rSet);
	SelectRetval = select(0, &rSet, nullptr, nullptr, &timeout);

	// 리슨소켓이 리드셋에 있는 경우 accept 요청이 있음.
	if (FD_ISSET(listenSocket, &rSet))
		netProc_Accept();

	SessionManager::GetInstance()->RemoveSessions();
}

void NetworkManager::netProc_Accept()
{
	char addr_str[INET_ADDRSTRLEN];
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	SOCKET clientSock = accept(listenSocket, (SOCKADDR*)&clientaddr, &addrlen);
	if (clientSock == INVALID_SOCKET)
	{
		std::cout << "accept() error" << std::endl;
		std::cout << WSAGetLastError() << std::endl;
		DebugBreak();
		g_bShutdown = false;
		return;
	}
	// 세션 생성
	Session* session = SessionManager::GetInstance()->CreateSession();
	session->socket = clientSock;

	inet_ntop(AF_INET, &clientaddr.sin_addr, addr_str, sizeof(addr_str));
	session->port = ntohs(clientaddr.sin_port);
	strcpy_s(session->ip, sizeof(session->ip), addr_str);
}

void NetworkManager::SendUnicast(Session* session, st_PACKET_HEADER* pHeader, CPacket* pPacket)
{
	session->sendBuffer->Enqueue((char*)pHeader, sizeof(st_PACKET_HEADER));
	session->sendBuffer->Enqueue(pPacket->GetBufferPtr(), (int)pHeader->bySize);
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
	closesocket(session->socket);
	session->deathFlag = true;
	session->socket = INVALID_SOCKET;
}

void NetworkManager::netProc_Recv(Session* session)
{
	int RecvRetval;
	int RecvError;

	// 통째로 리시브	
	RecvRetval = recv(session->socket, session->recvBuffer->GetRearBufferPtr(), session->recvBuffer->DirectEnqueueSize(), 0);
	// Select에 반응이 보인 소켓에서 리시브가 0 -> 
	if (RecvRetval == 0)
	{
		Disconnect(session);
		return;
	}
	if (RecvRetval == SOCKET_ERROR)
	{
		RecvError = WSAGetLastError();
		// 아래 두 경우는 강제로 / 소프트웨어로 인해 소켓이 닫힌 경우
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
		// 나머지 에러 체크를 위해 서버 셧다운
		DebugBreak();
	}
	session->recvBuffer->MoveRear(RecvRetval);
	while (true)
	{
		// 헤더만큼 읽을 수 있으면
		if (session->recvBuffer->GetUseSize() >= sizeof(st_PACKET_HEADER))
		{
			st_PACKET_HEADER header;

			int ret = session->recvBuffer->Peek((char*)&header, sizeof(st_PACKET_HEADER));

			if (session->recvBuffer->GetUseSize() < header.bySize + sizeof(st_PACKET_HEADER))
				break;

			char* packetData = new char[header.bySize];
			session->recvBuffer->MoveFront(sizeof(st_PACKET_HEADER));

			// 사이즈보다 경계까지의 값이 작다면
			if (session->recvBuffer->DirectDequeueSize() < header.bySize)
			{
				int freeSize = session->recvBuffer->DirectDequeueSize();
				int remainLength = header.bySize - freeSize;
				memcpy(packetData, session->recvBuffer->GetFrontBufferPtr(), freeSize);
				session->recvBuffer->MoveFront(freeSize);

				memcpy((packetData)+freeSize, session->recvBuffer->GetFrontBufferPtr(), remainLength);
				session->recvBuffer->MoveFront(remainLength);
			}
			// 충분히 읽을 수 있으면
			else
			{
				memcpy(packetData, session->recvBuffer->GetFrontBufferPtr(), header.bySize);
				session->recvBuffer->MoveFront(header.bySize);
			}
			// 패킷 처리
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
	// 보내야 할 데이터 크기 확인
	int dataSize = session->sendBuffer->GetUseSize();

	// 경계를 안넘는 경우
	if (session->sendBuffer->DirectDequeueSize() >= dataSize)
	{
		// 데이터를 한 번에 전송
		int sentBytes = send(session->socket, session->sendBuffer->GetFrontBufferPtr(), dataSize, 0);

		if (sentBytes == SOCKET_ERROR)
		{
			SendError = WSAGetLastError();

			// 아래 두 경우는 강제로 / 소프트웨어로 인해 소켓이 닫힌 경우
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
			// 이외의 경우 서버 셧다운
			DebugBreak();
		}
		session->sendBuffer->MoveFront(sentBytes);
	}
	else
	{
		// 링버퍼의 경계를 넘어가는 경우
		int sentBytes = send(session->socket, session->sendBuffer->GetFrontBufferPtr(), session->sendBuffer->DirectDequeueSize(), 0);
		// 전송 실패 -> select 모델에선 소켓 종료뿐

		if (sentBytes == SOCKET_ERROR)
		{
			SendError = WSAGetLastError();

			// 아래 두 경우는 강제로 / 소프트웨어로 인해 소켓이 닫힌 경우
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
			// 이외의 경우 서버 셧다운
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
	// 패킷 프로세서가 설정되지 않은 경우
	return false;
}