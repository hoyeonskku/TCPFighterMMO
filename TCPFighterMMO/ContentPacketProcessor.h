#pragma once
#include "NetworkManager.h"
#include "PacketDefine.h"
#include "netPacketProcs.h"
#include "Session.h"
#include "SerializingBuffer.h"
#include "NetworkManager.h"
#include "TimeManager.h"

class ContentPacketProcessor : public IPacketProcessor
{
public:
	// 패킷 처리 함수
	// 점프 테이블 생성을 위한 순서 보장해주기
	virtual ~ContentPacketProcessor() {}
	bool ProcessPacket(Session* session, unsigned char packetType, CPacket* packetData) 
	{
		// 직렬화버퍼 예외처리
		try
		{
			switch (packetType)
			{
				case dfPACKET_CS_MOVE_START:
				{
					return netPacketProc_MoveStart(session, packetData);
				}
				case dfPACKET_CS_MOVE_STOP:
				{
					return netPacketProc_MoveStop(session, packetData);
				}
				case dfPACKET_CS_ATTACK1:
				{
					return netPacketProc_Attack1(session, packetData);
				}
				case dfPACKET_CS_ATTACK2:
				{
					return netPacketProc_Attack2(session, packetData);
				}
				case dfPACKET_CS_ATTACK3:
				{
					return netPacketProc_Attack3(session, packetData);
				}
				case dfPACKET_CS_ECHO:
				{
					return netPacketProc_Echo(session, packetData);
				}
				default:
				{
					NetworkManager::GetInstance()->Disconnect(session);
					_LOG(dfLOG_LEVEL_ERROR, L"Packet Type Error, sessionID : %d", session->sessionID);
					return false;
				}
			}
		}
		catch (const std::runtime_error& e)
		{
			std::cerr << "Caught an exception: " << e.what() << std::endl;
			DebugBreak();
		}
	}

};

