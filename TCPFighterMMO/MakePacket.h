#pragma once

struct PacketHeader;
class CPacket;

void mpCreateMyCharacter(PacketHeader* pHeader, CPacket* pPacket, unsigned int id, unsigned char dir, unsigned short x, unsigned short y, unsigned char hp);
void mpCreateOtherCharacter(PacketHeader* pHeader, CPacket* pPacket, unsigned int id, unsigned char dir, unsigned short x, unsigned short y, unsigned char hp);
void mpDelete(PacketHeader* pHeader, CPacket* pPacket, unsigned int id);
void mpMoveStart(PacketHeader* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id);
void mpMoveStop(PacketHeader* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id);
void mpDamage(PacketHeader* pHeader, CPacket* pPacket, unsigned int attackID, unsigned int DamageID, unsigned char hp);
void mpAttack1(PacketHeader* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id);
void mpAttack2(PacketHeader* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id);
void mpAttack3(PacketHeader* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id);