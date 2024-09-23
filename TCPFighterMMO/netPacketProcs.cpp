#include "pch.h"
#include "netPacketProcs.h"
#include <iostream>
#include "Session.h"
#include "Player.h"
#include "PacketDefine.h"
#include "MakePacket.h"
#include "NetworkManager.h"
#include "ObjectManager.h"
#include "SerializingBuffer.h"
#include "ContentPacketProcessor.h"

using namespace std;


#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3
#define dfERROR_RANGE		50
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20

bool netPacketProc_MoveStart(Session* session, CPacket* pPacket)
{
	// cout << "# PACKET_RECV # SessionID:" << pPlayer->session->id << endl;
	// cout << "dfPACKET_CS_MOVE_START # SessionID :" << pPlayer->session->id << " X : " << pPlayer->x << " Y : " << pPlayer->y << endl;

	// ���� ��Ŷ �ؼ�
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	*pPacket >> Direction >> x >> y;
	// Ŭ���̾�Ʈ ��ǥ�� ���� �ּ����� ����üũ
	if ((abs(x - pPlayer->x) > dfERROR_RANGE) ||
		(abs(y - pPlayer->y) > dfERROR_RANGE))
	{
		DisconnectByPlayer(pPlayer);
		return false;
	}
	pPlayer->moveFlag = true;
	pPlayer->dir = Direction;
	pPlayer->x = x;
	pPlayer->y = y;
	// ��� �� ���̷ε� ���� �� ����
	st_PACKET_HEADER pMoveHeader;
	CPacket pMovePacket;

	mpMoveStart(&pMoveHeader, &pMovePacket, Direction, x, y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &pMoveHeader, &pMovePacket);
	return true;
}

bool netPacketProc_MoveStop(Session* session, CPacket* pPacket)
{
	// ���� ��Ŷ �ؼ�
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	*pPacket >> Direction >> x >> y;

	// cout << "# PACKET_RECV # SessionID:" << pPlayer->session->id << endl;
	// cout << "dfPACKET_CS_MOVE_STOP # SessionID :" << pPlayer->session->id << " X : " << pPlayer->x << " Y : " << pPlayer->y << endl;
	// Ŭ���̾�Ʈ ��ǥ�� ���� �ּ����� ����üũ
	if ((abs(x - pPlayer->x) > dfERROR_RANGE) ||
		(abs(y - pPlayer->y) > dfERROR_RANGE))
	{
		DisconnectByPlayer(pPlayer);
		return false;
	}
	pPlayer->moveFlag = false;
	pPlayer->dir = Direction;
	pPlayer->x = x;
	pPlayer->y = y;
	// ��� �� ���̷ε� ���� �� ����
	CPacket scMoveStopPacket;
	st_PACKET_HEADER scMoveStopHeader;
	mpMoveStop(&scMoveStopHeader, &scMoveStopPacket, Direction, x, y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &scMoveStopHeader, &scMoveStopPacket);

	return true;
}

bool netPacketProc_Attack1(Session* session, CPacket* pPacket)
{
	// ���� ��Ŷ �ؼ�
	// cout << "# PACKET_RECV # SessionID:" << pPlayer->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK1 # SessionID :" << pPlayer->session->id << " X : " << pPlayer->x << " Y : " << pPlayer->y << endl;
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	*pPacket >> Direction >> x >> y;


	pPlayer->dir = Direction;
	// ��� �� ���̷ε� ���� �� ����
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack1(&sendAttackHeader, &sendAttackPacket, pPlayer->dir, pPlayer->x, pPlayer->y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &sendAttackHeader, &sendAttackPacket);
	// �ǰ� ��� Ž��
	Player* targetPlayer = nullptr;
	for (auto& searchPlayer : ObjectManager::GetInstance()->GetObjectList())
	{
		// ���� ����
		if (searchPlayer == pPlayer)
			continue;
		// ���� ���� ����
		if (pPlayer->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((pPlayer->x - searchPlayer->x) >= 0 &&
				(pPlayer->x - searchPlayer->x) < dfATTACK1_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK1_RANGE_Y) // y ��ǥ ���밪 ��
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// ������ ���� ����
		else if (pPlayer->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - pPlayer->x) >= 0 &&
				(searchPlayer->x - pPlayer->x) < dfATTACK1_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK1_RANGE_Y) // y ��ǥ ���밪 ��
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
	}
	// ��ã������ ����
	if (targetPlayer == nullptr)
		return true;
	targetPlayer->hp -= dfATTACK1_DAMAGE;
	// ������ ��Ŷ �� ��� ����, ����
	CPacket damagePacket;
	st_PACKET_HEADER damageHeader;
	mpDamage(&damageHeader, &damagePacket, pPlayer->session->id, targetPlayer->session->id, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// ���� ���
	if (targetPlayer->hp <= 0)
	{
		// ���� ��Ŷ ��ε�ĳ��Ʈ
		CPacket deletePacket;
		st_PACKET_HEADER deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->id);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		DisconnectByPlayer(targetPlayer);
	}
	return true;
}

// �Ʒ� 2, 3 �ڵ�� �� �ڵ�� ���� ����
bool netPacketProc_Attack2(Session* session, CPacket* pPacket)
{
	// ���� ��Ŷ �ؼ�
	// cout << "# PACKET_RECV # SessionID:" << pPlayer->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK2 # SessionID :" << pPlayer->session->id << " X : " << pPlayer->x << " Y : " << pPlayer->y << endl;
	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;


	pPlayer->dir = Direction;
	// ��� �� ���̷ε� ���� �� ����
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack2(&sendAttackHeader, &sendAttackPacket, pPlayer->dir, pPlayer->x, pPlayer->y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &sendAttackHeader, &sendAttackPacket);
	// �ǰ� ��� Ž��
	Player* targetPlayer = nullptr;
	for (auto& searchPlayer : ObjectManager::GetInstance()->GetObjectList())
	{
		// ���� ����
		if (searchPlayer == pPlayer)
			continue;
		// ���� ���� ����
		if (pPlayer->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((pPlayer->x - searchPlayer->x) >= 0 &&
				(pPlayer->x - searchPlayer->x) < dfATTACK2_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK2_RANGE_Y) // y ��ǥ ���밪 ��
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// ������ ���� ����
		else if (pPlayer->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - pPlayer->x) >= 0 &&
				(searchPlayer->x - pPlayer->x) < dfATTACK2_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK2_RANGE_Y) // y ��ǥ ���밪 ��
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
	}
	// ��ã������ ����
	if (targetPlayer == nullptr)
		return true;
	targetPlayer->hp -= dfATTACK2_DAMAGE;
	// ������ ��Ŷ �� ��� ����, ����
	CPacket damagePacket;
	st_PACKET_HEADER damageHeader;
	mpDamage(&damageHeader, &damagePacket, pPlayer->session->id, targetPlayer->session->id, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// ���� ���
	if (targetPlayer->hp <= 0)
	{
		// ���� ��Ŷ ��ε�ĳ��Ʈ
		CPacket deletePacket;
		st_PACKET_HEADER deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->id);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		DisconnectByPlayer(targetPlayer);
	}
	return true;
}

bool netPacketProc_Attack3(Session* session, CPacket* pPacket)
{
	// ���� ��Ŷ �ؼ�
	// cout << "# PACKET_RECV # SessionID:" << pPlayer->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK3 # SessionID :" << pPlayer->session->id << " X : " << pPlayer->x << " Y : " << pPlayer->y << endl;
	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;


	pPlayer->dir = Direction;
	// ��� �� ���̷ε� ���� �� ����
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack3(&sendAttackHeader, &sendAttackPacket, pPlayer->dir, pPlayer->x, pPlayer->y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &sendAttackHeader, &sendAttackPacket);
	// �ǰ� ��� Ž��
	Player* targetPlayer = nullptr;
	for (auto& searchPlayer : ObjectManager::GetInstance()->GetObjectList())
	{
		// ���� ����
		if (searchPlayer == pPlayer)
			continue;
		// ���� ���� ����
		if (pPlayer->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((pPlayer->x - searchPlayer->x) >= 0 &&
				(pPlayer->x - searchPlayer->x) < dfATTACK3_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK3_RANGE_Y) // y ��ǥ ���밪 ��
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// ������ ���� ����
		else if (pPlayer->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - pPlayer->x) >= 0 &&
				(searchPlayer->x - pPlayer->x) < dfATTACK3_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK3_RANGE_Y) // y ��ǥ ���밪 ��
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
	}
	// ��ã������ ����
	if (targetPlayer == nullptr)
		return true;
	targetPlayer->hp -= dfATTACK3_DAMAGE;
	// ������ ��Ŷ �� ��� ����, ����
	CPacket damagePacket;
	st_PACKET_HEADER damageHeader;
	mpDamage(&damageHeader, &damagePacket, pPlayer->session->id, targetPlayer->session->id, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// ���� ���
	if (targetPlayer->hp <= 0)
	{
		// ���� ��Ŷ ��ε�ĳ��Ʈ
		CPacket deletePacket;
		st_PACKET_HEADER deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->id);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		DisconnectByPlayer(targetPlayer);
	}
	return true;
}