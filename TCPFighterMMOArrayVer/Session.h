#pragma once
#include "CRingBuffer.h"

#define BUFFERSIZE 19999

// 디버깅용 패킷 인포, 이슈 추적 시에만 사용
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
//	// 패킷 추가
//	void Enqueue(const PacketInfo& packet) {
//		if (size == MAX_SIZE) {
//			// 큐가 가득 찼을 때는 가장 오래된 항목을 덮어씌움
//			head = (head + 1) % MAX_SIZE;
//		}
//		else {
//			size++;
//		}
//		queue[tail] = packet;
//		tail = (tail + 1) % MAX_SIZE;
//	}
//
//	// 패킷 출력 (디버깅용)
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
//	int head;   // 큐의 시작점
//	int tail;   // 큐의 끝점
//	int size;   // 현재 큐에 있는 패킷 수
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