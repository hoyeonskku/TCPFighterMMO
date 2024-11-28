#pragma once
#include <memory.h>
#include <iostream>
#include <assert.h>

class DebugLog
{
public:
	int _prevFront;
	int _prevRear;
	int _currentFront;
	int _currentRear;
	int _moveFrontValue;
	int _moveRearValue;
};


class CRingBuffer
{
public:
	CRingBuffer(void) : CRingBuffer(2000) {};
	CRingBuffer(int iBufferSize = 2000)
	{
		_capacity = iBufferSize + 1;
		_buffer = new char[_capacity];
	}
	~CRingBuffer(void)
	{
		delete[] _buffer;
	}


	void Resize(int size)
	{
		char* newBuffer = new char[size + 1];
		int currentSize = size;

		// Copy data from old buffer to new buffer
		if (_front < _rear)
			memcpy(newBuffer, _buffer + _front, GetUseSize());
		else
		{
			int firstPart = _capacity - _front;
			memcpy(newBuffer, _buffer + _front, firstPart);
			memcpy(newBuffer + firstPart, _buffer, _rear);
		}

		delete[] _buffer;
		_buffer = newBuffer;
		_capacity = size + 1;
		_front = 0;
		_rear = GetUseSize() % _capacity;
	}
	inline int GetBufferSize(void)
	{
		return _capacity - 1;
	};
	/////////////////////////////////////////////////////////////////////////
	// ���� ������� �뷮 ���.
	//
	// Parameters: ����.
	// Return: (int)������� �뷮.
	/////////////////////////////////////////////////////////////////////////
	inline int GetUseSize(void)
	{
		int front = _front;
		int rear = _rear;
		if (front <= rear)
			return rear - front;
		else
			return _capacity - front + rear;
	};

	///////////////////////////////////////////////////////////////////////
	// ���� ���ۿ� ���� �뷮 ���. 
	//
	// Parameters: ����.
	// Return: (int)�����뷮.
	/////////////////////////////////////////////////////////////////////////
	inline int GetFreeSize(void)
	{
		return _capacity - 1 - GetUseSize();
	};

	inline bool IsFull()
	{
		return GetFreeSize() == 0;
	}

	/////////////////////////////////////////////////////////////////////////
	// WritePos �� ����Ÿ ����.
	//
	// Parameters: (char *)����Ÿ ������. (int)ũ��. 
	// Return: (int)���� ũ��.
	/////////////////////////////////////////////////////////////////////////
	int	Enqueue(const char* chpData, int iSize)
	{
//#ifdef _DEBUG
		// ����� ��忡���� ������ �����÷ο� ����
		if (_capacity < GetUseSize() + iSize)
		{
			_LOG(dfLOG_LEVEL_ERROR, L"RingBuffer Overflow");
			DebugBreak();
			return 0; // ������ �߰��� ����
		}
//#endif
		/*DebugLog debugLog;
		debugLog._prevFront = _front;
		debugLog._prevRear = _rear;*/

		int freeSize = GetFreeSize();
		if (freeSize < iSize)
			iSize = freeSize; // ���� ������ �°� ũ�� ����

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

		/*debugLog._currentFront = _front;
		debugLog._currentRear = _rear;
		debugLog._moveRearValue = iSize;
		_debugLogList.push_back(debugLog);*/
		return iSize;
	}

	/////////////////////////////////////////////////////////////////////////
	// ReadPos ���� ����Ÿ ������. ReadPos �̵�.
	//
	// Parameters: (char *)����Ÿ ������. (int)ũ��.
	// Return: (int)������ ũ��.
	/////////////////////////////////////////////////////////////////////////
	int	Dequeue(char* chpDest, int iSize)
	{
		/*DebugLog debugLog;
		debugLog._prevFront = _front;
		debugLog._prevRear = _rear;*/
		int size = GetUseSize();
		if (size < iSize)
			iSize = size; // ��û�� ũ�⺸�� ���� ������ ũ�Ⱑ ���� ���

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

		/*debugLog._currentFront = _front;
		debugLog._currentRear = _rear;
		debugLog._moveFrontValue = iSize;
		_debugLogList.push_back(debugLog);*/

		return iSize;
	}

	/////////////////////////////////////////////////////////////////////////
	// ReadPos ���� ����Ÿ �о��. ReadPos ����.
	//
	// Parameters: (char *)����Ÿ ������. (int)ũ��.
	// Return: (int)������ ũ��.
	/////////////////////////////////////////////////////////////////////////
	int	Peek(char* chpDest, int iSize)
	{
		int size = GetUseSize();
		if (size < iSize)
			iSize = size; // ��û�� ũ�⺸�� ���� ������ ũ�Ⱑ ���� ���

		int firstPart = _capacity - _front;
		if (firstPart >= iSize)
			memcpy(chpDest, _buffer + _front, iSize);
		else
		{
			memcpy(chpDest, _buffer + _front, firstPart);
			memcpy(chpDest + firstPart, _buffer, iSize - firstPart);
		}
		return iSize;
	}
	/////////////////////////////////////////////////////////////////////////
	// ������ ��� ����Ÿ ����.
	//
	// Parameters: ����.
	// Return: ����.
	/////////////////////////////////////////////////////////////////////////
	void ClearBuffer(void)
	{
		_front = 0;
		_rear = 0;
	};

public:
	/////////////////////////////////////////////////////////////////////////
	// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ����.
	// (������ ���� ����)
	//
	// ���� ť�� ������ ������ ���ܿ� �ִ� �����ʹ� �� -> ó������ ���ư���
	// 2���� �����͸� ��ų� ���� �� ����. �� �κп��� �������� ���� ���̸� �ǹ�
	//
	// Parameters: ����.
	// Return: (int)��밡�� �뷮.
	////////////////////////////////////////////////////////////////////////
	int	DirectEnqueueSize(void)
	{
		if (_front > _rear)
			return _front - _rear - 1;
		// ������ ������ ���� �ʱ� ������ front0�϶��� ����ó���� �ʿ�
		if (_front == 0)
			return _capacity - _rear - 1;
		else
			return _capacity - _rear;
	}

	int DirectDequeueSize(void)
	{
		if (_front > _rear)
			return _capacity - _front;
		return _rear - _front;
	}

	/////////////////////////////////////////////////////////////////////////
	// ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
	//
	// Parameters: ����.
	// Return: (int)�̵�ũ��
	/////////////////////////////////////////////////////////////////////////
	int	MoveRear(int iSize)
	{
		return _rear = (_rear + iSize) % _capacity;
	}

	int	MoveFront(int iSize)
	{
		return _front = (_front + iSize) % _capacity;
	}
	/////////////////////////////////////////////////////////////////////////
	// ������ Front ������ ����.
	//
	// Parameters: ����.
	// Return: (char *) ���� ������.
	/////////////////////////////////////////////////////////////////////////
	char* GetFrontBufferPtr(void)
	{
		return  _buffer + _front;
	}

	/////////////////////////////////////////////////////////////////////////
	// ������ RearPos ������ ����.
	//
	// Parameters: ����.
	// Return: (char *) ���� ������.
	/////////////////////////////////////////////////////////////////////////
	char* GetRearBufferPtr(void)
	{
		return _buffer + _rear;
	}
public:
	//private:
	char* _buffer;
	int _rear = 0;
	int _front = 0;
	int _capacity;

	//std::list<DebugLog> _debugLogList;
};


// ������ ����ϴ� size ��� ������
// ��Ƽ������ ������ ���� size�� �ƴ�, front , rear���� �����ϴ� �����۷� ��ü
//#pragma once
//#include <memory.h>
//#include <iostream>
//#include <assert.h>
//
//class CRingBuffer
//{
//public:
//	CRingBuffer(void) : CRingBuffer(2000) {};
//	CRingBuffer(int iBufferSize = 2000)
//	{
//		_capacity = iBufferSize;
//		_buffer = new char[iBufferSize];
//	}
//	~CRingBuffer(void)
//	{
//		delete[] _buffer;
//	}
//
//	void Resize(int size)
//	{
//		char* newBuffer = new char[size];
//		int currentSize = (size);
//
//		// Copy data from old buffer to new buffer
//		if (_front < _rear) {
//			memcpy(newBuffer, _buffer + _front, _size);
//		}
//		else {
//			int firstPart = _capacity - _front;
//			memcpy(newBuffer, _buffer + _front, firstPart);
//			memcpy(newBuffer + firstPart, _buffer, _rear);
//		}
//
//		delete[] _buffer;
//		_buffer = newBuffer;
//		_capacity = size;
//		_front = 0;
//		_rear = currentSize % _capacity;
//		_size = currentSize;
//	}
//	inline int GetBufferSize(void)
//	{
//		return _capacity;
//	};
//	/////////////////////////////////////////////////////////////////////////
//	// ���� ������� �뷮 ���.
//	//
//	// Parameters: ����.
//	// Return: (int)������� �뷮.
//	/////////////////////////////////////////////////////////////////////////
//	inline int GetUseSize(void)
//	{
//		return _size;
//	};
//
//	///////////////////////////////////////////////////////////////////////
//	// ���� ���ۿ� ���� �뷮 ���. 
//	//
//	// Parameters: ����.
//	// Return: (int)�����뷮.
//	/////////////////////////////////////////////////////////////////////////
//	inline int GetFreeSize(void)
//	{
//		return _capacity - _size;
//	};
//
//	/////////////////////////////////////////////////////////////////////////
//	// WritePos �� ����Ÿ ����.
//	//
//	// Parameters: (char *)����Ÿ ������. (int)ũ��. 
//	// Return: (int)���� ũ��.
//	/////////////////////////////////////////////////////////////////////////
//	int	Enqueue(char* chpData, int iSize)
//	{
//		if (GetFreeSize() < iSize)
//		{
//			iSize = GetFreeSize(); // ���� ������ �°� ũ�� ����
//		}
//
//		int firstPart = _capacity - _rear;
//		if (firstPart >= iSize)
//		{
//			memcpy(_buffer + _rear, chpData, iSize);
//			_rear = (_rear + iSize) % _capacity;
//		}
//		else
//		{
//			memcpy(_buffer + _rear, chpData, firstPart);
//			memcpy(_buffer, chpData + firstPart, iSize - firstPart);
//			_rear = (_rear + iSize) % _capacity;
//		}
//
//		_size += iSize;
//
//		// ����� ��忡���� ������ �����÷ο� ����
//#ifdef _DEBUG
//		if (_capacity < _size)
//		{
//			DebugBreak();
//			_LOG(dfLOG_LEVEL_ERROR, L"LingBuffer OverFlow");
//		}
//#endif // _DEBUG
//
//		return iSize;
//	}
//
//	/////////////////////////////////////////////////////////////////////////
//	// ReadPos ���� ����Ÿ ������. ReadPos �̵�.
//	//
//	// Parameters: (char *)����Ÿ ������. (int)ũ��.
//	// Return: (int)������ ũ��.
//	/////////////////////////////////////////////////////////////////////////
//	int	Dequeue(char* chpDest, int iSize)
//	{
//		if (_size < iSize)
//		{
//			iSize = _size; // ��û�� ũ�⺸�� ���� ������ ũ�Ⱑ ���� ���
//		}
//
//		int firstPart = _capacity - _front;
//		if (firstPart >= iSize)
//		{
//			memcpy(chpDest, _buffer + _front, iSize);
//			_front = (_front + iSize) % _capacity;
//		}
//		else
//		{
//			memcpy(chpDest, _buffer + _front, firstPart);
//			memcpy(chpDest + firstPart, _buffer, iSize - firstPart);
//			_front = (_front + iSize) % _capacity;
//		}
//
//		_size -= iSize;
//		return iSize;
//	}
//
//	/////////////////////////////////////////////////////////////////////////
//	// ReadPos ���� ����Ÿ �о��. ReadPos ����.
//	//
//	// Parameters: (char *)����Ÿ ������. (int)ũ��.
//	// Return: (int)������ ũ��.
//	/////////////////////////////////////////////////////////////////////////
//	int	Peek(char* chpDest, int iSize)
//	{
//		if (_size < iSize)
//		{
//			iSize = _size; // ��û�� ũ�⺸�� ���� ������ ũ�Ⱑ ���� ���
//		}
//
//		int firstPart = _capacity - _front;
//		if (firstPart >= iSize)
//		{
//			memcpy(chpDest, _buffer + _front, iSize);
//		}
//		else
//		{
//			memcpy(chpDest, _buffer + _front, firstPart);
//			memcpy(chpDest + firstPart, _buffer, iSize - firstPart);
//		}
//		return iSize;
//	}
//	/////////////////////////////////////////////////////////////////////////
//	// ������ ��� ����Ÿ ����.
//	//
//	// Parameters: ����.
//	// Return: ����.
//	/////////////////////////////////////////////////////////////////////////
//	void ClearBuffer(void)
//	{
//		_rear = 0;
//		_front = 0;
//		_size = 0;
//	};
//
//public:
//	/////////////////////////////////////////////////////////////////////////
//	// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ����.
//	// (������ ���� ����)
//	//
//	// ���� ť�� ������ ������ ���ܿ� �ִ� �����ʹ� �� -> ó������ ���ư���
//	// 2���� �����͸� ��ų� ���� �� ����. �� �κп��� �������� ���� ���̸� �ǹ�
//	//
//	// Parameters: ����.
//	// Return: (int)��밡�� �뷮.
//	////////////////////////////////////////////////////////////////////////
//	int	DirectEnqueueSize(void)
//	{
//		if (_size == _capacity)
//			return 0;
//		if (_front > _rear)
//			return _front - _rear;
//		else
//			return _capacity - _rear;
//	}
//
//	int	DirectDequeueSize(void)
//	{
//		if (_size == _capacity)
//			return _capacity - _front;
//		if (_rear > _front)
//			return _rear - _front;
//		else
//			return _capacity - _front;
//	}
//
//	/////////////////////////////////////////////////////////////////////////
//	// ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
//	//
//	// Parameters: ����.
//	// Return: (int)�̵�ũ��
//	/////////////////////////////////////////////////////////////////////////
//	int	MoveRear(int iSize)
//	{
//		_size += iSize;
//
//		return _rear = (_rear + iSize) % _capacity;
//	}
//
//	int	MoveFront(int iSize)
//	{
//		_size -= iSize;
//
//		return _front = (_front + iSize) % _capacity;
//	}
//	/////////////////////////////////////////////////////////////////////////
//	// ������ Front ������ ����.
//	//
//	// Parameters: ����.
//	// Return: (char *) ���� ������.
//	/////////////////////////////////////////////////////////////////////////
//	char* GetFrontBufferPtr(void)
//	{
//		char* returnvalue = _buffer + _front;
//		return returnvalue;
//	}
//
//	/////////////////////////////////////////////////////////////////////////
//	// ������ RearPos ������ ����.
//	//
//	// Parameters: ����.
//	// Return: (char *) ���� ������.
//	/////////////////////////////////////////////////////////////////////////
//	char* GetRearBufferPtr(void)
//	{
//		return _buffer + _rear;
//	}
//
//private:
//	char* _buffer;
//	int _size = 0;
//	int _rear = 0;
//	int _front = 0;
//	int _capacity;
//};