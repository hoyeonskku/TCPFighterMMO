#pragma once

struct st_PACKET_HEADER;
class CPacket;

void mpCreateMyCharacter(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned int id, unsigned char dir, unsigned short x, unsigned short y, unsigned char hp);
void mpCreateOtherCharacter(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned int id, unsigned char dir, unsigned short x, unsigned short y, unsigned char hp);
void mpDelete(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned int id);
void mpMoveStart(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id);
void mpMoveStop(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id);
void mpDamage(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned int attackID, unsigned int DamageID, unsigned char hp);
void mpAttack1(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id);
void mpAttack2(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id);
void mpAttack3(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id);