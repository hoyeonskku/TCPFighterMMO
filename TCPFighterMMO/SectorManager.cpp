#include "pch.h"
#include "SectorManager.h"
#include "PacketDefine.h"
#include "MakePacket.h"
#include "NetworkManager.h"
#include "ObjectManager.h"
#include "Sector.h"
#include "Session.h"
#include "Player.h"
#include "SerializingBuffer.h"

void SectorManager::Init()
{
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

void SectorManager::SendSector(st_PACKET_HEADER* pHeader, CPacket* pPacket, int x, int y, Session* elseSession)
{
    for (auto& pair : _sectorArray[y][x]->_playerMap)
    {
        if (pair.second->session == elseSession)
            continue;
        NetworkManager::GetInstance()->SendUnicast(pair.second->session, pHeader, pPacket);
    }
}

void SectorManager::SendAround(Session* session, st_PACKET_HEADER* pHeader, CPacket* pPacket, bool bSendMe)
{
    int direction[9][2] = { {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, 0} };

    Player* player = ObjectManager::GetInstance()->FindPlayer(session->sessionID);
    for (int i = 0; i < 9; i++)
    {
        int dy = player->sectorPos.y + direction[i][0];
        int dx = player->sectorPos.x + direction[i][1];
        {
            if (bSendMe == false)
                SendSector(pHeader, pPacket, dx, dy, session);
            else
                SendSector(pHeader, pPacket, dx, dy);
            
        }
    }
}

void SectorManager::SectorMove(Player* player)
{
    SectorPos nextSectorPos = FindSectorPos(player->y, player->x);
    if (nextSectorPos.x == player->sectorPos.x && nextSectorPos.y == player->sectorPos.y)
        return;

    int dy = nextSectorPos.y - player->sectorPos.y;
    int dx = nextSectorPos.x - player->sectorPos.x;
    std::list<SectorPos> createSectorList;
    std::list<SectorPos> deleteSectorList;
    switch (dy)
    {
    case -1:
    {
        switch (dx)
        {
        case -1: // (-1, -1) 왼위
        {       
            createSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x));
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x + 1));

            deleteSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x + 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x + 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 2, nextSectorPos.x + 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 2, nextSectorPos.x + 1));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 2, nextSectorPos.x));
            break;
        }        
        case 0: // (-1, 0) 위
        {
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x));
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x + 1));

            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 2, nextSectorPos.x - 1));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 2, nextSectorPos.x));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 2, nextSectorPos.x + 1));
            break;
        }
        case 1: // (-1, 1) 오른위
        {
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x));
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x + 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x + 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x + 1));

            deleteSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x - 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x - 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 2, nextSectorPos.x - 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 2, nextSectorPos.x - 1));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 2, nextSectorPos.x));
            break;
        }
        }
        break;
    }
    case 0:
    {
        switch (dx)
        {
        case -1: // (0, -1) 왼
        {
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x - 1));

            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x + 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x + 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x + 2));

            break;
        }
        case 1: // (0, 1) 오른
        {
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x + 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x + 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x + 1));

            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x - 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x - 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x - 2));
            break;
        }
        }
        break;
    }
    case 1:
    {
        switch (dx)
        {
        case -1: // (1, -1) 아래왼
        {
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x + 1));

            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 2, nextSectorPos.x));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 2, nextSectorPos.x + 1));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 2, nextSectorPos.x + 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x + 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x + 2));
            break;
        }
        case 0: // (1, 0) 아래
        {
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x - 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x + 1));

            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 2, nextSectorPos.x - 1));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 2, nextSectorPos.x));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 2, nextSectorPos.x + 1));
            break;
        }
        case 1: // (1, 1) 아래오른
        {
            createSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x + 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x + 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x + 1));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x));
            createSectorList.push_back(SectorPos(nextSectorPos.y + 1, nextSectorPos.x - 1));

            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 2, nextSectorPos.x  - 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 2, nextSectorPos.x - 1));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 2, nextSectorPos.x));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y - 1, nextSectorPos.x - 2));
            deleteSectorList.push_back(SectorPos(nextSectorPos.y, nextSectorPos.x - 2));
            break;
        }
        }
        break;
    }
    }

    // 영역마다 생성/ 삭제 메세지 보내고, 그 영역에 있던 플레이어의 생성/ 삭제 메세지 받음
    st_PACKET_HEADER playerCreateHeader;
    CPacket playerCreatePacket;
    mpCreateOtherCharacter(&playerCreateHeader, &playerCreatePacket, player->sessionID, player->dir, player->x, player->y, player->hp);

    st_PACKET_HEADER playerDeleteHeader;
    CPacket playerDeletePacket;
    mpDelete(&playerDeleteHeader, &playerDeletePacket, player->sessionID);

    st_PACKET_HEADER pMoveHeader;
    CPacket pMovePacket;

    mpMoveStart(&pMoveHeader, &pMovePacket, player->dir, player->x, player->y, player->session->sessionID);
    for (SectorPos& pos : createSectorList)
    {
        {
            SendSector(&playerCreateHeader, &playerCreatePacket, pos.x, pos.y, player->session);
            SendSector(&pMoveHeader, &pMovePacket, pos.x, pos.y, player->session);
            for (auto& pair : _sectorArray[pos.y][pos.x]->_playerMap)
            {
                st_PACKET_HEADER sectorPlayerCreateHeader;
                CPacket sectorPlayerCreatePacket;
                mpCreateOtherCharacter(&sectorPlayerCreateHeader, &sectorPlayerCreatePacket, pair.second->sessionID, pair.second->dir, pair.second->x, pair.second->y, pair.second->hp);
                NetworkManager::GetInstance()->SendUnicast(player->session, &sectorPlayerCreateHeader, &sectorPlayerCreatePacket);

                // 움직이는 플레이어인 경우 무브 패킷도 보내줌
                if (pair.second->moveFlag == true)
                {
                    st_PACKET_HEADER pOtherPlayerMoveHeader;
                    CPacket pOtherPlayerMovePacket;

                    mpMoveStart(&pOtherPlayerMoveHeader, &pOtherPlayerMovePacket, pair.second->dir, pair.second->x, pair.second->y, pair.second->session->sessionID);
                    NetworkManager::GetInstance()->SendUnicast(player->session, &pOtherPlayerMoveHeader, &pOtherPlayerMovePacket);
                }
            }
        }
    }
    for (SectorPos& pos : deleteSectorList)
    {
        {
            SendSector(&playerDeleteHeader, &playerDeletePacket, pos.x, pos.y, player->session);
            for (auto& pair : _sectorArray[pos.y][pos.x]->_playerMap)
            {
                st_PACKET_HEADER sectorPlayerDeleteHeader;
                CPacket sectorPlayerDeletePacket;
                mpDelete(&sectorPlayerDeleteHeader, &sectorPlayerDeletePacket, pair.second->sessionID);
                NetworkManager::GetInstance()->SendUnicast(player->session, &sectorPlayerDeleteHeader, &sectorPlayerDeletePacket);
            }
        }
    }
    _sectorArray[player->sectorPos.y][player->sectorPos.x]->_playerMap.erase(player->sessionID);
    player->sectorPos = nextSectorPos;
    _sectorArray[player->sectorPos.y][player->sectorPos.x]->_playerMap.insert(std::make_pair(player->sessionID, player));
}

void SectorManager::InsertPlayerInSector(Player* player)
{
    _sectorArray[player->sectorPos.y][player->sectorPos.x]->_playerMap.insert(std::make_pair(player->sessionID, player));
}

void SectorManager::DeletePlayerInSector(Player* player)
{
    _sectorArray[player->sectorPos.y][player->sectorPos.x]->_playerMap.erase(player->sessionID);
}

std::unordered_map<int, Player*>& SectorManager::GetSectorPlayerMap(int y, int x)
{
    return _sectorArray[y][x]->_playerMap;
}

SectorPos SectorManager::FindSectorPos(int y, int x)
{
    SectorPos pos(y / dfRANGE_SECTOR_BOTTOM + 1, x / dfRANGE_SECTOR_RIGHT + 1);
    return pos; 
}
