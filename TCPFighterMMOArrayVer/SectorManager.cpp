#include "pch.h"
#include "SectorManager.h"
#include "PacketDefine.h"
#include "MakePacket.h"
#include "NetworkManager.h"
#include "SessionManager.h"
#include "ObjectManager.h"
#include "Sector.h"
#include "Session.h"
#include "Player.h"
#include "SerializingBuffer.h"
#include "SerializingBufferManager.h"

// 섹터 초기화, 메모리 침범을 줄이기 위해 테두리 빈 섹터 1줄 추가
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
// 섹터가 컨텐츠 코드이기 때문에 SendUnicast를 래핑해서 사용
void SectorManager::SendSector(st_PACKET_HEADER* pHeader, CPacket* pPacket, int x, int y, unsigned long long elseSessionID)
{
    for (Player* player : _sectorArray[y][x]->_playerSet)
    {
        if (player->_sessionID == elseSessionID)
            continue;
        NetworkManager::GetInstance()->SendUnicast(player->_sessionID, pHeader, pPacket);
    }
}

void SectorManager::SendAround(unsigned long long sessionID, st_PACKET_HEADER* pHeader, CPacket* pPacket, bool bSendMe)
{
    int index = SessionManager::GetInstance()->GetIndexBySessionID(sessionID);
    Player* player = ObjectManager::GetInstance()->GetPlayer(sessionID);
    for (int i = 0; i < 9; i++)
    {
        // 플레이어에 있는 sectorDir 재활용했는데 이걸 오브젝트 상위객체에 이동해야할 지 고민중.
        int dy = player->sectorPos.y + Player::sectorDir[i][0];
        int dx = player->sectorPos.x + Player::sectorDir[i][1];
        {
            if (bSendMe == false)
                SendSector(pHeader, pPacket, dx, dy, sessionID);
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
    CPacket* playerCreatePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
    mpCreateOtherCharacter(&playerCreateHeader, playerCreatePacket, player->_sessionID, player->dir, player->x, player->y, player->hp);

    st_PACKET_HEADER playerDeleteHeader;
    CPacket* playerDeletePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
    mpDelete(&playerDeleteHeader, playerDeletePacket, player->_sessionID);

    st_PACKET_HEADER pMoveHeader;
    CPacket* pMovePacket = SerializingBufferManager::GetInstance()->_cPacketPool.Alloc();
    mpMoveStart(&pMoveHeader, pMovePacket, player->dir, player->x, player->y, player->_sessionID);

    for (SectorPos& pos : createSectorList)
    {
        {
            SendSector(&playerCreateHeader, playerCreatePacket, pos.x, pos.y, player->_sessionID);
            SendSector(&pMoveHeader, pMovePacket, pos.x, pos.y, player->_sessionID);
            for (Player* searchedPlayer : _sectorArray[pos.y][pos.x]->_playerSet)
            {
                if (searchedPlayer->useFlag == false)
                    DebugBreak();
                st_PACKET_HEADER sectorPlayerCreateHeader;
                CPacket sectorPlayerCreatePacket;
                mpCreateOtherCharacter(&sectorPlayerCreateHeader, &sectorPlayerCreatePacket, searchedPlayer->_sessionID, searchedPlayer->dir, searchedPlayer->x, searchedPlayer->y, searchedPlayer->hp);
                NetworkManager::GetInstance()->SendUnicast(player->_sessionID, &sectorPlayerCreateHeader, &sectorPlayerCreatePacket);

                // 움직이는 플레이어인 경우 무브 패킷도 보내줌
                if (searchedPlayer->moveFlag == true)
                {
                    st_PACKET_HEADER pOtherPlayerMoveHeader;
                    CPacket pOtherPlayerMovePacket;

                    mpMoveStart(&pOtherPlayerMoveHeader, &pOtherPlayerMovePacket, searchedPlayer->dir, searchedPlayer->x, searchedPlayer->y, searchedPlayer->_sessionID);
                    NetworkManager::GetInstance()->SendUnicast(player->_sessionID, &pOtherPlayerMoveHeader, &pOtherPlayerMovePacket);
                }
            }
        }
    }
    for (SectorPos& pos : deleteSectorList)
    {
        {
            SendSector(&playerDeleteHeader, playerDeletePacket, pos.x, pos.y, player->_sessionID);
            for (Player* searchedPlayer : _sectorArray[pos.y][pos.x]->_playerSet)
            {
                if (searchedPlayer->useFlag == false)
                    DebugBreak();
                st_PACKET_HEADER sectorPlayerDeleteHeader;
                CPacket sectorPlayerDeletePacket;
                mpDelete(&sectorPlayerDeleteHeader, &sectorPlayerDeletePacket, searchedPlayer->_sessionID);
                NetworkManager::GetInstance()->SendUnicast(player->_sessionID, &sectorPlayerDeleteHeader, &sectorPlayerDeletePacket);
            }
        }
    }


    SerializingBufferManager::GetInstance()->_cPacketPool.Free(playerCreatePacket);
    SerializingBufferManager::GetInstance()->_cPacketPool.Free(playerDeletePacket);
    SerializingBufferManager::GetInstance()->_cPacketPool.Free(pMovePacket);

    _sectorArray[player->sectorPos.y][player->sectorPos.x]->_playerSet.erase(player);
    player->sectorPos = nextSectorPos;
    _sectorArray[player->sectorPos.y][player->sectorPos.x]->_playerSet.insert(player);
}

void SectorManager::InsertPlayerInSector(Player* player)
{
    _sectorArray[player->sectorPos.y][player->sectorPos.x]->_playerSet.insert(player);
}

void SectorManager::DeletePlayerInSector(Player* player)
{
    _sectorArray[player->sectorPos.y][player->sectorPos.x]->_playerSet.erase(player);
}

std::unordered_set<Player*>& SectorManager::GetSectorPlayerSet(int y, int x)
{
    return _sectorArray[y][x]->_playerSet;
}

SectorPos SectorManager::FindSectorPos(int y, int x)
{
    SectorPos pos(y / dfRANGE_SECTOR_BOTTOM + 1, x / dfRANGE_SECTOR_RIGHT + 1);
    return pos;
}