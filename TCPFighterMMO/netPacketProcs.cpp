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

#define dfATTACK1_DAMAGE		10
#define dfATTACK2_DAMAGE		20
#define dfATTACK3_DAMAGE		30
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
		NetworkManager::GetInstance()->Disconnect(session);
		return false;
	}
	player->moveFlag = true;
	player->dir = Direction;
	player->x = x;
	player->y = y;
	// ��� �� ���̷ε� ���� �� ����
	st_PACKET_HEADER pMoveHeader;
	CPacket pMovePacket;

	mpMoveStart(&pMoveHeader, &pMovePacket, Direction, x, y, player->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &pMoveHeader, &pMovePacket);
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
		NetworkManager::GetInstance()->Disconnect(session);
		return false;
	}
	player->moveFlag = false;
	player->dir = Direction;
	player->x = x;
	player->y = y;
	// ��� �� ���̷ε� ���� �� ����
	CPacket scMoveStopPacket;
	st_PACKET_HEADER scMoveStopHeader;
	mpMoveStop(&scMoveStopHeader, &scMoveStopPacket, Direction, x, y, session->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &scMoveStopHeader, &scMoveStopPacket);

	return true;
}

bool netPacketProc_Attack1(Session* session, CPacket* pPacket)
{

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);

	// ���� ��Ŷ �ؼ�
	// cout << "# PACKET_RECV # SessionID:" << player->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK1 # SessionID :" << player->session->id << " X : " << player->x << " Y : " << player->y << endl;
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	*pPacket >> Direction >> x >> y;


	player->dir = Direction;
	// ��� �� ���̷ε� ���� �� ����
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack1(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &sendAttackHeader, &sendAttackPacket);
	// �ǰ� ��� Ž��
	Player* targetPlayer = nullptr;
	for (auto& pair : ObjectManager::GetInstance()->GetObjectMap())
	{
		Player* searchPlayer = pair.second;
		// ���� ����
		if (searchPlayer == player)
			continue;
		// ���� ���� ����
		if (player->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((player->x - searchPlayer->x) >= 0 &&
				(player->x - searchPlayer->x) < dfATTACK1_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK1_RANGE_Y) // y ��ǥ ���밪 ��
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// ������ ���� ����
		else if (player->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - player->x) >= 0 &&
				(searchPlayer->x - player->x) < dfATTACK1_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK1_RANGE_Y) // y ��ǥ ���밪 ��
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
	mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// ���� ���
	if (targetPlayer->hp <= 0)
	{
		// ���� ��Ŷ ��ε�ĳ��Ʈ
		CPacket deletePacket;
		st_PACKET_HEADER deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->sessionID);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		NetworkManager::GetInstance()->Disconnect(targetPlayer->session);
	}
	return true;
}

// �Ʒ� 2, 3 �ڵ�� �� �ڵ�� ���� ����
bool netPacketProc_Attack2(Session* session, CPacket* pPacket)
{

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);

	// ���� ��Ŷ �ؼ�
	// cout << "# PACKET_RECV # SessionID:" << player->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK2 # SessionID :" << player->session->id << " X : " << player->x << " Y : " << player->y << endl;
	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;


	player->dir = Direction;
	// ��� �� ���̷ε� ���� �� ����
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack2(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &sendAttackHeader, &sendAttackPacket);
	// �ǰ� ��� Ž��
	Player* targetPlayer = nullptr;
	for (auto& pair : ObjectManager::GetInstance()->GetObjectMap())
	{
		Player* searchPlayer = pair.second;
		// ���� ����
		if (searchPlayer == player)
			continue;
		// ���� ���� ����
		if (player->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((player->x - searchPlayer->x) >= 0 &&
				(player->x - searchPlayer->x) < dfATTACK2_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK2_RANGE_Y) // y ��ǥ ���밪 ��
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// ������ ���� ����
		else if (player->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - player->x) >= 0 &&
				(searchPlayer->x - player->x) < dfATTACK2_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK2_RANGE_Y) // y ��ǥ ���밪 ��
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
	mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// ���� ���
	if (targetPlayer->hp <= 0)
	{
		// ���� ��Ŷ ��ε�ĳ��Ʈ
		CPacket deletePacket;
		st_PACKET_HEADER deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->sessionID);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		NetworkManager::GetInstance()->Disconnect(targetPlayer->session);
	}
	return true;
}

bool netPacketProc_Attack3(Session* session, CPacket* pPacket)
{

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);

	// ���� ��Ŷ �ؼ�
	// cout << "# PACKET_RECV # SessionID:" << player->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK3 # SessionID :" << player->session->id << " X : " << player->x << " Y : " << player->y << endl;
	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;


	player->dir = Direction;
	// ��� �� ���̷ε� ���� �� ����
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack3(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &sendAttackHeader, &sendAttackPacket);
	// �ǰ� ��� Ž��
	Player* targetPlayer = nullptr;
	for (auto& pair : ObjectManager::GetInstance()->GetObjectMap())
	{
		Player* searchPlayer = pair.second;
		// ���� ����
		if (searchPlayer == player)
			continue;
		// ���� ���� ����
		if (player->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((player->x - searchPlayer->x) >= 0 &&
				(player->x - searchPlayer->x) < dfATTACK3_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK3_RANGE_Y) // y ��ǥ ���밪 ��
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// ������ ���� ����
		else if (player->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - player->x) >= 0 &&
				(searchPlayer->x - player->x) < dfATTACK3_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK3_RANGE_Y) // y ��ǥ ���밪 ��
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
	mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// ���� ���
	if (targetPlayer->hp <= 0)
	{
		// ���� ��Ŷ ��ε�ĳ��Ʈ
		CPacket deletePacket;
		st_PACKET_HEADER deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->sessionID);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		NetworkManager::GetInstance()->Disconnect(targetPlayer->session);
	}
	return true;
}

