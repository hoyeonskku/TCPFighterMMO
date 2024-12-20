#include "pch.h"
#include "Player.h"
#include "PacketDefine.h"
#include "ObjectManager.h"
#include "SessionManager.h"
#include "SectorManager.h"
#include "MakePacket.h"
#include "NetworkManager.h"
#include "SerializingBuffer.h"
#include "SerializingBufferManager.h"
#include "TimeManager.h"
#include <string>


// 이동, 섹터 순회에 이용되는 변수를 player static 변수로 저장하여 재할당 억제
// 오브젝트로 상속받아 오브젝트에서 관리할지 고민중
const int Player::sectorDir[9][2] = { {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, 0} };

//int direction[8][2] = { {0, -3}, {-2, -3}, {-2, 0}, {-2, 3}, {0, 3}, {2, 3}, {2, 0}, {2, -3} }
const int Player::direction[8][2] = { {0, -6}, {-4, -6}, {-4, 0}, {-4, 6}, {0, 6}, {4, 6}, {4, 0}, {4, -6} };

// 플레이어 생성 콜백함수
// 세선 생성시 이 콜백함수가 실행됨
void CreatePlayer(Session* session)
{
	Player* player = ObjectManager::GetInstance()->GetPlayer(session->sessionID);
	player->Clear();
	player->hp = 100;
	player->_sessionID = session->sessionID;
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

	// 캐릭터 생성 패킷 생성
	CPacket* CreateMyCharacterPacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
	CreateMyCharacterPacket->Clear();
	st_PACKET_HEADER CreateMyCharacterHeader;
	mpCreateMyCharacter(&CreateMyCharacterHeader, CreateMyCharacterPacket, session->sessionID, player->dir, player->x, player->y, player->hp);
	SerializingBufferManager::GetInstance()->_cPacketPool.Free(CreateMyCharacterPacket);

	// 내 캐릭터 생성
	NetworkManager::GetInstance()->SendUnicast(session->sessionID, &CreateMyCharacterHeader, CreateMyCharacterPacket);
	ObjectManager::GetInstance()->CreatePlayer(session->sessionID);

	CPacket* BroadcastMyCharacterToOthersPacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
	BroadcastMyCharacterToOthersPacket->Clear();
	st_PACKET_HEADER BroadcastMyCharacterToOthersHeader;
	mpCreateOtherCharacter(&BroadcastMyCharacterToOthersHeader, BroadcastMyCharacterToOthersPacket, session->sessionID, player->dir, player->x, player->y, player->hp);

	// 내 캐릭터 생성 전파
	SectorManager::GetInstance()->SendAround(session->sessionID, &BroadcastMyCharacterToOthersHeader, BroadcastMyCharacterToOthersPacket);
	SerializingBufferManager::GetInstance()->_cPacketPool.Free(BroadcastMyCharacterToOthersPacket);

	// 다른 캐릭터 생성
	for (int i = 0; i < 9; i++)
	{
		int dy = player->sectorPos.y + Player::sectorDir[i][0];
		int dx = player->sectorPos.x + Player::sectorDir[i][1];
		const auto& playerUSet = SectorManager::GetInstance()->GetSectorPlayerSet(dy, dx);

		for (Player* searchedPlayer : playerUSet)
		{
			CPacket* CreateOtherCharacterPacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
			CreateOtherCharacterPacket->Clear();
			st_PACKET_HEADER CreateOtherCharacterHeader;
			mpCreateOtherCharacter(&CreateOtherCharacterHeader, CreateOtherCharacterPacket, searchedPlayer->session->sessionID, searchedPlayer->dir, searchedPlayer->x, searchedPlayer->y, searchedPlayer->hp);
			NetworkManager::GetInstance()->SendUnicast(session->sessionID, &CreateOtherCharacterHeader, CreateOtherCharacterPacket);
			SerializingBufferManager::GetInstance()->_cPacketPool.Free(BroadcastMyCharacterToOthersPacket);
			// 움직이는 플레이어인 경우 무브 패킷도 보내줌
			if (searchedPlayer->moveFlag == true)
			{
				st_PACKET_HEADER pMoveHeader;

				CPacket* pMovePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
				pMovePacket->Clear();
				mpMoveStart(&pMoveHeader, pMovePacket, searchedPlayer->dir, searchedPlayer->x, searchedPlayer->y, searchedPlayer->session->sessionID);
				NetworkManager::GetInstance()->SendUnicast(session->sessionID, &pMoveHeader, pMovePacket);
				SerializingBufferManager::GetInstance()->_cPacketPool.Free(pMovePacket);
			}
		}
	}
	// 플레이어 섹터에 추가
	SectorManager::GetInstance()->InsertPlayerInSector(player);
	return;
}

// 이걸 세션 삭제시에 콜백으로 던짐
void DeletePlayer(Session* session)
{
	CPacket* deletePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
	deletePacket->Clear();
	st_PACKET_HEADER deleteHeader;
	mpDelete(&deleteHeader, deletePacket, session->sessionID);
	SectorManager::GetInstance()->SendAround(session->sessionID, &deleteHeader, deletePacket);
	SerializingBufferManager::GetInstance()->_cPacketPool.Free(deletePacket);

	Player* player = ObjectManager::GetInstance()->GetPlayer(session->sessionID);
	// 섹터에서 제거, 플레이어 제거
	SectorManager::GetInstance()->DeletePlayerInSector(player);
	ObjectManager::GetInstance()->DeletePlayer(session->sessionID);
}

void Player::Update()
{
	DWORD currentTick = TimeManager::GetInstance()->GetCurrentTick();
	if (currentTick - lastProcTime >= dfNETWORK_PACKET_RECV_TIMEOUT)
	{
		NetworkManager::GetInstance()->Disconnect(session->sessionID);
		//_LOG(dfLOG_LEVEL_DEBUG, L"Timeout Disconnect, sessionID: %d, time : %d ms", session->sessionID, currentTick - lastProcTime);
		return;
	}
	if (moveFlag == true)
	{
		int tempX, tempY;
		tempY = y + Player::direction[dir][0];
		tempX = x + Player::direction[dir][1];

		// 범위 밖으로 벗어나면
		if (tempX < dfRANGE_MOVE_LEFT)
			return;
		if (tempX >= dfRANGE_MOVE_RIGHT)
			return;
		if (tempY < dfRANGE_MOVE_TOP)
			return;
		if (tempY >= dfRANGE_MOVE_BOTTOM)
			return;

		// 조건에 부합하는 경우에만 값 갱신
		y = tempY;
		x = tempX;

		// 플레이어 섹터 이동 처리
		SectorManager::GetInstance()->SectorMove(this);
	}
}
