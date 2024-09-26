#pragma once
#include "Sector.h"

class Player;
class CPacket;
class Session;
struct st_PACKET_HEADER;

class SectorManager
{
private:
	SectorManager() { };
	~SectorManager() { };
public:
	static SectorManager* GetInstance(void)
	{
		static SectorManager Sys;
		return &Sys;
	}
	void Init();
	void SendSector(st_PACKET_HEADER* pHeader, CPacket* pPacket, int x, int y, Session* elseSession = nullptr);
	void SendAround(Session* session, st_PACKET_HEADER* pHeader, CPacket* pPacket, bool bSendMe = false);
	void SectorMove(Player* player);
	void InsertPlayerInSector(Player* player);
	void DeletePlayerInSector(Player* player);
	std::unordered_map<int, Player*>& GetSectorPlayerMap(int y, int x);
	SectorPos FindSectorPos(int x, int y);

private:
	Sector*** _sectorArray;
};