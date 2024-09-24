#include "pch.h"
#include "Player.h"
#include "PacketDefine.h"
#include "ObjectManager.h"
#include "SessionManager.h"
#include "SectorManager.h"
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
	//player->y = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP;
	//player->x = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT;
	player->y = rand() % (dfRANGE_SECTOR_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP;
	player->x = rand() % (dfRANGE_SECTOR_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT;
	player->sectorPos.y = player->y / dfRANGE_SECTOR_BOTTOM;
	player->sectorPos.x = player->x / dfRANGE_SECTOR_RIGHT;
	player->session = session;

	// 캐릭터 생성 패킷 생성
	CPacket CreateMyCharacterPacket;
	st_PACKET_HEADER CreateMyCharacterHeader;
	mpCreateMyCharacter(&CreateMyCharacterHeader, &CreateMyCharacterPacket, session->sessionID, player->dir, player->x, player->y, player->hp);

	// 내 캐릭터 생성
	NetworkManager::GetInstance()->SendUnicast(session, &CreateMyCharacterHeader, &CreateMyCharacterPacket);
	ObjectManager::GetInstance()->AddPlayer(player);

	CPacket BroadcastMyCharacterToOthersPacket;
	st_PACKET_HEADER BroadcastMyCharacterToOthersHeader;
	mpCreateOtherCharacter(&BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket, session->sessionID, player->dir, player->x, player->y, player->hp);

	// 내 캐릭터 생성 전파
	SectorManager::GetInstance()->SendAround(session, &BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket);

	int direction[9][2] = { {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, 0} };

	// 다른 캐릭터 생성
	for (int i = 0; i < 9; i++)
	{
		int dy = player->sectorPos.y + direction[i][0];
		int dx = player->sectorPos.x + direction[i][1];
		if (dy < 0 || dy >= dfRANGE_SECTOR_Y || dx < 0 || dx >= dfRANGE_SECTOR_X)
			continue;
		const auto& playerUMap = SectorManager::GetInstance()->GetSectorPlayerMap(dy, dx);
		
		for (auto& pair : playerUMap)
		{
			CPacket CreateOtherCharacterPacket;
			st_PACKET_HEADER CreateOtherCharacterHeader;
			mpCreateOtherCharacter(&CreateOtherCharacterHeader, &CreateOtherCharacterPacket, pair.second->session->sessionID, pair.second->dir, pair.second->x, pair.second->y, pair.second->hp);
			NetworkManager::GetInstance()->SendUnicast(session, &CreateOtherCharacterHeader, &CreateOtherCharacterPacket);
			// 움직이는 플레이어인 경우 무브 패킷도 보내줌
			if (pair.second->moveFlag == true)
			{
				st_PACKET_HEADER pMoveHeader;
				CPacket pMovePacket;

				mpMoveStart(&pMoveHeader, &pMovePacket, pair.second->dir, pair.second->x, pair.second->y, pair.second->session->sessionID);
				NetworkManager::GetInstance()->SendUnicast(session, &pMoveHeader, &pMovePacket);
			}
		}
	}

	SectorManager::GetInstance()->InsertPlayerInSector(player);
	return;
}


// 이걸 세션 삭제시에 콜백으로 던짐
void DeletePlayer(Session* session)
{
	CPacket deletePacket;
	st_PACKET_HEADER deleteHeader;
	mpDelete(&deleteHeader, &deletePacket, session->sessionID);
	SectorManager::GetInstance()->SendAround(session, &deleteHeader, &deletePacket);

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	ObjectManager::GetInstance()->DeletePlayer(player);
	SectorManager::GetInstance()->DeletePlayerInSector(player);
	delete player;
}



void Player::Update()
{
	int direction[8][2] = { {0, -6}, {-4, -6}, {-4, 0}, {-4, 6}, {0, 6}, {4, 6}, {4, 0}, {4, -6} };
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
		SectorManager::GetInstance()->SectorMove(this);
	}
}
