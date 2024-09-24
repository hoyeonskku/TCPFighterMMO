#include "pch.h"
#include "NetworkLogic.h"
#include "MakePacket.h"
#include "PacketDefine.h"
#include "SerializingBuffer.h"
#include "NetworkManager.h"
#include "Player.h"

void SynchronizePos(Player* player)
{
	st_PACKET_HEADER header;
	CPacket packet;

	mpSync(&header, &packet, player->x, player->y, player->sessionID);
	NetworkManager::GetInstance()->SendUnicast(player->session, &header, &packet);
}
