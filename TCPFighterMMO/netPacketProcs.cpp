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
bool netPacketProc_MoveStart(Player* pPlayer, CPacket* pPacket)
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
	PacketHeader pMoveHeader;
	CPacket pMovePacket;

	mpMoveStart(&pMoveHeader, &pMovePacket, Direction, x, y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &pMoveHeader, &pMovePacket);
	return true;
}

bool netPacketProc_MoveStop(Player* pPlayer, CPacket* pPacket)
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
	PacketHeader scMoveStopHeader;
	mpMoveStop(&scMoveStopHeader, &scMoveStopPacket, Direction, x, y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &scMoveStopHeader, &scMoveStopPacket);

	return true;
}

bool netPacketProc_Attack1(Player* pPlayer, CPacket* pPacket)
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
	PacketHeader sendAttackHeader;
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
	targetPlayer->hp -= 10;
	// ������ ��Ŷ �� ��� ����, ����
	CPacket damagePacket;
	PacketHeader damageHeader;
	mpDamage(&damageHeader, &damagePacket, pPlayer->session->id, targetPlayer->session->id, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// ���� ���
	if (targetPlayer->hp <= 0)
	{
		// ���� ��Ŷ ��ε�ĳ��Ʈ
		CPacket deletePacket;
		PacketHeader deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->id);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		DisconnectByPlayer(targetPlayer);
	}
	return true;
}

// �Ʒ� 2, 3 �ڵ�� �� �ڵ�� ���� ����
bool netPacketProc_Attack2(Player* pPlayer, CPacket* pPacket)
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
	PacketHeader sendAttackHeader;
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
	targetPlayer->hp -= 20;
	// ������ ��Ŷ �� ��� ����, ����
	CPacket damagePacket;
	PacketHeader damageHeader;
	mpDamage(&damageHeader, &damagePacket, pPlayer->session->id, targetPlayer->session->id, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// ���� ���
	if (targetPlayer->hp <= 0)
	{
		// ���� ��Ŷ ��ε�ĳ��Ʈ
		CPacket deletePacket;
		PacketHeader deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->id);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		DisconnectByPlayer(targetPlayer);
	}
	return true;
}

bool netPacketProc_Attack3(Player* pPlayer, CPacket* pPacket)
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
	PacketHeader sendAttackHeader;
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
	targetPlayer->hp -= 30;
	// ������ ��Ŷ �� ��� ����, ����
	CPacket damagePacket;
	PacketHeader damageHeader;
	mpDamage(&damageHeader, &damagePacket, pPlayer->session->id, targetPlayer->session->id, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// ���� ���
	if (targetPlayer->hp <= 0)
	{
		// ���� ��Ŷ ��ε�ĳ��Ʈ
		CPacket deletePacket;
		PacketHeader deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->id);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		DisconnectByPlayer(targetPlayer);
	}
	return true;
}