#include "pch.h"
#include "Player.h"
#include "PacketDefine.h"
#include "ObjectManager.h"
#include "SessionManager.h"
#include "MakePacket.h"
#include "NetworkManager.h"
#include "SerializingBuffer.h"


// 플레이어 생성 콜백함수
// 세선 생성시 이 콜백함수가 실행됨
void CreatePlayer(Session* session)
{
	Player* player = new Player;
	player->hp = 100;
	player->sessionID = session->sessionID;
	player->dir = dfPACKET_MOVE_DIR_LL;
	player->y = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP;
	player->x = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT;
	player->sectorPos.y = player->y % dfRANGE_SECTOR_Y;
	player->sectorPos.x = player->x % dfRANGE_SECTOR_X;
	player->session = session;

	// 캐릭터 생성 패킷 생성
	CPacket CreateMyCharacterPacket;
	st_PACKET_HEADER CreateMyCharacterHeader;
	mpCreateMyCharacter(&CreateMyCharacterHeader, &CreateMyCharacterPacket, session->sessionID, player->dir, player->x, player->y, player->hp);

	// 내 캐릭터 생성
	NetworkManager::GetInstance()->SendUnicast(session, &CreateMyCharacterHeader, &CreateMyCharacterPacket);

	CPacket BroadcastMyCharacterToOthersPacket;
	st_PACKET_HEADER BroadcastMyCharacterToOthersHeader;
	mpCreateOtherCharacter(&BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket, session->sessionID, player->dir, player->x, player->y, player->hp);

	// 내 캐릭터 생성 전파
	NetworkManager::GetInstance()->SendBroadCast(session, &BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket);

	// 다른 캐릭터 생성
	for (auto& pair : ObjectManager::GetInstance()->GetObjectMap())
	{
		Player* otherPlayer = pair.second;
		CPacket CreateOtherCharacterPacket;
		st_PACKET_HEADER CreateOtherCharacterHeader;
		mpCreateOtherCharacter(&CreateOtherCharacterHeader, &CreateOtherCharacterPacket, otherPlayer->session->sessionID, otherPlayer->dir, otherPlayer->x, otherPlayer->y, otherPlayer->hp);
		NetworkManager::GetInstance()->SendUnicast(session, &CreateOtherCharacterHeader, &CreateOtherCharacterPacket);
		// 움직이는 플레이어인 경우 무브 패킷도 보내줌
		if (otherPlayer->moveFlag == true)
		{
			st_PACKET_HEADER pMoveHeader;
			CPacket pMovePacket;

			mpMoveStart(&pMoveHeader, &pMovePacket, otherPlayer->dir, otherPlayer->x, otherPlayer->y, otherPlayer->session->sessionID);
			NetworkManager::GetInstance()->SendUnicast(session, &pMoveHeader, &pMovePacket);
		}
	}
	const auto& map = ObjectManager::GetInstance()->GetObjectMap();
	ObjectManager::GetInstance()->GetObjectMap().insert(std::make_pair(session->sessionID, player));
	return;
}


// 이걸 세션 삭제시에 콜백으로 던짐
void DeletePlayer(Session* session)
{
	CPacket deletePacket;
	st_PACKET_HEADER deleteHeader;
	mpDelete(&deleteHeader, &deletePacket, session->sessionID);
	NetworkManager::GetInstance()->SendBroadCast(session, &deleteHeader, &deletePacket);
	ObjectManager::GetInstance()->DeletePlayer(session->sessionID);

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	delete player;
}


int direction[8][2] = { {0, -3}, {-2, -3}, {-2, 0}, {-2, 3}, {0, 3}, {2, 3}, {2, 0}, {2, -3} };

void Player::Update()
{
	int tempX, tempY;
	if (moveFlag == true)
	{
		tempY = y + direction[dir][0];
		tempX = x + direction[dir][1];

		// 범위 밖으로 벗어나면
		if (tempX < dfRANGE_MOVE_LEFT)
		{
			//session->player->x = dfRANGE_MOVE_LEFT;
			return;
		}
		if (tempX > dfRANGE_MOVE_RIGHT)
		{
			//session->player->x = dfRANGE_MOVE_RIGHT;
			return;
		}
		if (tempY < dfRANGE_MOVE_TOP)
		{
			//session->player->y = dfRANGE_MOVE_TOP;
			return;
		}
		if (tempY > dfRANGE_MOVE_BOTTOM)
		{
			//session->player->y = dfRANGE_MOVE_BOTTOM;
			return;
		}
		// 조건에 부합하는 경우에만 값 갱신
		y = tempY;
		x = tempX;
	}
}
