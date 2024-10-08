#include "pch.h"
#include "netPacketProcs.h"
#include <iostream>
#include "Session.h"
#include "Player.h"
#include "PacketDefine.h"
#include "MakePacket.h"
#include "NetworkManager.h"
#include "SectorManager.h"
#include "ObjectManager.h"
#include "SerializingBuffer.h"
#include "SerializingBufferManager.h"
#include "ContentPacketProcessor.h"
#include "NetworkLogic.h"
#include "TimeManager.h"

using namespace std;

#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3
#define dfERROR_RANGE			50
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20

bool netPacketProc_MoveStart(Session* session, CPacket* pPacket)
{
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	player->lastProcTime = TimeManager::GetInstance()->GetCurrentTick();
	*pPacket >> Direction >> x >> y;

	PacketInfo packetInfo;
	packetInfo.packetId = dfPACKET_CS_MOVE_START;
	packetInfo.x = x;
	packetInfo.y = y;
	packetInfo.dir = Direction;
	packetInfo.serverX = player->x;
	packetInfo.serverY = player->y;
	packetInfo.serverDir = player->dir;
	packetInfo.currentFrameCount = TimeManager::GetInstance()->currentFrameCount;
	packetInfo.recivedTime = TimeManager::GetInstance()->GetCurrentTick();
	session->packetQueue.Enqueue(packetInfo);
	// 클라이언트 좌표에 대한 최소한의 에러체크
	if ((abs(x - player->x) > dfERROR_RANGE) ||
		(abs(y - player->y) > dfERROR_RANGE))
	{
		SynchronizePos(player);
	}
	else
	{
		player->x = x;
		player->y = y;
	}
	player->moveFlag = true;
	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	st_PACKET_HEADER pMoveHeader;
	CPacket* pMovePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
	pMovePacket->Clear();
	mpMoveStart(&pMoveHeader, pMovePacket, Direction, player->x, player->y, player->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &pMoveHeader, pMovePacket);
	SerializingBufferManager::GetInstance()->_cPacketPool.Free(pMovePacket);
	
	return true;
}

bool netPacketProc_MoveStop(Session* session, CPacket* pPacket)
{
	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	/*if (player->moveFlag == false)
		DebugBreak();*/
	player->lastProcTime = TimeManager::GetInstance()->GetCurrentTick();
	// 받은 패킷 해석
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	*pPacket >> Direction >> x >> y;

	PacketInfo packetInfo;
	packetInfo.packetId = dfPACKET_CS_MOVE_STOP;
	packetInfo.x = x;
	packetInfo.y = y;
	packetInfo.dir = Direction;
	packetInfo.serverX = player->x;
	packetInfo.serverY = player->y;
	packetInfo.currentFrameCount = TimeManager::GetInstance()->currentFrameCount;
	packetInfo.serverDir = player->dir;
	packetInfo.recivedTime = TimeManager::GetInstance()->GetCurrentTick();

	session->packetQueue.Enqueue(packetInfo);
	// cout << "# PACKET_RECV # SessionID:" << player->session->id << endl;
	// cout << "dfPACKET_CS_MOVE_STOP # SessionID :" << player->session->id << " X : " << player->x << " Y : " << player->y << endl;
	// 클라이언트 좌표에 대한 최소한의 에러체크
	if ((abs(x - player->x) > dfERROR_RANGE) ||
		(abs(y - player->y) > dfERROR_RANGE))
	{
		SynchronizePos(player);
	}
	else
	{
		player->x = x;
		player->y = y;
	}
	player->moveFlag = false;
	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket* scMoveStopPacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
	scMoveStopPacket->Clear();
	st_PACKET_HEADER scMoveStopHeader;
	mpMoveStop(&scMoveStopHeader,scMoveStopPacket, Direction, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &scMoveStopHeader, scMoveStopPacket);
	SerializingBufferManager::GetInstance()->_cPacketPool.Free(scMoveStopPacket);

	return true;
}

bool netPacketProc_Attack1(Session* session, CPacket* pPacket)
{
	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	player->lastProcTime = TimeManager::GetInstance()->GetCurrentTick();
	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;

	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket* sendAttackPacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
	sendAttackPacket->Clear();
	st_PACKET_HEADER sendAttackHeader;
	mpAttack1(&sendAttackHeader, sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &sendAttackHeader, sendAttackPacket);
	SerializingBufferManager::GetInstance()->_cPacketPool.Free(sendAttackPacket);
	// 피격 대상 탐색
	Player* targetPlayer = nullptr;
	int leftUpY;
	int leftUpX;
	int rightDownY;
	int rightDownX;

	// 공격 방향에 따른 범위 설정
	if (player->dir == dfPACKET_MOVE_DIR_LL)
	{
		// 왼쪽 방향 공격
		leftUpY = player->y - dfATTACK1_RANGE_Y;
		leftUpX = player->x - dfATTACK1_RANGE_X;
		rightDownY = player->y + dfATTACK1_RANGE_Y;
		rightDownX = player->x;
	}
	else if (player->dir == dfPACKET_MOVE_DIR_RR)
	{
		// 오른쪽 방향 공격
		leftUpY = player->y - dfATTACK1_RANGE_Y;
		leftUpX = player->x;
		rightDownY = player->y + dfATTACK1_RANGE_Y;
		rightDownX = player->x + dfATTACK1_RANGE_X;
	}
	else
	{
		// 다른 방향은 처리하지 않음
		return false;
	}

	// 공격 범위에 속하는 섹터 좌표 찾기
	SectorPos pos1 = SectorManager::GetInstance()->FindSectorPos(leftUpY, leftUpX);
	SectorPos pos2 = SectorManager::GetInstance()->FindSectorPos(rightDownY, rightDownX);
	int startSectorX = pos1.x;
	int startSectorY = pos1.y;
	int endSectorX = pos2.x;
	int endSectorY = pos2.y;
	// 걸치는 섹터 범위를 순회
	for (int sectorX = startSectorX; sectorX <= endSectorX; ++sectorX)
	{
		for (int sectorY = startSectorY; sectorY <= endSectorY; ++sectorY)
		{
			// 각 섹터의 객체들을 순회하며 대상 찾기
			const auto& playerUSet = SectorManager::GetInstance()->GetSectorPlayerSet(sectorY, sectorX);
			for (Player* searchedPlayer : playerUSet)
			{
				if (searchedPlayer == player)
					continue;
				// 대상이 공격 범위 안에 있는지 확인
				if (searchedPlayer->x >= leftUpX && searchedPlayer->x <= rightDownX &&
					searchedPlayer->y >= leftUpY && searchedPlayer->y <= rightDownY)
				{
					targetPlayer = searchedPlayer;
					targetPlayer->hp -= dfATTACK1_DAMAGE;
					// 데미지 패킷 및 헤더 생성, 전송
					CPacket* damagePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
					damagePacket->Clear();
					st_PACKET_HEADER damageHeader;
					mpDamage(&damageHeader, damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
					SectorManager::GetInstance()->SendAround(session, &damageHeader, damagePacket, true);
					SerializingBufferManager::GetInstance()->_cPacketPool.Free(damagePacket);
					// 죽은 경우
					if (targetPlayer->hp <= 0)
					{
						// 삭제 패킷 브로드캐스트
						CPacket* deletePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
						deletePacket->Clear();
						st_PACKET_HEADER deleteHeader;
						mpDelete(&deleteHeader, deletePacket, targetPlayer->session->sessionID);
						SectorManager::GetInstance()->SendAround(session, &deleteHeader, deletePacket, true);
						SerializingBufferManager::GetInstance()->_cPacketPool.Free(deletePacket);
						NetworkManager::GetInstance()->Disconnect(targetPlayer->session);
					}
					return true;
				}
			}
		}
	}
	return true;
}


bool netPacketProc_Attack2(Session* session, CPacket* pPacket)
{
	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	player->lastProcTime = TimeManager::GetInstance()->GetCurrentTick();

	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;

	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket* sendAttackPacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
	sendAttackPacket->Clear();
	st_PACKET_HEADER sendAttackHeader;
	mpAttack2(&sendAttackHeader, sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &sendAttackHeader, sendAttackPacket);
	SerializingBufferManager::GetInstance()->_cPacketPool.Free(sendAttackPacket);
	// 피격 대상 탐색
	Player* targetPlayer = nullptr;
	int leftUpY;
	int leftUpX;
	int rightDownY;
	int rightDownX;

	// 공격 방향에 따른 범위 설정
	if (player->dir == dfPACKET_MOVE_DIR_LL)
	{
		// 왼쪽 방향 공격
		leftUpY = player->y - dfATTACK2_RANGE_Y;
		leftUpX = player->x - dfATTACK2_RANGE_X;
		rightDownY = player->y + dfATTACK2_RANGE_Y;
		rightDownX = player->x;
	}
	else if (player->dir == dfPACKET_MOVE_DIR_RR)
	{
		// 오른쪽 방향 공격
		leftUpY = player->y - dfATTACK2_RANGE_Y;
		leftUpX = player->x;
		rightDownY = player->y + dfATTACK2_RANGE_Y;
		rightDownX = player->x + dfATTACK2_RANGE_X;
	}
	else
	{
		// 다른 방향은 처리하지 않음
		return false;
	}

	// 공격 범위에 속하는 섹터 좌표 찾기
	SectorPos pos1 = SectorManager::GetInstance()->FindSectorPos(leftUpY, leftUpX);
	SectorPos pos2 = SectorManager::GetInstance()->FindSectorPos(rightDownY, rightDownX);
	int startSectorX = pos1.x;
	int startSectorY = pos1.y;
	int endSectorX = pos2.x;
	int endSectorY = pos2.y;

	// 걸치는 섹터 범위를 순회
	for (int sectorX = startSectorX; sectorX <= endSectorX; ++sectorX)
	{
		for (int sectorY = startSectorY; sectorY <= endSectorY; ++sectorY)
		{
			// 각 섹터의 객체들을 순회하며 대상 찾기
			const auto& playerUSet = SectorManager::GetInstance()->GetSectorPlayerSet(sectorY, sectorX);
			for (Player* searchedPlayer : playerUSet)
			{
				if (searchedPlayer == player)
					continue;
				// 대상이 공격 범위 안에 있는지 확인
				if (searchedPlayer->x >= leftUpX && searchedPlayer->x <= rightDownX &&
					searchedPlayer->y >= leftUpY && searchedPlayer->y <= rightDownY)
				{
					targetPlayer = searchedPlayer;
					targetPlayer->hp -= dfATTACK2_DAMAGE;
					// 데미지 패킷 및 헤더 생성, 전송
					CPacket* damagePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
					damagePacket->Clear();
					st_PACKET_HEADER damageHeader;
					mpDamage(&damageHeader, damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
					SectorManager::GetInstance()->SendAround(session, &damageHeader, damagePacket, true);
					SerializingBufferManager::GetInstance()->_cPacketPool.Free(damagePacket);
					// 죽은 경우
					if (targetPlayer->hp <= 0)
					{
						// 삭제 패킷 브로드캐스트
						CPacket* deletePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
						deletePacket->Clear();
						st_PACKET_HEADER deleteHeader;
						mpDelete(&deleteHeader, deletePacket, targetPlayer->session->sessionID);
						SectorManager::GetInstance()->SendAround(session, &deleteHeader, deletePacket, true);
						SerializingBufferManager::GetInstance()->_cPacketPool.Free(deletePacket);
						NetworkManager::GetInstance()->Disconnect(targetPlayer->session);
					}
					return true;
				}
			}
		}
	}
	return true;
}



bool netPacketProc_Attack3(Session* session, CPacket* pPacket)
{
	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	player->lastProcTime = TimeManager::GetInstance()->GetCurrentTick();

	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;

	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket* sendAttackPacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
	sendAttackPacket->Clear();
	st_PACKET_HEADER sendAttackHeader;
	mpAttack3(&sendAttackHeader, sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &sendAttackHeader, sendAttackPacket);
	SerializingBufferManager::GetInstance()->_cPacketPool.Free(sendAttackPacket);
	// 피격 대상 탐색
	Player* targetPlayer = nullptr;
	int leftUpY;
	int leftUpX;
	int rightDownY;
	int rightDownX;

	// 공격 방향에 따른 범위 설정
	if (player->dir == dfPACKET_MOVE_DIR_LL)
	{
		// 왼쪽 방향 공격
		leftUpY = player->y - dfATTACK3_RANGE_Y;
		leftUpX = player->x - dfATTACK3_RANGE_X;
		rightDownY = player->y + dfATTACK3_RANGE_Y;
		rightDownX = player->x;
	}
	else if (player->dir == dfPACKET_MOVE_DIR_RR)
	{
		// 오른쪽 방향 공격
		leftUpY = player->y - dfATTACK3_RANGE_Y;
		leftUpX = player->x;
		rightDownY = player->y + dfATTACK3_RANGE_Y;
		rightDownX = player->x + dfATTACK3_RANGE_X;
	}
	else
	{
		// 다른 방향은 처리하지 않음
		return false;
	}

	// 공격 범위에 속하는 섹터 좌표 찾기
	SectorPos pos1 = SectorManager::GetInstance()->FindSectorPos(leftUpY, leftUpX);
	SectorPos pos2 = SectorManager::GetInstance()->FindSectorPos(rightDownY, rightDownX);
	int startSectorX = pos1.x;
	int startSectorY = pos1.y;
	int endSectorX = pos2.x;
	int endSectorY = pos2.y;

	// 걸치는 섹터 범위를 순회
	for (int sectorX = startSectorX; sectorX <= endSectorX; ++sectorX)
	{
		for (int sectorY = startSectorY; sectorY <= endSectorY; ++sectorY)
		{
			// 각 섹터의 객체들을 순회하며 대상 찾기
			const auto& playerUSet = SectorManager::GetInstance()->GetSectorPlayerSet(sectorY, sectorX);
			for (Player* searchedPlayer : playerUSet)
			{
				if (searchedPlayer == player)
					continue;
				// 대상이 공격 범위 안에 있는지 확인
				if (searchedPlayer->x >= leftUpX && searchedPlayer->x <= rightDownX &&
					searchedPlayer->y >= leftUpY && searchedPlayer->y <= rightDownY)
				{
					targetPlayer = searchedPlayer;
					targetPlayer->hp -= dfATTACK3_DAMAGE;
					// 데미지 패킷 및 헤더 생성, 전송
					CPacket* damagePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
					damagePacket->Clear();
					st_PACKET_HEADER damageHeader;
					mpDamage(&damageHeader, damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
					SectorManager::GetInstance()->SendAround(session, &damageHeader, damagePacket, true);
					SerializingBufferManager::GetInstance()->_cPacketPool.Free(damagePacket);
					// 죽은 경우
					if (targetPlayer->hp <= 0)
					{
						// 삭제 패킷 브로드캐스트
						CPacket* deletePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
						deletePacket->Clear();
						st_PACKET_HEADER deleteHeader;
						mpDelete(&deleteHeader, deletePacket, targetPlayer->session->sessionID);
						SectorManager::GetInstance()->SendAround(session, &deleteHeader, deletePacket, true);
						SerializingBufferManager::GetInstance()->_cPacketPool.Free(deletePacket);
						NetworkManager::GetInstance()->Disconnect(targetPlayer->session);
					}
					return true;
				}
			}
		}
	}
	return true;
}

bool netPacketProc_Echo(Session* session, CPacket* packet)
{
	CPacket* sendEchoPacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
	sendEchoPacket->Clear();
	st_PACKET_HEADER sendEchoHeader;
	unsigned int time;

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	player->lastProcTime = TimeManager::GetInstance()->GetCurrentTick();

	*packet >> time;

	// 헤더 및 페이로드 생성 및 전송
	mpEcho(&sendEchoHeader, sendEchoPacket, time);
	NetworkManager::GetInstance()->SendUnicast(session, &sendEchoHeader, sendEchoPacket);

	SerializingBufferManager::GetInstance()->_cPacketPool.Free(sendEchoPacket);
	return true;
}

