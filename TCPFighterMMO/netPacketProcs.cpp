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
		NetworkManager::GetInstance()->Disconnect(session);
		return false;
	}
	player->moveFlag = true;
	player->dir = Direction;
	player->x = x;
	player->y = y;
	// 헤더 및 페이로드 생성 및 전송
	st_PACKET_HEADER pMoveHeader;
	CPacket pMovePacket;

	mpMoveStart(&pMoveHeader, &pMovePacket, Direction, x, y, player->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &pMoveHeader, &pMovePacket);
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
		NetworkManager::GetInstance()->Disconnect(session);
		return false;
	}
	player->moveFlag = false;
	player->dir = Direction;
	player->x = x;
	player->y = y;
	// 헤더 및 페이로드 생성 및 전송
	CPacket scMoveStopPacket;
	st_PACKET_HEADER scMoveStopHeader;
	mpMoveStop(&scMoveStopHeader, &scMoveStopPacket, Direction, x, y, session->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &scMoveStopHeader, &scMoveStopPacket);

	return true;
}

bool netPacketProc_Attack1(Session* session, CPacket* pPacket)
{

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);

	// 받은 패킷 해석
	// cout << "# PACKET_RECV # SessionID:" << player->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK1 # SessionID :" << player->session->id << " X : " << player->x << " Y : " << player->y << endl;
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	*pPacket >> Direction >> x >> y;


	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack1(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &sendAttackHeader, &sendAttackPacket);
	// 피격 대상 탐색
	Player* targetPlayer = nullptr;
	for (auto& pair : ObjectManager::GetInstance()->GetObjectMap())
	{
		Player* searchPlayer = pair.second;
		// 본인 제외
		if (searchPlayer == player)
			continue;
		// 왼쪽 방향 공격
		if (player->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((player->x - searchPlayer->x) >= 0 &&
				(player->x - searchPlayer->x) < dfATTACK1_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK1_RANGE_Y) // y 좌표 절대값 비교
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// 오른쪽 방향 공격
		else if (player->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - player->x) >= 0 &&
				(searchPlayer->x - player->x) < dfATTACK1_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK1_RANGE_Y) // y 좌표 절대값 비교
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
	}
	// 못찾았으면 리턴
	if (targetPlayer == nullptr)
		return true;
	targetPlayer->hp -= dfATTACK1_DAMAGE;
	// 데미지 패킷 및 헤더 생성, 전송
	CPacket damagePacket;
	st_PACKET_HEADER damageHeader;
	mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// 죽은 경우
	if (targetPlayer->hp <= 0)
	{
		// 삭제 패킷 브로드캐스트
		CPacket deletePacket;
		st_PACKET_HEADER deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->sessionID);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		NetworkManager::GetInstance()->Disconnect(targetPlayer->session);
	}
	return true;
}

// 아래 2, 3 코드는 위 코드와 로직 동일
bool netPacketProc_Attack2(Session* session, CPacket* pPacket)
{

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);

	// 받은 패킷 해석
	// cout << "# PACKET_RECV # SessionID:" << player->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK2 # SessionID :" << player->session->id << " X : " << player->x << " Y : " << player->y << endl;
	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;


	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack2(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &sendAttackHeader, &sendAttackPacket);
	// 피격 대상 탐색
	Player* targetPlayer = nullptr;
	for (auto& pair : ObjectManager::GetInstance()->GetObjectMap())
	{
		Player* searchPlayer = pair.second;
		// 본인 제외
		if (searchPlayer == player)
			continue;
		// 왼쪽 방향 공격
		if (player->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((player->x - searchPlayer->x) >= 0 &&
				(player->x - searchPlayer->x) < dfATTACK2_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK2_RANGE_Y) // y 좌표 절대값 비교
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// 오른쪽 방향 공격
		else if (player->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - player->x) >= 0 &&
				(searchPlayer->x - player->x) < dfATTACK2_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK2_RANGE_Y) // y 좌표 절대값 비교
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
	}
	// 못찾았으면 리턴
	if (targetPlayer == nullptr)
		return true;
	targetPlayer->hp -= dfATTACK2_DAMAGE;
	// 데미지 패킷 및 헤더 생성, 전송
	CPacket damagePacket;
	st_PACKET_HEADER damageHeader;
	mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// 죽은 경우
	if (targetPlayer->hp <= 0)
	{
		// 삭제 패킷 브로드캐스트
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

	// 받은 패킷 해석
	// cout << "# PACKET_RECV # SessionID:" << player->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK3 # SessionID :" << player->session->id << " X : " << player->x << " Y : " << player->y << endl;
	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;


	player->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket sendAttackPacket;
	st_PACKET_HEADER sendAttackHeader;
	mpAttack3(&sendAttackHeader, &sendAttackPacket, player->dir, player->x, player->y, session->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &sendAttackHeader, &sendAttackPacket);
	// 피격 대상 탐색
	Player* targetPlayer = nullptr;
	for (auto& pair : ObjectManager::GetInstance()->GetObjectMap())
	{
		Player* searchPlayer = pair.second;
		// 본인 제외
		if (searchPlayer == player)
			continue;
		// 왼쪽 방향 공격
		if (player->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((player->x - searchPlayer->x) >= 0 &&
				(player->x - searchPlayer->x) < dfATTACK3_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK3_RANGE_Y) // y 좌표 절대값 비교
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// 오른쪽 방향 공격
		else if (player->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - player->x) >= 0 &&
				(searchPlayer->x - player->x) < dfATTACK3_RANGE_X &&
				abs(player->y - searchPlayer->y) < dfATTACK3_RANGE_Y) // y 좌표 절대값 비교
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
	}
	// 못찾았으면 리턴
	if (targetPlayer == nullptr)
		return true;
	targetPlayer->hp -= dfATTACK3_DAMAGE;
	// 데미지 패킷 및 헤더 생성, 전송
	CPacket damagePacket;
	st_PACKET_HEADER damageHeader;
	mpDamage(&damageHeader, &damagePacket, session->sessionID, targetPlayer->session->sessionID, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// 죽은 경우
	if (targetPlayer->hp <= 0)
	{
		// 삭제 패킷 브로드캐스트
		CPacket deletePacket;
		st_PACKET_HEADER deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->sessionID);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		NetworkManager::GetInstance()->Disconnect(targetPlayer->session);
	}
	return true;
}

