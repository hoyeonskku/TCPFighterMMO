#pragma once
#define SERVERPORT 20000
#include <list>
#include <winsock2.h>
#include <assert.h>

class Session;
struct st_PACKET_HEADER;
class CPacket;

class IPacketProcessor
{
public:
    virtual bool ProcessPacket(Session* session, unsigned char packetType, CPacket* packetData) = 0;
    virtual ~IPacketProcessor() = default;
};

class NetworkManager {
private:
    IPacketProcessor* packetProcessor;
    NetworkManager() {}
    ~NetworkManager() { delete packetProcessor; }
public:
    static NetworkManager* GetInstance();

    int netInit(IPacketProcessor* customProcessor = nullptr);
    void netCleanUp();
    void netIOProcess();
    void netProc_Accept();

    void SendUnicast(unsigned long long sessionID, st_PACKET_HEADER* pHeader, CPacket* pPacket);
    //void SendBroadCast(unsigned long long elseSessionID, st_PACKET_HEADER* pHeader, CPacket* pPacket);
    void Disconnect(unsigned long long sessionID);
    void netProc_Send(Session* session);
    void netProc_Recv(Session* session);
    bool ProcessPacket(Session* session, char packetType, CPacket* packetData);
    SOCKET listenSocket;
    int networkFps = 0;
};

