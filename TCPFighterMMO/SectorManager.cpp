#include "pch.h"
#include "SectorManager.h"
#include "PacketDefine.h"
#include "NetworkManager.h"
#include "Sector.h"
#include "Player.h"


void SectorManager::Init()
{
    // 힙에 2차원 배열 할당
    _sectorArray = new Sector **[dfRANGE_SECTOR_Y];
    for (int i = 0; i < dfRANGE_SECTOR_Y; ++i)
    {
        _sectorArray[i] = new Sector *[dfRANGE_SECTOR_X];
        for (int j = 0; j < dfRANGE_SECTOR_X; ++j)
        {
            _sectorArray[i][j] = new Sector();
            _sectorArray[i][j]->y = i;
            _sectorArray[i][j]->x = j;
        }
    }
}

void SectorManager::LogicUpdate()
{
    for (int i = 0; i < dfRANGE_SECTOR_Y; ++i)
    {
        for (int j = 0; j < dfRANGE_SECTOR_X; ++j)
        {
            for (auto& pair : _sectorArray[i][j]->_playerMap)
                pair.second->Update();
        }
    }
}

void SectorManager::LifeCycleUpdate()
{
}

void SectorManager::ContentsUpdate()
{
}


void SectorManager::SendSectorToCreateBuffer(char* packet, int size, int x, int y)
{
    _sectorArray[y][x]->contentsBuffer->Enqueue(packet, size);
}

void SectorManager::SendSectorToLifeCycleBuffer(char* packet, int size, int x, int y)
{
    _sectorArray[y][x]->lifeCycleBuffer->Enqueue(packet, size);
}

void SectorManager::SectorMove(Player* player)
{
}

SectorPos SectorManager::FindSectorPos(int x, int y)
{
    // SectorPos 객체를 여기서 생성
    SectorPos pos;
    pos.x = x / dfRANGE_SECTOR_RIGHT;  
    pos.y = y / dfRANGE_SECTOR_BOTTOM; 

    return pos; 
}
