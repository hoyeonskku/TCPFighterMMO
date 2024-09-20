#pragma once
#include "NetworkManager.h"
#include "PacketDefine.h"
#include "netPacketProcs.h"
#include "Session.h"
#include "NetworkLogic.h"
#include "SerializingBuffer.h"

class ContentPacketProcessor : public IPacketProcessor
{
public:
	// 패킷 처리 함수
	// 주의! 점프 테이블 생성을 위한 순서 보장해주기
	virtual ~ContentPacketProcessor() {}
	bool ProcessPacket(Session* session, char packetType, CPacket* packetData) {
		switch (packetType)
		{
		case dfPACKET_CS_MOVE_START:
		{
			return netPacketProc_MoveStart(session->player, packetData);
		}
		case dfPACKET_CS_MOVE_STOP:
		{
			return netPacketProc_MoveStop(session->player, packetData);
		}
		case dfPACKET_CS_ATTACK1:
		{
			return netPacketProc_Attack1(session->player, packetData);
		}
		case dfPACKET_CS_ATTACK2:
		{
			return netPacketProc_Attack2(session->player, packetData);
		}
		case dfPACKET_CS_ATTACK3:
		{
			return netPacketProc_Attack3(session->player, packetData);
		}
		case dfPACKET_CS_SYNC:
		{
			break;
		}
		default:
		{
			DisconnectByPlayer(session->player);
			return false;
		}
		}
	}

};

