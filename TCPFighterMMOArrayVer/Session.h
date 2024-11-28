#pragma once
#include "CRingBuffer.h"

#define BUFFERSIZE 19999

// ������ ��Ŷ ����, �̽� ���� �ÿ��� ���
//struct PacketInfo
//{
//public:
//	DWORD recivedTime;
//	int packetId;
//	int x;
//	int y;
//	int dir;
//	int serverX;
//	int serverY;
//	int serverDir;
//	int currentFrameCount;
//};
//
//class PacketQueue {
//public:
//	PacketQueue() : head(0), tail(0), size(0) {}
//
//	// ��Ŷ �߰�
//	void Enqueue(const PacketInfo& packet) {
//		if (size == MAX_SIZE) {
//			// ť�� ���� á�� ���� ���� ������ �׸��� �����
//			head = (head + 1) % MAX_SIZE;
//		}
//		else {
//			size++;
//		}
//		queue[tail] = packet;
//		tail = (tail + 1) % MAX_SIZE;
//	}
//
//	// ��Ŷ ��� (������)
//	void PrintQueue() const {
//		int index = head;
//		for (int i = 0; i < size; i++) {
//			std::cout << "Packet ID: " << queue[index].packetId << std::endl;
//			index = (index + 1) % MAX_SIZE;
//		}
//	}
//
//public:
//	static const int MAX_SIZE = 10;
//	PacketInfo queue[MAX_SIZE];
//	int head;   // ť�� ������
//	int tail;   // ť�� ����
//	int size;   // ���� ť�� �ִ� ��Ŷ ��
//};
class Session
{
public:
	Session()
	{
		sendBuffer = new CRingBuffer(BUFFERSIZE);
		recvBuffer = new CRingBuffer(BUFFERSIZE);
	}
	~Session()
	{
		delete sendBuffer;
		delete recvBuffer;
	}
	SOCKET socket;
	bool deathFlag = false;
	bool useFlag = false;
	unsigned short port;
	char ip[16];
	CRingBuffer* sendBuffer;
	CRingBuffer* recvBuffer;
	DWORD dwLastRecvTime;
	unsigned long long sessionID;

	//PacketQueue packetQueue;

	short GetIndex();
	void Clear(int index);
};