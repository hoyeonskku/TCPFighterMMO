#include "Player.h"
#include "PacketDefine.h"
#include "GameLogic.h"
#include "ObjectManager.h"
#include "SessionManager.h"
#include "MakePacket.h"
#include "NetworkManager.h"
#include "SerializingBuffer.h"

// �÷��̾� ���� �ݹ��Լ�
// ���� ������ �� �ݹ��Լ��� �����
Player* CreatePlayer(Session* pSession)
{
	Player* player = new Player;
	player->hp = 100;
	player->dir = dfPACKET_MOVE_DIR_LL;
	player->y = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP;
	player->x = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT;
	player->session = pSession;

	//cout << "Connect # IP:" << addr_str << "/ SessionID :" << session->id << endl;

	// ĳ���� ���� ��Ŷ ����
	CPacket CreateMyCharacterPacket;
	PacketHeader CreateMyCharacterHeader;
	mpCreateMyCharacter(&CreateMyCharacterHeader, &CreateMyCharacterPacket, pSession->id, player->dir, player->x, player->y, player->hp);

	// �� ĳ���� ����
	NetworkManager::GetInstance()->SendUnicast(pSession, &CreateMyCharacterHeader, &CreateMyCharacterPacket);

	//cout << "# PACKET_CONNECT # SessionID:" << session->id << endl;
	//cout << "Create Character # SessionID :" << session->id <<" X : " << session->player->x<< " Y : " << session->player->y << endl;
	CPacket BroadcastMyCharacterToOthersPacket;
	PacketHeader BroadcastMyCharacterToOthersHeader;
	mpCreateOtherCharacter(&BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket, pSession->id, player->dir, player->x, player->y, player->hp);

	// �� ĳ���� ���� ����
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket);

	// �ٸ� ĳ���� ����
	for (auto& existingObject : ObjectManager::GetInstance()->GetObjectList())
	{
		CPacket CreateOtherCharacterPacket;
		PacketHeader CreateOtherCharacterHeader;
		mpCreateOtherCharacter(&CreateOtherCharacterHeader, &CreateOtherCharacterPacket, existingObject->session->id, existingObject->dir, existingObject->x, existingObject->y, existingObject->hp);
		NetworkManager::GetInstance()->SendUnicast(pSession, &CreateOtherCharacterHeader, &CreateOtherCharacterPacket);
		// �����̴� �÷��̾��� ��� ���� ��Ŷ�� ������
		if (existingObject->moveFlag == true)
		{
			PacketHeader pMoveHeader;
			CPacket pMovePacket;

			mpMoveStart(&pMoveHeader, &pMovePacket, existingObject->dir, existingObject->x, existingObject->y, existingObject->session->id);
			NetworkManager::GetInstance()->SendUnicast(pSession, &pMoveHeader, &pMovePacket);
		}
	}
	ObjectManager::GetInstance()->GetObjectList().push_back(player);
	return player;
}

// �÷��̾� ���� �ݹ��Լ�
// ���� ������ �� �ݹ��Լ��� �����
void DeletePlayer(Session* pSession)
{
	pSession->player->deathFlag = true;
	SessionManager::GetInstance()->DelayedSessionDelete(pSession);
}


// ��Ŀ��Ʈ �Լ�
void DisconnectByPlayer(Player* player)
{
	if (player->session->deathFlag == true)
		return;
	closesocket(player->session->socket);
	// ���ǰ� �÷��̾� ��� �����Ǿ�� ��
	player->session->deathFlag = true;
	player->deathFlag = true;
	// ���� �̸� ���� �� bool ����ó�� ����ϱ� ����
	player->session->socket = INVALID_SOCKET;
	CPacket deletePacket;
	PacketHeader deleteHeader;
	mpDelete(&deleteHeader, &deletePacket, player->session->id);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &deleteHeader, &deletePacket);
}
