#include "Player.h"
#include "PacketDefine.h"
#include "GameLogic.h"
#include "ObjectManager.h"
#include "SessionManager.h"
#include "MakePacket.h"
#include "NetworkManager.h"
#include "SerializingBuffer.h"

// 플레이어 생성 콜백함수
// 세선 생성시 이 콜백함수가 실행됨
Player* CreatePlayer(Session* pSession)
{
	Player* player = new Player;
	player->hp = 100;
	player->sessionID = pSession->sessionID;
	player->dir = dfPACKET_MOVE_DIR_LL;
	player->y = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP;
	player->x = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT;
	player->session = pSession;

	//cout << "Connect # IP:" << addr_str << "/ SessionID :" << session->id << endl;

	// 캐릭터 생성 패킷 생성
	CPacket CreateMyCharacterPacket;
	st_PACKET_HEADER CreateMyCharacterHeader;
	mpCreateMyCharacter(&CreateMyCharacterHeader, &CreateMyCharacterPacket, pSession->id, player->dir, player->x, player->y, player->hp);

	// 내 캐릭터 생성
	NetworkManager::GetInstance()->SendUnicast(pSession, &CreateMyCharacterHeader, &CreateMyCharacterPacket);

	//cout << "# PACKET_CONNECT # SessionID:" << session->id << endl;
	//cout << "Create Character # SessionID :" << session->id <<" X : " << session->player->x<< " Y : " << session->player->y << endl;
	CPacket BroadcastMyCharacterToOthersPacket;
	st_PACKET_HEADER BroadcastMyCharacterToOthersHeader;
	mpCreateOtherCharacter(&BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket, pSession->id, player->dir, player->x, player->y, player->hp);

	// 내 캐릭터 생성 전파
	NetworkManager::GetInstance()->SendBroadCast(nullptr, &BroadcastMyCharacterToOthersHeader, &BroadcastMyCharacterToOthersPacket);

	// 다른 캐릭터 생성
	for (auto& existingObject : ObjectManager::GetInstance()->GetObjectList())
	{
		CPacket CreateOtherCharacterPacket;
		st_PACKET_HEADER CreateOtherCharacterHeader;
		mpCreateOtherCharacter(&CreateOtherCharacterHeader, &CreateOtherCharacterPacket, existingObject->session->id, existingObject->dir, existingObject->x, existingObject->y, existingObject->hp);
		NetworkManager::GetInstance()->SendUnicast(pSession, &CreateOtherCharacterHeader, &CreateOtherCharacterPacket);
		// 움직이는 플레이어인 경우 무브 패킷도 보내줌
		if (existingObject->moveFlag == true)
		{
			st_PACKET_HEADER pMoveHeader;
			CPacket pMovePacket;

			mpMoveStart(&pMoveHeader, &pMovePacket, existingObject->dir, existingObject->x, existingObject->y, existingObject->session->id);
			NetworkManager::GetInstance()->SendUnicast(pSession, &pMoveHeader, &pMovePacket);
		}
	}
	ObjectManager::GetInstance()->GetObjectList().push_back(player);
	return player;
}


// 디스커넥트 함수
void DisconnectByPlayer(Player* player)
{
	if (player->session->deathFlag == true)
		return;
	// 세션과 플레이어 모두 삭제되어야 함
	player->session->deathFlag = true;
	player->deathFlag = true;
	// 소켓 미리 제거 및 bool 변수처럼 사용하기 위함
	player->session->socket = INVALID_SOCKET;
	CPacket deletePacket;
	st_PACKET_HEADER deleteHeader;
	mpDelete(&deleteHeader, &deletePacket, player->session->id);
	NetworkManager::GetInstance()->SendBroadCast(player->session, &deleteHeader, &deletePacket);
}
