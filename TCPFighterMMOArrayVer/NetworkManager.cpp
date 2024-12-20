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
	static NetworkManager instance; // 싱글톤 인스턴스 생성
	return &instance;
}

int NetworkManager::netInit(IPacketProcessor* customProcessor)
{
	// 패킷 프로세서 등록
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
	// 네트워크 프레임 측정
	TimeManager::GetInstance()->CheckLogicFrameTime();
	networkFps++;

	FD_SET rSet;
	FD_SET wSet;

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	Session* sessionArray = SessionManager::GetInstance()->GetSessionArray();


	struct timeval timeout;
	timeout.tv_sec = 0;  // 초 단위 설정
	timeout.tv_usec = 0; // 마이크로초 단위 설정

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
		// 셋에 세션 하나씩 등록
		int count = 0;
		while (count++ < 63)
		{
			setSession = &sessionArray[setIndex];
			if (setSession->useFlag == true)
			{
				FD_SET(setSession->socket, &rSet);
				// 보낼 데이터가 있을때에만 wSet에 등록
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
			// 전부 확인했다면 다음 루프로 이동,
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

		// 리슨소켓이 리드셋에 있는 경우 accept 요청이 있음.

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
				// 더 이상 처리할 클라이언트가 없으므로 루프 종료
				break;

			acceptError = WSAGetLastError();
			_LOG(dfLOG_LEVEL_ERROR, L"accept() error, code %d", acceptError);
			g_bShutdown = false;
			return;
		}
		// 세션 생성
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

	// 디버깅용 코드
	/*DebugLog debugLog;
	debugLog._prevFront = session->recvBuffer->_front;
	debugLog._prevRear = session->recvBuffer->_rear;*/



	// 통째로 리시브	
	RecvRetval = recv(session->socket, session->recvBuffer->GetRearBufferPtr(), session->recvBuffer->DirectEnqueueSize(), 0);
	// Select에 반응이 보인 소켓에서 리시브가 0 -> 세션 정상 종료
	if (RecvRetval == 0)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"Recv() 0 Disconnect.");
		Disconnect(session->sessionID);
		return;
	}
	if (RecvRetval == SOCKET_ERROR)
	{
		RecvError = WSAGetLastError();
		// 아래 두 경우는 강제로 / 소프트웨어로 인해 소켓이 닫힌 경우
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
		// 나머지 에러 체크를 위해 서버 셧다운
		_LOG(dfLOG_LEVEL_SYSTEM, L"Recv Error ShutDown %d.", RecvError);
		DebugBreak();
		g_bShutdown = false;
	}

	session->recvBuffer->MoveRear(RecvRetval);
	// 디버깅용 코드
	/*debugLog._currentFront = session->recvBuffer->_front;
	debugLog._currentRear = session->recvBuffer->_rear;
	debugLog._moveRearValue = RecvRetval;
	session->recvBuffer->_debugLogList.push_back(debugLog);*/

	while (true)
	{
		// 헤더만큼 읽을 수 있으면
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
			// 디버깅용 코드
			/*DebugLog debugLog1;
			debugLog1._prevFront = session->recvBuffer->_front;
			debugLog1._prevRear = session->recvBuffer->_rear;*/
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

			packet->MoveWritePos(header.bySize);
			// 디버깅용 코드
			/*debugLog1._currentFront = session->recvBuffer->_front;
			debugLog1._currentRear = session->recvBuffer->_rear;
			debugLog1._moveFrontValue = header.bySize;
			session->recvBuffer->_debugLogList.push_back(debugLog1);*/
			// 패킷 처리
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
	// 보내야 할 데이터 크기 확인
	int dataSize = session->sendBuffer->GetUseSize();

	if (session->deathFlag == true)
		return;

	// 경계를 안넘는 경우
	if (session->sendBuffer->DirectDequeueSize() >= dataSize)
	{
	// 디버깅용 코드
	/*DebugLog debugLog;
	debugLog._prevFront = session->sendBuffer->_front;
	debugLog._prevRear = session->sendBuffer->_rear;*/
		// 데이터를 한 번에 전송
		SentBytes = send(session->socket, session->sendBuffer->GetFrontBufferPtr(), dataSize, 0);

		if (SentBytes == SOCKET_ERROR)
		{
			SendError = WSAGetLastError();

			// 아래 두 경우는 강제로 / 소프트웨어로 인해 소켓이 닫힌 경우
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
			// 이외의 경우 서버 셧다운
			_LOG(dfLOG_LEVEL_SYSTEM, L"Send Error ShutDown %d.", SendError);
			DebugBreak();
			g_bShutdown = false;
		}
		session->sendBuffer->MoveFront(SentBytes);
		// 디버깅용 코드
		/*debugLog._currentFront = session->sendBuffer->_front;
		debugLog._currentRear = session->sendBuffer->_rear;
		debugLog._moveFrontValue = sentBytes;
		session->sendBuffer->_debugLogList.push_back(debugLog);*/
	}
	else
	{
		// 링버퍼의 경계를 넘어가는 경우
			// 디버깅용 코드
		/*DebugLog debugLog;
		debugLog._prevFront = session->sendBuffer->_front;
		debugLog._prevRear = session->sendBuffer->_rear;*/

		int sentBytes = send(session->socket, session->sendBuffer->GetFrontBufferPtr(), session->sendBuffer->DirectDequeueSize(), 0);
		// 전송 실패 -> select 모델에선 소켓 종료뿐

		if (sentBytes == SOCKET_ERROR)
		{
			SendError = WSAGetLastError();

			// 아래 두 경우는 강제로 / 소프트웨어로 인해 소켓이 닫힌 경우
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
			// 이외의 경우 서버 셧다운
			_LOG(dfLOG_LEVEL_SYSTEM, L"Send Error ShutDown %d.", SendError);
			DebugBreak();
			g_bShutdown = false;
		}
		session->sendBuffer->MoveFront(sentBytes);
		//// 디버깅용 코드
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
	// 패킷 프로세서가 설정되지 않은 경우
	_LOG(dfLOG_LEVEL_SYSTEM, L"ProcessPacket is not Set.");
	return false;


	return true;

}