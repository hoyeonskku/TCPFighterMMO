#pragma once

class Session;
class CPacket;

bool netPacketProc_MoveStart(Session* pSession, CPacket* pPacket);
bool netPacketProc_MoveStop(Session* pSession, CPacket* pPacket);
bool netPacketProc_Attack1(Session* pSession, CPacket* pPacket);
bool netPacketProc_Attack2(Session* pSession, CPacket* pPacket);
bool netPacketProc_Attack3(Session* pSession, CPacket* pPacket);

