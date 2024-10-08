#include "pch.h"
#include "NetworkLogic.h"
#include "MakePacket.h"
#include "PacketDefine.h"
#include "SerializingBuffer.h"
#include "NetworkManager.h"
#include "SectorManager.h"
#include "Player.h"

int SyncCount = 0;

void SynchronizePos(Player* player)
{
	SyncCount++;
	st_PACKET_HEADER header;
	CPacket packet;

	mpSync(&header, &packet, player->x, player->y, player->sessionID);
	SectorManager::GetInstance()->SendAround(player->session, &header, &packet, true);

	_LOG(dfLOG_LEVEL_DEBUG, L"Sync Count : %d", SyncCount);
}
