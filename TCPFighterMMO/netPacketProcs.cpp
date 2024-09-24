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
#include "ContentPacketProcessor.h"
#include "NetworkLogic.h"

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
	// cout << "# PACKET_RECV # SessionID:" << player->session->id << endl;
	// cout << "dfPACKET_CS_MOVE_START # SessionID :" << player->session->id << " X : " << player->x << " Y : " << player->y << endl;

	// 받은 패킷 해석
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);

	*pPacket >> Direction >> x >> y;
	// 클라이언트 좌표에 대한 최소한의 에러체크
	if ((abs(x - player->x) > dfERROR_RANGE) ||
		(abs(y - player->y) > dfERROR_RANGE))
	{
		SynchronizePos(player);
	}
	player->moveFlag = true;
	player->dir = Direction;
	player->x = x;
	player->y = y;
	// 헤더 및 페이로드 생성 및 전송
	st_PACKET_HEADER pMoveHeader;
	CPacket pMovePacket;

	mpMoveStart(&pMoveHeader, &pMovePacket, Direction, player->x, player->y, player->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &pMoveHeader, &pMovePacket);
	return true;
}

bool netPacketProc_MoveStop(Session* session, CPacket* pPacket)
{

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	// 받은 패킷 해석
	unsigned char Direction;
	unsigned short x;
	unsigned short y;


	*pPacket >> Direction >> x >> y;

	// cout << "# PACKET_RECV # SessionID:" << player->session->id << endl;
	// cout << "dfPACKET_CS_MOVE_STOP # SessionID :" << player->session->id << " X : " << player->x << " Y : " << player->y << endl;
	// 클라이언트 좌표에 대한 최소한의 에러체크
	if ((abs(x - player->x) > dfERROR_RANGE) ||
		(abs(y - player->y) > dfERROR_RANGE))
	{
		SynchronizePos(player);
	}
	player->moveFlag = false;
	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket scMoveStopPacket;
	st_PACKET_HEADER scMoveStopHeader;
	mpMoveStop(&scMoveStopHeader, &scMoveStopPacket, Direction, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &scMoveStopHeader, &scMoveStopPacket);

	return true;
}

bool netPacketProc_Attack1(Session* session, CPacket* pPacket)
{
	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);

	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;

	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack1(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &sendAttackHeader, &sendAttackPacket);
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
	int startSectorX = leftUpX / dfRANGE_SECTOR_RIGHT;
	int startSectorY = leftUpY / dfRANGE_SECTOR_BOTTOM;
	int endSectorX = rightDownX / dfRANGE_SECTOR_RIGHT;
	int endSectorY = rightDownY / dfRANGE_SECTOR_BOTTOM;

	// 걸치는 섹터 범위를 순회
	for (int sectorX = startSectorX; sectorX <= endSectorX; ++sectorX)
	{
		for (int sectorY = startSectorY; sectorY <= endSectorY; ++sectorY)
		{
			// 각 섹터의 객체들을 순회하며 대상 찾기
			const auto& playerUMap = SectorManager::GetInstance()->GetSectorPlayerMap(sectorY, sectorX);
			for (auto& pair : playerUMap)
			{
				if (pair.second == player)
					continue;
				// 대상이 공격 범위 안에 있는지 확인
				if (pair.second->x >= leftUpX && pair.second->x <= rightDownX &&
					pair.second->y >= leftUpY && pair.second->y <= rightDownY)
				{
					targetPlayer = pair.second;
					targetPlayer->hp -= dfATTACK1_DAMAGE;
					// 데미지 패킷 및 헤더 생성, 전송
					CPacket damagePacket;
					st_PACKET_HEADER damageHeader;
					mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
					SectorManager::GetInstance()->SendAround(session, &damageHeader, &damagePacket, true);
					// 죽은 경우
					if (targetPlayer->hp <= 0)
					{
						// 삭제 패킷 브로드캐스트
						CPacket deletePacket;
						st_PACKET_HEADER deleteHeader;
						mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->sessionID);
						SectorManager::GetInstance()->SendAround(session, &deleteHeader, &deletePacket, true);
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

	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;

	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack2(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &sendAttackHeader, &sendAttackPacket);
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
	int startSectorX = leftUpX / dfRANGE_SECTOR_RIGHT;
	int startSectorY = leftUpY / dfRANGE_SECTOR_BOTTOM;
	int endSectorX = rightDownX / dfRANGE_SECTOR_RIGHT;
	int endSectorY = rightDownY / dfRANGE_SECTOR_BOTTOM;

	// 걸치는 섹터 범위를 순회
	for (int sectorX = startSectorX; sectorX <= endSectorX; ++sectorX)
	{
		for (int sectorY = startSectorY; sectorY <= endSectorY; ++sectorY)
		{
			// 각 섹터의 객체들을 순회하며 대상 찾기
			const auto& playerUMap = SectorManager::GetInstance()->GetSectorPlayerMap(sectorY, sectorX);
			for (auto& pair : playerUMap)
			{
				if (pair.second == player)
					continue;
				// 대상이 공격 범위 안에 있는지 확인
				if (pair.second->x >= leftUpX && pair.second->x <= rightDownX &&
					pair.second->y >= leftUpY && pair.second->y <= rightDownY)
				{
					targetPlayer = pair.second;
					targetPlayer->hp -= dfATTACK2_DAMAGE;
					// 데미지 패킷 및 헤더 생성, 전송
					CPacket damagePacket;
					st_PACKET_HEADER damageHeader;
					mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
					SectorManager::GetInstance()->SendAround(session, &damageHeader, &damagePacket, true);
					// 죽은 경우
					if (targetPlayer->hp <= 0)
					{
						// 삭제 패킷 브로드캐스트
						CPacket deletePacket;
						st_PACKET_HEADER deleteHeader;
						mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->sessionID);
						SectorManager::GetInstance()->SendAround(session, &deleteHeader, &deletePacket, true);
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

	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;

	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack3(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &sendAttackHeader, &sendAttackPacket);
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
	int startSectorX = leftUpX / dfRANGE_SECTOR_RIGHT;
	int startSectorY = leftUpY / dfRANGE_SECTOR_BOTTOM;
	int endSectorX = rightDownX / dfRANGE_SECTOR_RIGHT;
	int endSectorY = rightDownY / dfRANGE_SECTOR_BOTTOM;

	// 걸치는 섹터 범위를 순회
	for (int sectorX = startSectorX; sectorX <= endSectorX; ++sectorX)
	{
		for (int sectorY = startSectorY; sectorY <= endSectorY; ++sectorY)
		{
			// 각 섹터의 객체들을 순회하며 대상 찾기
			const auto& playerUMap = SectorManager::GetInstance()->GetSectorPlayerMap(sectorY, sectorX);
			for (auto& pair : playerUMap)
			{
				if (pair.second == player)
					continue;
				// 대상이 공격 범위 안에 있는지 확인
				if (pair.second->x >= leftUpX && pair.second->x <= rightDownX &&
					pair.second->y >= leftUpY && pair.second->y <= rightDownY)
				{
					targetPlayer = pair.second;
					targetPlayer->hp -= dfATTACK3_DAMAGE;
					// 데미지 패킷 및 헤더 생성, 전송
					CPacket damagePacket;
					st_PACKET_HEADER damageHeader;
					mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
					SectorManager::GetInstance()->SendAround(session, &damageHeader, &damagePacket, true);
					// 죽은 경우
					if (targetPlayer->hp <= 0)
					{
						// 삭제 패킷 브로드캐스트
						CPacket deletePacket;
						st_PACKET_HEADER deleteHeader;
						mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->sessionID);
						SectorManager::GetInstance()->SendAround(session, &deleteHeader, &deletePacket, true);
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
	CPacket sendEchoPacket;
	st_PACKET_HEADER sendEchoHeader;
	unsigned int id;

	*packet >> id;

	// 헤더 및 페이로드 생성 및 전송
	mpEcho(&sendEchoHeader, &sendEchoPacket, id);
	return true;
}

