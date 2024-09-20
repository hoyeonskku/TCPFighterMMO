#pragma once

class Player;
class CPacket;

bool netPacketProc_MoveStart(Player* pSession, CPacket* pPacket);
bool netPacketProc_MoveStop(Player* pSession, CPacket* pPacket);
bool netPacketProc_Attack1(Player* pSession, CPacket* pPacket);
bool netPacketProc_Attack2(Player* pSession, CPacket* pPacket);
bool netPacketProc_Attack3(Player* pSession, CPacket* pPacket);
