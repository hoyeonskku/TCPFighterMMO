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

	// 받은 패킷 해석
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	*pPacket >> Direction >> x >> y;
	// 클라이언트 좌표에 대한 최소한의 에러체크
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
	// 헤더 및 페이로드 생성 및 전송
	PacketHeader pMoveHeader;
	CPacket pMovePacket;

	mpMoveStart(&pMoveHeader, &pMovePacket, Direction, x, y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &pMoveHeader, &pMovePacket);
	return true;
}

bool netPacketProc_MoveStop(Player* pPlayer, CPacket* pPacket)
{
	// 받은 패킷 해석
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	*pPacket >> Direction >> x >> y;

	// cout << "# PACKET_RECV # SessionID:" << pPlayer->session->id << endl;
	// cout << "dfPACKET_CS_MOVE_STOP # SessionID :" << pPlayer->session->id << " X : " << pPlayer->x << " Y : " << pPlayer->y << endl;
	// 클라이언트 좌표에 대한 최소한의 에러체크
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
	// 헤더 및 페이로드 생성 및 전송
	CPacket scMoveStopPacket;
	PacketHeader scMoveStopHeader;
	mpMoveStop(&scMoveStopHeader, &scMoveStopPacket, Direction, x, y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &scMoveStopHeader, &scMoveStopPacket);

	return true;
}

bool netPacketProc_Attack1(Player* pPlayer, CPacket* pPacket)
{
	// 받은 패킷 해석
	// cout << "# PACKET_RECV # SessionID:" << pPlayer->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK1 # SessionID :" << pPlayer->session->id << " X : " << pPlayer->x << " Y : " << pPlayer->y << endl;
	unsigned char Direction;
	unsigned short x;
	unsigned short y;

	*pPacket >> Direction >> x >> y;


	pPlayer->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket sendAttackPacket;
	PacketHeader sendAttackHeader;
	mpAttack1(&sendAttackHeader, &sendAttackPacket, pPlayer->dir, pPlayer->x, pPlayer->y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &sendAttackHeader, &sendAttackPacket);
	// 피격 대상 탐색
	Player* targetPlayer = nullptr;
	for (auto& searchPlayer : ObjectManager::GetInstance()->GetObjectList())
	{
		// 본인 제외
		if (searchPlayer == pPlayer)
			continue;
		// 왼쪽 방향 공격
		if (pPlayer->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((pPlayer->x - searchPlayer->x) >= 0 &&
				(pPlayer->x - searchPlayer->x) < dfATTACK1_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK1_RANGE_Y) // y 좌표 절대값 비교
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// 오른쪽 방향 공격
		else if (pPlayer->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - pPlayer->x) >= 0 &&
				(searchPlayer->x - pPlayer->x) < dfATTACK1_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK1_RANGE_Y) // y 좌표 절대값 비교
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
	targetPlayer->hp -= 10;
	// 데미지 패킷 및 헤더 생성, 전송
	CPacket damagePacket;
	PacketHeader damageHeader;
	mpDamage(&damageHeader, &damagePacket, pPlayer->session->id, targetPlayer->session->id, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// 죽은 경우
	if (targetPlayer->hp <= 0)
	{
		// 삭제 패킷 브로드캐스트
		CPacket deletePacket;
		PacketHeader deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->id);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		DisconnectByPlayer(targetPlayer);
	}
	return true;
}

// 아래 2, 3 코드는 위 코드와 로직 동일
bool netPacketProc_Attack2(Player* pPlayer, CPacket* pPacket)
{
	// 받은 패킷 해석
	// cout << "# PACKET_RECV # SessionID:" << pPlayer->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK2 # SessionID :" << pPlayer->session->id << " X : " << pPlayer->x << " Y : " << pPlayer->y << endl;
	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;


	pPlayer->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket sendAttackPacket;
	PacketHeader sendAttackHeader;
	mpAttack2(&sendAttackHeader, &sendAttackPacket, pPlayer->dir, pPlayer->x, pPlayer->y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &sendAttackHeader, &sendAttackPacket);
	// 피격 대상 탐색
	Player* targetPlayer = nullptr;
	for (auto& searchPlayer : ObjectManager::GetInstance()->GetObjectList())
	{
		// 본인 제외
		if (searchPlayer == pPlayer)
			continue;
		// 왼쪽 방향 공격
		if (pPlayer->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((pPlayer->x - searchPlayer->x) >= 0 &&
				(pPlayer->x - searchPlayer->x) < dfATTACK2_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK2_RANGE_Y) // y 좌표 절대값 비교
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// 오른쪽 방향 공격
		else if (pPlayer->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - pPlayer->x) >= 0 &&
				(searchPlayer->x - pPlayer->x) < dfATTACK2_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK2_RANGE_Y) // y 좌표 절대값 비교
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
	targetPlayer->hp -= 20;
	// 데미지 패킷 및 헤더 생성, 전송
	CPacket damagePacket;
	PacketHeader damageHeader;
	mpDamage(&damageHeader, &damagePacket, pPlayer->session->id, targetPlayer->session->id, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// 죽은 경우
	if (targetPlayer->hp <= 0)
	{
		// 삭제 패킷 브로드캐스트
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
	// 받은 패킷 해석
	// cout << "# PACKET_RECV # SessionID:" << pPlayer->session->id << endl;
	// cout << "dfPACKET_CS_ATTACK3 # SessionID :" << pPlayer->session->id << " X : " << pPlayer->x << " Y : " << pPlayer->y << endl;
	unsigned char Direction = 0;
	unsigned short x = 0;
	unsigned short y = 0;

	*pPacket >> Direction >> x >> y;


	pPlayer->dir = Direction;
	// 헤더 및 페이로드 생성 및 전송
	CPacket sendAttackPacket;
	PacketHeader sendAttackHeader;
	mpAttack3(&sendAttackHeader, &sendAttackPacket, pPlayer->dir, pPlayer->x, pPlayer->y, pPlayer->session->id);
	NetworkManager::GetInstance()->SendBroadCast(pPlayer->session, &sendAttackHeader, &sendAttackPacket);
	// 피격 대상 탐색
	Player* targetPlayer = nullptr;
	for (auto& searchPlayer : ObjectManager::GetInstance()->GetObjectList())
	{
		// 본인 제외
		if (searchPlayer == pPlayer)
			continue;
		// 왼쪽 방향 공격
		if (pPlayer->dir == dfPACKET_MOVE_DIR_LL)
		{
			if ((pPlayer->x - searchPlayer->x) >= 0 &&
				(pPlayer->x - searchPlayer->x) < dfATTACK3_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK3_RANGE_Y) // y 좌표 절대값 비교
			{
				if (targetPlayer == nullptr)
					targetPlayer = searchPlayer;
				else if (targetPlayer->x < searchPlayer->x)
					targetPlayer = searchPlayer;
			}
		}
		// 오른쪽 방향 공격
		else if (pPlayer->dir == dfPACKET_MOVE_DIR_RR)
		{
			if ((searchPlayer->x - pPlayer->x) >= 0 &&
				(searchPlayer->x - pPlayer->x) < dfATTACK3_RANGE_X &&
				abs(pPlayer->y - searchPlayer->y) < dfATTACK3_RANGE_Y) // y 좌표 절대값 비교
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
	targetPlayer->hp -= 30;
	// 데미지 패킷 및 헤더 생성, 전송
	CPacket damagePacket;
	PacketHeader damageHeader;
	mpDamage(&damageHeader, &damagePacket, pPlayer->session->id, targetPlayer->session->id, targetPlayer->hp);
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &damageHeader, &damagePacket);
	// 죽은 경우
	if (targetPlayer->hp <= 0)
	{
		// 삭제 패킷 브로드캐스트
		CPacket deletePacket;
		PacketHeader deleteHeader;
		mpDelete(&deleteHeader, &deletePacket, targetPlayer->session->id);
		NetworkManager::GetInstance()->SendBroadCast(nullptr, &deleteHeader, &deletePacket);
		DisconnectByPlayer(targetPlayer);
	}
	return true;
}