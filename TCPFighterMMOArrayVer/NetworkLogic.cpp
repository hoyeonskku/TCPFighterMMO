#include "pch.h"
#include "NetworkLogic.h"
#include "MakePacket.h"
#include "PacketDefine.h"
#include "SerializingBuffer.h"
#include "NetworkManager.h"
#include "SectorManager.h"
#include "Player.h"

int g_SyncCount = 0;

// 싱크 난 경우 주위에 전파하는 데 사용, 현재 구조에서는 sandUnicast 해도 상관 없으나, 보통 sendAround를 해야 하므로 여기서도 sendAround로 사용.
void SynchronizePos(Player* player)
{
	g_SyncCount++;
	st_PACKET_HEADER header;
	CPacket packet;

	mpSync(&header, &packet, player->x, player->y, player->_sessionID);
	SectorManager::GetInstance()->SendAround(player->_sessionID, &header, &packet, true);

	_LOG(dfLOG_LEVEL_DEBUG, L"Sync Count : %d", g_SyncCount);
}
