#pragma once
#include <memory.h>
#include <iostream>
#include <assert.h>

/////////////////////////////////////////////////////////////////////
// www.gamecodi.com						이주행 master@gamecodi.com
//
//
/////////////////////////////////////////////////////////////////////
class CRingBuffer
{
public:
	CRingBuffer(void) : CRingBuffer(2000) {};
	CRingBuffer(int iBufferSize = 2000)
	{
		_capacity = iBufferSize;
		_buffer = new char[iBufferSize];
	}
	~CRingBuffer(void)
	{
		delete[] _buffer;
	}

	void Resize(int size)
	{
		char* newBuffer = new char[size];
		int currentSize = (size);

		// Copy data from old buffer to new buffer
		if (_front < _rear) {
			memcpy(newBuffer, _buffer + _front, _size);
		}
		else {
			int firstPart = _capacity - _front;
			memcpy(newBuffer, _buffer + _front, firstPart);
			memcpy(newBuffer + firstPart, _buffer, _rear);
		}

		delete[] _buffer;
		_buffer = newBuffer;
		_capacity = size;
		_front = 0;
		_rear = currentSize % _capacity;
		_size = currentSize;
	}
	inline int GetBufferSize(void)
	{
		return _capacity;
	};
	/////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 용량 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 용량.
	/////////////////////////////////////////////////////////////////////////
	inline int GetUseSize(void)
	{
		return _size;
	};

	///////////////////////////////////////////////////////////////////////
	// 현재 버퍼에 남은 용량 얻기. 
	//
	// Parameters: 없음.
	// Return: (int)남은용량.
	/////////////////////////////////////////////////////////////////////////
	inline int GetFreeSize(void)
	{
		return _capacity - _size;
	};

	/////////////////////////////////////////////////////////////////////////
	// WritePos 에 데이타 넣음.
	//
	// Parameters: (char *)데이타 포인터. (int)크기. 
	// Return: (int)넣은 크기.
	/////////////////////////////////////////////////////////////////////////
	int	Enqueue(char* chpData, int iSize)
	{
		if (GetFreeSize() < iSize)
		{
			iSize = GetFreeSize(); // 남은 공간에 맞게 크기 조정
		}

		int firstPart = _capacity - _rear;
		if (firstPart >= iSize)
		{
			memcpy(_buffer + _rear, chpData, iSize);
			_rear = (_rear + iSize) % _capacity;
		}
		else
		{
			memcpy(_buffer + _rear, chpData, firstPart);
			memcpy(_buffer, chpData + firstPart, iSize - firstPart);
			_rear = (_rear + iSize) % _capacity;
		}

		_size += iSize;
		return iSize;
	}

	/////////////////////////////////////////////////////////////////////////
	// ReadPos 에서 데이타 가져옴. ReadPos 이동.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)가져온 크기.
	/////////////////////////////////////////////////////////////////////////
	int	Dequeue(char* chpDest, int iSize)
	{
		if (_size < iSize)
		{
			iSize = _size; // 요청한 크기보다 실제 데이터 크기가 작을 경우
		}

		int firstPart = _capacity - _front;
		if (firstPart >= iSize)
		{
			memcpy(chpDest, _buffer + _front, iSize);
			_front = (_front + iSize) % _capacity;
		}
		else
		{
			memcpy(chpDest, _buffer + _front, firstPart);
			memcpy(chpDest + firstPart, _buffer, iSize - firstPart);
			_front = (_front + iSize) % _capacity;
		}

		_size -= iSize;
		return iSize;
	}

	/////////////////////////////////////////////////////////////////////////
	// ReadPos 에서 데이타 읽어옴. ReadPos 고정.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)가져온 크기.
	/////////////////////////////////////////////////////////////////////////
	int	Peek(char* chpDest, int iSize)
	{
		if (_size < iSize)
		{
			iSize = _size; // 요청한 크기보다 실제 데이터 크기가 작을 경우
		}

		int firstPart = _capacity - _front;
		if (firstPart >= iSize)
		{
			memcpy(chpDest, _buffer + _front, iSize);
		}
		else
		{
			memcpy(chpDest, _buffer + _front, firstPart);
			memcpy(chpDest + firstPart, _buffer, iSize - firstPart);
		}
		return iSize;
	}
	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 모든 데이타 삭제.
	//
	// Parameters: 없음.
	// Return: 없음.
	/////////////////////////////////////////////////////////////////////////
	void ClearBuffer(void)
	{
		_rear = 0;
		_front = 0;
		_size = 0;
	};

public:
	/////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이.
	// (끊기지 않은 길이)
	//
	// 원형 큐의 구조상 버퍼의 끝단에 있는 데이터는 끝 -> 처음으로 돌아가서
	// 2번에 데이터를 얻거나 넣을 수 있음. 이 부분에서 끊어지지 않은 길이를 의미
	//
	// Parameters: 없음.
	// Return: (int)사용가능 용량.
	////////////////////////////////////////////////////////////////////////
	int	DirectEnqueueSize(void)
	{
		if (_size == _capacity)
			return 0;
		if (_front > _rear)
			return _front - _rear;
		else
			return _capacity - _rear;
	}

	int	DirectDequeueSize(void)
	{
		if (_size == _capacity)
			return _capacity - _front;
		if (_rear > _front)
			return _rear - _front;
		else
			return _capacity - _front;
	}

	/////////////////////////////////////////////////////////////////////////
	// 원하는 길이만큼 읽기위치 에서 삭제 / 쓰기 위치 이동
	//
	// Parameters: 없음.
	// Return: (int)이동크기
	/////////////////////////////////////////////////////////////////////////
	int	MoveRear(int iSize)
	{
		_size += iSize;

		return _rear = (_rear + iSize) % _capacity;
	}

	int	MoveFront(int iSize)
	{
		_size -= iSize;

		return _front = (_front + iSize) % _capacity;
	}
	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 Front 포인터 얻음.
	//
	// Parameters: 없음.
	// Return: (char *) 버퍼 포인터.
	/////////////////////////////////////////////////////////////////////////
	char* GetFrontBufferPtr(void)
	{
		char* returnvalue = _buffer + _front;
		return returnvalue;
	}

	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 RearPos 포인터 얻음.
	//
	// Parameters: 없음.
	// Return: (char *) 버퍼 포인터.
	/////////////////////////////////////////////////////////////////////////
	char* GetRearBufferPtr(void)
	{
		return _buffer + _rear;
	}

private:
	char* _buffer;
	int _size = 0;
	int _rear = 0;
	int _front = 0;
	int _capacity;
};