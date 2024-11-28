#include "pch.h"
#include "Session.h"
#include "SessionManager.h"


short Session::GetIndex()
{
	return static_cast<USHORT>((sessionID >> 48) & 0xFFFF);
}

void Session::Clear(int index)
{
	sendBuffer->ClearBuffer();
	recvBuffer->ClearBuffer();
	/*packetQueue.size = 0;
	packetQueue.head = 0;
	packetQueue.tail = 0;*/
	sessionID = SessionManager::GetInstance()->_id++;
	sessionID &= 0x0000FFFFFFFFFFFF;  // ���� 6����Ʈ�� �״�� �ΰ�
	sessionID |= static_cast<UINT64>(index) << 48;  // ���� 2����Ʈ�� index ���� ����
}