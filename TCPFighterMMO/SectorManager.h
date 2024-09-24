#pragma once
#include "Sector.h"

class Player;

class SectorManager
{
public:
	SectorManager() { };
	~SectorManager() { };
	void Init();
	void LogicUpdate();
	void LifeCycleUpdate();
	void ContentsUpdate();

	void SendSectorToCreateBuffer(char* packet, int size, int x, int y);
	void SendSectorToLifeCycleBuffer(char* packet, int size, int x, int y);
	void SectorMove(Player* player);
	SectorPos FindSectorPos(int x, int y);

private:
	Sector*** _sectorArray;
};
