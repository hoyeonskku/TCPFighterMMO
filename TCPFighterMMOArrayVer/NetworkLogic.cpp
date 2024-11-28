#include "pch.h"
#include "NetworkLogic.h"
#include "MakePacket.h"
#include "PacketDefine.h"
#include "SerializingBuffer.h"
#include "NetworkManager.h"
#include "SectorManager.h"
#include "Player.h"

int g_SyncCount = 0;

// ��ũ �� ��� ������ �����ϴ� �� ���, ���� ���������� sandUnicast �ص� ��� ������, ���� sendAround�� �ؾ� �ϹǷ� ���⼭�� sendAround�� ���.
void SynchronizePos(Player* player)
{
	g_SyncCount++;
	st_PACKET_HEADER header;
	CPacket packet;

	mpSync(&header, &packet, player->x, player->y, player->_sessionID);
	SectorManager::GetInstance()->SendAround(player->_sessionID, &header, &packet, true);

	_LOG(dfLOG_LEVEL_DEBUG, L"Sync Count : %d", g_SyncCount);
}
