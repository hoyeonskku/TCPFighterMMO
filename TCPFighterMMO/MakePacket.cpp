#include "pch.h"
#include "MakePacket.h"
#include "PacketDefine.h"
#include "SerializingBuffer.h"

void mpCreateMyCharacter(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned int id, unsigned char dir, unsigned short x, unsigned short y, unsigned char hp)
{
	// 페이로드
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	*pPacket << hp;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_CREATE_MY_CHARACTER;
}

void mpCreateOtherCharacter(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned int id, unsigned char dir, unsigned short x, unsigned short y, unsigned char hp)
{
	// 페이로드
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	*pPacket << hp;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
}

void mpDelete(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned int id)
{
	// 페이로드
	*pPacket << id;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_DELETE_CHARACTER;
}

void mpDamage(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned int attackID, unsigned int DamageID, unsigned char hp)
{
	// 페이로드
	*pPacket << attackID;
	*pPacket << DamageID;
	*pPacket << hp;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_DAMAGE;
}

void mpMoveStart(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id)
{
	// 페이로드
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_MOVE_START;
}

void mpMoveStop(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id)
{
	// 페이로드
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_MOVE_STOP;
}

void mpAttack1(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id)
{
	// 페이로드
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_ATTACK1;
}

void mpAttack2(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id)
{
	// 페이로드
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_ATTACK2;
}

void mpAttack3(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id)
{
	// 페이로드
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_ATTACK3;
}

void mpSync(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned short x, unsigned short y, unsigned int id)
{
	// 페이로드
	*pPacket << id;
	*pPacket << x;
	*pPacket << y;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_SYNC;
}

void mpEcho(st_PACKET_HEADER* pHeader, CPacket* pPacket, unsigned int time)
{
	// 페이로드
	*pPacket << time;
	//헤더
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_ECHO;
}


