#include "pch.h"
#include "Player.h"
#include "PacketDefine.h"
#include "ObjectManager.h"
#include "SessionManager.h"
#include "SectorManager.h"
#include "MakePacket.h"
#include "NetworkManager.h"
#include "SerializingBuffer.h"
#include "TimeManager.h"

// �÷��̾� ���� �ݹ��Լ�
// ���� ������ �� �ݹ��Լ��� �����
void CreatePlayer(Session* session)
{
	Player* player = new Player;
	player->hp = 100;
	player->sessionID = session->sessionID;
	player->dir = dfPACKET_MOVE_DIR_LL;
	player->y = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP - 1;
	player->x = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT - 1;
	//player->y = rand() % (dfRANGE_SECTOR_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP;
	//player->x = rand() % (dfRANGE_SECTOR_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT;
	SectorPos pos = SectorManager::GetInstance()->FindSectorPos(player->y, player->x);
	player->sectorPos.y = pos.y;
	player->sectorPos.x = pos.x;
	player->session = session;
	player->lastProcTime = TimeManager::GetInstance()->GetCurrentTick();

	// ĳ���� ���� ��Ŷ ����
	CPacket CreateMyCharacterPacket;
	st_PACKET_HEADER CreateMyCharacterHeader;
	mpCreateMyCharacter(&CreateMyCharacterHeader, &CreateMyCharacterPacket, session->sessionID, player->dir, player->x, player->y, player->hp);

	// �� ĳ���� ����
	NetworkManager::GetInstance()->SendUnicast(session, &CreateMyCharacterHeader, &CreateMyCharacterPacket);
	ObjectManager::GetInstance()->AddPlayer(player);

	CPacket BroadcastMyCharacterToOthersPacket;
	st_PACKET_HEADER BroadcastMyCharacterToOthersHeader;
	mpCreateOtherCharacter(&BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket, session->sessionID, player->dir, player->x, player->y, player->hp);

	// �� ĳ���� ���� ����
	SectorManager::GetInstance()->SendAround(session, &BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket);

	int direction[9][2] = { {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, 0} };

	// �ٸ� ĳ���� ����
	for (int i = 0; i < 9; i++)
	{
		int dy = player->sectorPos.y + direction[i][0];
		int dx = player->sectorPos.x + direction[i][1];
		const auto& playerUMap = SectorManager::GetInstance()->GetSectorPlayerMap(dy, dx);
		
		for (auto& pair : playerUMap)
		{
			CPacket CreateOtherCharacterPacket;
			st_PACKET_HEADER CreateOtherCharacterHeader;
			mpCreateOtherCharacter(&CreateOtherCharacterHeader, &CreateOtherCharacterPacket, pair.second->session->sessionID, pair.second->dir, pair.second->x, pair.second->y, pair.second->hp);
			NetworkManager::GetInstance()->SendUnicast(session, &CreateOtherCharacterHeader, &CreateOtherCharacterPacket);
			// �����̴� �÷��̾��� ��� ���� ��Ŷ�� ������
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

// �̰� ���� �����ÿ� �ݹ����� ����
void DeletePlayer(Session* session)
{
	CPacket deletePacket;
	st_PACKET_HEADER deleteHeader;
	mpDelete(&deleteHeader, &deletePacket, session->sessionID);
	SectorManager::GetInstance()->SendAround(session, &deleteHeader, &deletePacket);

	Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
	SectorManager::GetInstance()->DeletePlayerInSector(player);
	ObjectManager::GetInstance()->DeletePlayer(player);
	delete player;
}

void Player::Update()
{
	DWORD currentTick = TimeManager::GetInstance()->GetCurrentTick();
	if (currentTick - lastProcTime >= dfNETWORK_PACKET_RECV_TIMEOUT)
	{
		NetworkManager::GetInstance()->Disconnect(session);
		return;
	}
	int direction[8][2] = { {0, -6}, {-4, -6}, {-4, 0}, {-4, 6}, {0, 6}, {4, 6}, {4, 0}, {4, -6} };
	int tempX, tempY;
	if (moveFlag == true)
	{
		tempY = y + direction[dir][0];
		tempX = x + direction[dir][1];

		// ���� ������ �����
		if (tempX < dfRANGE_MOVE_LEFT)
		{
			//session->player->x = dfRANGE_MOVE_LEFT;
			return;
		}
		if (tempX >= dfRANGE_MOVE_RIGHT)
		{
			//session->player->x = dfRANGE_MOVE_RIGHT;
			return;
		}
		if (tempY < dfRANGE_MOVE_TOP)
		{
			//session->player->y = dfRANGE_MOVE_TOP;
			return;
		}
		if (tempY >= dfRANGE_MOVE_BOTTOM)
		{
			//session->player->y = dfRANGE_MOVE_BOTTOM;
			return;
		}
		// ���ǿ� �����ϴ� ��쿡�� �� ����
		y = tempY;
		x = tempX;
		SectorManager::GetInstance()->SectorMove(this);
	}
}
