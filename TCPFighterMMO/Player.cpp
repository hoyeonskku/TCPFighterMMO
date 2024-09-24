#include "pch.h"
#include "Player.h"
#include "PacketDefine.h"
#include "ObjectManager.h"
#include "SessionManager.h"
#include "MakePacket.h"
#include "NetworkManager.h"
#include "SerializingBuffer.h"


// �÷��̾� ���� �ݹ��Լ�
// ���� ������ �� �ݹ��Լ��� �����
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

	// ĳ���� ���� ��Ŷ ����
	CPacket CreateMyCharacterPacket;
	st_PACKET_HEADER CreateMyCharacterHeader;
	mpCreateMyCharacter(&CreateMyCharacterHeader, &CreateMyCharacterPacket, session->sessionID, player->dir, player->x, player->y, player->hp);

	// �� ĳ���� ����
	NetworkManager::GetInstance()->SendUnicast(session, &CreateMyCharacterHeader, &CreateMyCharacterPacket);

	CPacket BroadcastMyCharacterToOthersPacket;
	st_PACKET_HEADER BroadcastMyCharacterToOthersHeader;
	mpCreateOtherCharacter(&BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket, session->sessionID, player->dir, player->x, player->y, player->hp);

	// �� ĳ���� ���� ����
	NetworkManager::GetInstance()->SendBroadCast(session, &BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket);

	// �ٸ� ĳ���� ����
	for (auto& pair : ObjectManager::GetInstance()->GetObjectMap())
	{
		Player* otherPlayer = pair.second;
		CPacket CreateOtherCharacterPacket;
		st_PACKET_HEADER CreateOtherCharacterHeader;
		mpCreateOtherCharacter(&CreateOtherCharacterHeader, &CreateOtherCharacterPacket, otherPlayer->session->sessionID, otherPlayer->dir, otherPlayer->x, otherPlayer->y, otherPlayer->hp);
		NetworkManager::GetInstance()->SendUnicast(session, &CreateOtherCharacterHeader, &CreateOtherCharacterPacket);
		// �����̴� �÷��̾��� ��� ���� ��Ŷ�� ������
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


// �̰� ���� �����ÿ� �ݹ����� ����
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

		// ���� ������ �����
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
		// ���ǿ� �����ϴ� ��쿡�� �� ����
		y = tempY;
		x = tempX;
	}
}
