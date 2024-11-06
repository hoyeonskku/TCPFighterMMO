#pragma once
#include "CObjectPool.h"
#include "SerializingBuffer.h"


class SerializingBufferManager
{
private:
	SerializingBufferManager() { };
	~SerializingBufferManager() { };
public:
	static SerializingBufferManager* GetInstance(void)
	{
		static SerializingBufferManager Sys;
		return &Sys;
	}
	
	CMemoryPool<CPacket, true> _cPacketPool = CMemoryPool<CPacket, true>(0);
};
