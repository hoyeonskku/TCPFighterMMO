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

	// ���� ��Ŷ �ؼ�
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);

	*pPacket >> Direction >> x >> y;
	// Ŭ���̾�Ʈ ��ǥ�� ���� �ּ����� ����üũ
	if ((abs(x - player->x) > dfERROR_RANGE) ||
		(abs(y - player->y) > dfERROR_RANGE))
	{
		SynchronizePos(player);
	}
	player->moveFlag = true;
	player->dir = Direction;
	player->x = x;
	player->y = y;
	// ��� �� ���̷ε� ���� �� ����
	st_PACKET_HEADER pMoveHeader;
	CPacket pMovePacket;

	mpMoveStart(&pMoveHeader, &pMovePacket, Direction, player->x, player->y, player->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &pMoveHeader, &pMovePacket);
	return true;
}

bool netPacketProc_MoveStop(Session* session, CPacket* pPacket)
{

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	// ���� ��Ŷ �ؼ�
	unsigned char Direction;
	unsigned short x;
	unsigned short y;


	*pPacket >> Direction >> x >> y;

	// cout << "# PACKET_RECV # SessionID:" << player->session->id << endl;
	// cout << "dfPACKET_CS_MOVE_STOP # SessionID :" << player->session->id << " X : " << player->x << " Y : " << player->y << endl;
	// Ŭ���̾�Ʈ ��ǥ�� ���� �ּ����� ����üũ
	if ((abs(x - player->x) > dfERROR_RANGE) ||
		(abs(y - player->y) > dfERROR_RANGE))
	{
		SynchronizePos(player);
	}
	player->moveFlag = false;
	player->dir = Direction;
	// ��� �� ���̷ε� ���� �� ����
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
	// ��� �� ���̷ε� ���� �� ����
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack1(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &sendAttackHeader, &sendAttackPacket);
	// �ǰ� ��� Ž��
	Player* targetPlayer = nullptr;
	int leftUpY;
	int leftUpX;
	int rightDownY;
	int rightDownX;

	// ���� ���⿡ ���� ���� ����
	if (player->dir == dfPACKET_MOVE_DIR_LL)
	{
		// ���� ���� ����
		leftUpY = player->y - dfATTACK1_RANGE_Y;
		leftUpX = player->x - dfATTACK1_RANGE_X;
		rightDownY = player->y + dfATTACK1_RANGE_Y;
		rightDownX = player->x;
	}
	else if (player->dir == dfPACKET_MOVE_DIR_RR)
	{
		// ������ ���� ����
		leftUpY = player->y - dfATTACK1_RANGE_Y;
		leftUpX = player->x;
		rightDownY = player->y + dfATTACK1_RANGE_Y;
		rightDownX = player->x + dfATTACK1_RANGE_X;
	}
	else
	{
		// �ٸ� ������ ó������ ����
		return false;
	}

	// ���� ������ ���ϴ� ���� ��ǥ ã��
	int startSectorX = leftUpX / dfRANGE_SECTOR_RIGHT;
	int startSectorY = leftUpY / dfRANGE_SECTOR_BOTTOM;
	int endSectorX = rightDownX / dfRANGE_SECTOR_RIGHT;
	int endSectorY = rightDownY / dfRANGE_SECTOR_BOTTOM;

	// ��ġ�� ���� ������ ��ȸ
	for (int sectorX = startSectorX; sectorX <= endSectorX; ++sectorX)
	{
		for (int sectorY = startSectorY; sectorY <= endSectorY; ++sectorY)
		{
			// �� ������ ��ü���� ��ȸ�ϸ� ��� ã��
			const auto& playerUMap = SectorManager::GetInstance()->GetSectorPlayerMap(sectorY, sectorX);
			for (auto& pair : playerUMap)
			{
				if (pair.second == player)
					continue;
				// ����� ���� ���� �ȿ� �ִ��� Ȯ��
				if (pair.second->x >= leftUpX && pair.second->x <= rightDownX &&
					pair.second->y >= leftUpY && pair.second->y <= rightDownY)
				{
					targetPlayer = pair.second;
					targetPlayer->hp -= dfATTACK1_DAMAGE;
					// ������ ��Ŷ �� ��� ����, ����
					CPacket damagePacket;
					st_PACKET_HEADER damageHeader;
					mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
					SectorManager::GetInstance()->SendAround(session, &damageHeader, &damagePacket, true);
					// ���� ���
					if (targetPlayer->hp <= 0)
					{
						// ���� ��Ŷ ��ε�ĳ��Ʈ
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
	// ��� �� ���̷ε� ���� �� ����
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack2(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &sendAttackHeader, &sendAttackPacket);
	// �ǰ� ��� Ž��
	Player* targetPlayer = nullptr;
	int leftUpY;
	int leftUpX;
	int rightDownY;
	int rightDownX;

	// ���� ���⿡ ���� ���� ����
	if (player->dir == dfPACKET_MOVE_DIR_LL)
	{
		// ���� ���� ����
		leftUpY = player->y - dfATTACK2_RANGE_Y;
		leftUpX = player->x - dfATTACK2_RANGE_X;
		rightDownY = player->y + dfATTACK2_RANGE_Y;
		rightDownX = player->x;
	}
	else if (player->dir == dfPACKET_MOVE_DIR_RR)
	{
		// ������ ���� ����
		leftUpY = player->y - dfATTACK2_RANGE_Y;
		leftUpX = player->x;
		rightDownY = player->y + dfATTACK2_RANGE_Y;
		rightDownX = player->x + dfATTACK2_RANGE_X;
	}
	else
	{
		// �ٸ� ������ ó������ ����
		return false;
	}

	// ���� ������ ���ϴ� ���� ��ǥ ã��
	int startSectorX = leftUpX / dfRANGE_SECTOR_RIGHT;
	int startSectorY = leftUpY / dfRANGE_SECTOR_BOTTOM;
	int endSectorX = rightDownX / dfRANGE_SECTOR_RIGHT;
	int endSectorY = rightDownY / dfRANGE_SECTOR_BOTTOM;

	// ��ġ�� ���� ������ ��ȸ
	for (int sectorX = startSectorX; sectorX <= endSectorX; ++sectorX)
	{
		for (int sectorY = startSectorY; sectorY <= endSectorY; ++sectorY)
		{
			// �� ������ ��ü���� ��ȸ�ϸ� ��� ã��
			const auto& playerUMap = SectorManager::GetInstance()->GetSectorPlayerMap(sectorY, sectorX);
			for (auto& pair : playerUMap)
			{
				if (pair.second == player)
					continue;
				// ����� ���� ���� �ȿ� �ִ��� Ȯ��
				if (pair.second->x >= leftUpX && pair.second->x <= rightDownX &&
					pair.second->y >= leftUpY && pair.second->y <= rightDownY)
				{
					targetPlayer = pair.second;
					targetPlayer->hp -= dfATTACK2_DAMAGE;
					// ������ ��Ŷ �� ��� ����, ����
					CPacket damagePacket;
					st_PACKET_HEADER damageHeader;
					mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
					SectorManager::GetInstance()->SendAround(session, &damageHeader, &damagePacket, true);
					// ���� ���
					if (targetPlayer->hp <= 0)
					{
						// ���� ��Ŷ ��ε�ĳ��Ʈ
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
	// ��� �� ���̷ε� ���� �� ����
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack3(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &sendAttackHeader, &sendAttackPacket);
	// �ǰ� ��� Ž��
	Player* targetPlayer = nullptr;
	int leftUpY;
	int leftUpX;
	int rightDownY;
	int rightDownX;

	// ���� ���⿡ ���� ���� ����
	if (player->dir == dfPACKET_MOVE_DIR_LL)
	{
		// ���� ���� ����
		leftUpY = player->y - dfATTACK3_RANGE_Y;
		leftUpX = player->x - dfATTACK3_RANGE_X;
		rightDownY = player->y + dfATTACK3_RANGE_Y;
		rightDownX = player->x;
	}
	else if (player->dir == dfPACKET_MOVE_DIR_RR)
	{
		// ������ ���� ����
		leftUpY = player->y - dfATTACK3_RANGE_Y;
		leftUpX = player->x;
		rightDownY = player->y + dfATTACK3_RANGE_Y;
		rightDownX = player->x + dfATTACK3_RANGE_X;
	}
	else
	{
		// �ٸ� ������ ó������ ����
		return false;
	}

	// ���� ������ ���ϴ� ���� ��ǥ ã��
	int startSectorX = leftUpX / dfRANGE_SECTOR_RIGHT;
	int startSectorY = leftUpY / dfRANGE_SECTOR_BOTTOM;
	int endSectorX = rightDownX / dfRANGE_SECTOR_RIGHT;
	int endSectorY = rightDownY / dfRANGE_SECTOR_BOTTOM;

	// ��ġ�� ���� ������ ��ȸ
	for (int sectorX = startSectorX; sectorX <= endSectorX; ++sectorX)
	{
		for (int sectorY = startSectorY; sectorY <= endSectorY; ++sectorY)
		{
			// �� ������ ��ü���� ��ȸ�ϸ� ��� ã��
			const auto& playerUMap = SectorManager::GetInstance()->GetSectorPlayerMap(sectorY, sectorX);
			for (auto& pair : playerUMap)
			{
				if (pair.second == player)
					continue;
				// ����� ���� ���� �ȿ� �ִ��� Ȯ��
				if (pair.second->x >= leftUpX && pair.second->x <= rightDownX &&
					pair.second->y >= leftUpY && pair.second->y <= rightDownY)
				{
					targetPlayer = pair.second;
					targetPlayer->hp -= dfATTACK3_DAMAGE;
					// ������ ��Ŷ �� ��� ����, ����
					CPacket damagePacket;
					st_PACKET_HEADER damageHeader;
					mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
					SectorManager::GetInstance()->SendAround(session, &damageHeader, &damagePacket, true);
					// ���� ���
					if (targetPlayer->hp <= 0)
					{
						// ���� ��Ŷ ��ε�ĳ��Ʈ
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

	// ��� �� ���̷ε� ���� �� ����
	mpEcho(&sendEchoHeader, &sendEchoPacket, id);
	return true;
}

