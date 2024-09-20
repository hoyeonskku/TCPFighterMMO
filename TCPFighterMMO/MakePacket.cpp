#include "pch.h"
#include "MakePacket.h"
#include "PacketDefine.h"
#include "SerializingBuffer.h"

void mpCreateMyCharacter(PacketHeader* pHeader, CPacket* pPacket, unsigned int id, unsigned char dir, unsigned short x, unsigned short y, unsigned char hp)
{
	// ���̷ε�
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	*pPacket << hp;
	//���
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_CREATE_MY_CHARACTER;
}

void mpCreateOtherCharacter(PacketHeader* pHeader, CPacket* pPacket, unsigned int id, unsigned char dir, unsigned short x, unsigned short y, unsigned char hp)
{
	// ���̷ε�
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	*pPacket << hp;
	//���
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
}

void mpDelete(PacketHeader* pHeader, CPacket* pPacket, unsigned int id)
{
	// ���̷ε�
	*pPacket << id;
	//���
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_DELETE_CHARACTER;
}

void mpDamage(PacketHeader* pHeader, CPacket* pPacket, unsigned int attackID, unsigned int DamageID, unsigned char hp)
{
	// ���̷ε�
	*pPacket << attackID;
	*pPacket << DamageID;
	*pPacket << hp;
	//���
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_DAMAGE;
}

void mpMoveStart(PacketHeader* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id)
{
	// ���̷ε�
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	//���
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_MOVE_START;
}

void mpMoveStop(PacketHeader* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id)
{
	// ���̷ε�
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	//���
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_MOVE_STOP;
}

void mpAttack1(PacketHeader* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id)
{
	// ���̷ε�
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	//���
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_ATTACK1;
}

void mpAttack2(PacketHeader* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id)
{
	// ���̷ε�
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	//���
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_ATTACK2;
}

void mpAttack3(PacketHeader* pHeader, CPacket* pPacket, unsigned char dir, unsigned short x, unsigned short y, unsigned int id)
{
	// ���̷ε�
	*pPacket << id;
	*pPacket << dir;
	*pPacket << x;
	*pPacket << y;
	//���
	pHeader->byCode = 0x89;
	pHeader->bySize = pPacket->GetDataSize();
	pHeader->byType = dfPACKET_SC_ATTACK3;
}


