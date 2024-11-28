#pragma once

class Session;
class CPacket;

bool netPacketProc_MoveStart(Session* session, CPacket* packet);
bool netPacketProc_MoveStop(Session* session, CPacket* packet);
bool netPacketProc_Attack1(Session* session, CPacket* packet);
bool netPacketProc_Attack2(Session* session, CPacket* packet);
bool netPacketProc_Attack3(Session* session, CPacket* packet);
bool netPacketProc_Echo(Session* session, CPacket* packet);

