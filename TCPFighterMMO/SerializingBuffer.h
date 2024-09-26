#pragma once
#define _WINSOCKAPI_
#include "pch.h"
#include <windows.h>
#include <stdexcept>

/////////////////////////////////////////////////////////////////////
// www.gamecodi.com						이주행 master@gamecodi.com
//
//
/////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------

	Packet.

	네트워크 패킷용 클래스.
	간편하게 패킷에 순서대로 데이타를 In, Out 한다.

	- 사용법.

	CPacket cPacket;  or CMessage Message;

	넣기.
	clPacket << 40030;		or	clPacket << iValue;	(int 넣기)
	clPacket << 1.4;		or	clPacket << fValue;	(float 넣기)


	빼기.
	clPacket >> iValue;		(int 빼기)
	clPacket >> byValue;		(BYTE 빼기)
	clPacket >> fValue;		(float 빼기)

	CPacket Packet2;

	!.	삽입되는 데이타 FIFO 순서로 관리된다.
		환형 큐는 아니므로, 넣기(<<).빼기(>>) 를 혼합해서 사용하지 않도록 한다



	* 실제 패킷 프로시저에서의 처리

	BOOL	netPacketProc_CreateMyCharacter(CPacket *clpPacket)
	{
		DWORD dwSessionID;
		short shX, shY;
		char chHP;
		BYTE byDirection;

//		*clpPacket >> dwSessionID >> byDirection >> shX >> shY >> chHP;


		*clpPacket >> dwSessionID;
		*clpPacket >> byDirection;
		*clpPacket >> shX;
		*clpPacket >> shY;
		*clpPacket >> chHP;

		...
		...
	}


	* 실제 메시지(패킷) 생성부에서의 처리

	CPacket MoveStart;
	mpMoveStart(&MoveStart, dir, x, y);
	SendPacket(&MoveStart);


	void	mpMoveStart(CPacket *clpPacket, BYTE byDirection, short shX, short shY)
	{
		st_NETWORK_PACKET_HEADER	stPacketHeader;
		stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
		stPacketHeader.bySize = 5;
		stPacketHeader.byType = dfPACKET_CS_MOVE_START;

		clpPacket->PutData((char *)&stPacketHeader, dfNETWORK_PACKET_HEADER_SIZE);

		*clpPacket << byDirection;
		*clpPacket << shX;
		*clpPacket << shY;

	}

----------------------------------------------------------------*/
#ifndef  __PACKET__
#define  __PACKET__

class CPacket
{
public:

	/*---------------------------------------------------------------
	Packet Enum.
	----------------------------------------------------------------*/
	enum en_PACKET
	{
		eBUFFER_DEFAULT = 1400		// 패킷의 기본 버퍼 사이즈.
	};

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CPacket() : _capacity(eBUFFER_DEFAULT) {}

	~CPacket()	{	}


	//////////////////////////////////////////////////////////////////////////
	// 패킷 청소.
	//
	// Parameters: 없음.
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	void Clear(void)
	{
		_front = 0;
		_rear = 0;
	}


	//////////////////////////////////////////////////////////////////////////
	// 버퍼 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)패킷 버퍼 사이즈 얻기.
	//////////////////////////////////////////////////////////////////////////
	int	GetBufferSize(void) { return _capacity; }
	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 데이타 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int GetDataSize(void) { return _rear; }

	int GetFreeSize(void) { return _capacity - _rear; };

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	char* GetBufferPtr(void) { return _buffer; }

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 Pos 이동. (음수이동은 안됨)
	// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
	//
	// Parameters: (int) 이동 사이즈.
	// Return: (int) 이동된 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		MoveWritePos(int iSize)
	{
		if (iSize <= 0)
			throw std::runtime_error("Serialization buffer has been requested to move readPos minus value!");

		_front += iSize;

		if (_front > _rear)
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");

		return iSize;
	};
	int		MoveReadPos(int iSize)
	{
		if (iSize <= 0)
			throw std::runtime_error("Serialization buffer has been requested to move readPos minus value!");

		_rear += iSize;

		if (_rear >= _capacity)
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");
		return iSize;
	};

	/* ============================================================================= */
	// 연산자 오버로딩
	/* ============================================================================= */
	CPacket& operator = (CPacket& clSrcPacket)
	{
		CPacket* cpyPacket = new CPacket;
		cpyPacket->_capacity = clSrcPacket._capacity;
		cpyPacket->_rear = clSrcPacket._rear;
		cpyPacket->_front = clSrcPacket._front;
		cpyPacket->_capacity = clSrcPacket._capacity;

		memcpy(cpyPacket->_buffer, clSrcPacket._buffer, _rear);
	}
	//////////////////////////////////////////////////////////////////////////
	// 넣기.	각 변수 타입마다 모두 만듬.
	// 캐패시티를 매번 검사하여 메모리 오염 막음
	//////////////////////////////////////////////////////////////////////////
	CPacket& operator << (UCHAR byValue)
	{
		if (_capacity < _rear + sizeof(UCHAR))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<UCHAR*>(_buffer + _rear) = byValue;
		_rear += sizeof(UCHAR);

		return *this;
	}

	CPacket& operator << (char chValue)
	{
		if (_rear < _front + sizeof(char) || _capacity < _rear + sizeof(char))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<char*>(_buffer + _rear) = chValue;
		_rear += sizeof(char);

		return *this;
	}

	CPacket& operator << (short shValue)
	{
		if (_capacity < _rear + sizeof(short))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<short*>(_buffer + _rear) = shValue;
		_rear += sizeof(short);

		return *this;
	}
	CPacket& operator << (USHORT wValue)
	{
		if (_capacity < _rear + sizeof(USHORT))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<USHORT*>(_buffer + _rear) = wValue;
		_rear += sizeof(USHORT);

		return *this;
	}

	CPacket& operator << (int iValue)
	{
		if (_capacity < _rear + sizeof(int))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<int*>(_buffer + _rear) = iValue;
		_rear += sizeof(int);

		return *this;
	}

	CPacket& operator << (UINT32 iValue)
	{
		if (_capacity < _rear + sizeof(UINT32))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<UINT32*>(_buffer + _rear) = iValue;
		_rear += sizeof(UINT32);

		return *this;
	}

	CPacket& operator << (long lValue)
	{
		if (_capacity < _rear + sizeof(long))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<long*>(_buffer + _rear) = lValue;
		_rear += sizeof(long);

		return *this;
	}

	CPacket& operator << (unsigned long lValue)
	{
		if (_capacity < _rear + sizeof(unsigned long))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<unsigned long*>(_buffer + _rear) = lValue;
		_rear += sizeof(unsigned long);

		return *this;
	}

	CPacket& operator << (float fValue)
	{
		if (_capacity < _rear + sizeof(float))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<float*>(_buffer + _rear) = fValue;
		_rear += sizeof(float);

		return *this;
	}

	CPacket& operator << (__int64 iValue)
	{
		if (_capacity < _rear + sizeof(__int64))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<__int64*>(_buffer + _rear) = iValue;
		_rear += sizeof(__int64);

		return *this;
	}

	CPacket& operator << (unsigned __int64 iValue)
	{
		if (_capacity < _rear + sizeof(unsigned __int64))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<unsigned __int64*>(_buffer + _rear) = iValue;
		_rear += sizeof(unsigned __int64);

		return *this;
	}

	CPacket& operator << (double dValue)
	{
		if (_capacity < _rear + sizeof(double))
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		*reinterpret_cast<double*>(_buffer + _rear) = dValue;
		_rear += sizeof(double);

		return *this;
	}


	//////////////////////////////////////////////////////////////////////////
	// 빼기.	각 변수 타입마다 모두 만듬.
	// 더 많이 빼는 순간을 조건문으로 검색
	//////////////////////////////////////////////////////////////////////////
	CPacket& operator >> (BYTE& byValue)
	{
		if (_rear < _front + sizeof(BYTE))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");

		byValue = *reinterpret_cast<BYTE*>(_buffer + _front);
		_front = _front + sizeof(BYTE);
		return *this;
	}
	CPacket& operator >> (char& chValue)
	{
		if (_rear < _front + sizeof(char))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");

		chValue = *reinterpret_cast<char*>(_buffer + _front);
		_front = _front + sizeof(char);
		return *this;
	}

	CPacket& operator >> (short& shValue)
	{
		if (_rear < _front + sizeof(short))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");

		shValue = *reinterpret_cast<short*>(_buffer + _front);
		_front = _front + sizeof(short);
		return *this;
	}
	CPacket& operator >> (WORD& wValue)
	{
		if (_rear < _front + sizeof(WORD))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");

		wValue = *reinterpret_cast<WORD*>(_buffer + _front);
		_front = _front + sizeof(WORD);
		return *this;
	}

	CPacket& operator >> (UINT32& iValue)
	{
		if (_rear < _front + sizeof(UINT32))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");

		iValue = *reinterpret_cast<UINT32*>(_buffer + _front);
		_front = _front + sizeof(UINT32);
		return *this;
	}

	CPacket& operator >> (int& iValue)
	{
		if (_rear < _front + sizeof(int))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");
		iValue = *reinterpret_cast<int*>(_buffer + _front);
		_front = _front + sizeof(int);
		return *this;
	}

	CPacket& operator >> (DWORD& dwValue)
	{
		if (_rear < _front + sizeof(DWORD))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");
		dwValue = *reinterpret_cast<DWORD*>(_buffer + _front);
		_front = _front + sizeof(DWORD);
		return *this;
	}

	CPacket& operator >> (float& fValue)
	{
		if (_rear < _front + sizeof(float))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");
		fValue = *reinterpret_cast<float*>(_buffer + _front);
		_front = _front + sizeof(float);
		return *this;
	}

	CPacket& operator >> (__int64& iValue)
	{
		if (_rear < _front + sizeof(__int64))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");
		iValue = *reinterpret_cast<__int64*>(_buffer + _front);
		_front = _front + sizeof(__int64);
		return *this;
	}

	CPacket& operator >> (unsigned __int64& iValue)
	{
		if (_rear < _front + sizeof(unsigned __int64))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");
		iValue = *reinterpret_cast<unsigned __int64*>(_buffer + _front);
		_front = _front + sizeof(unsigned __int64);
		return *this;
	}

	CPacket& operator >> (double& dValue)
	{
		if (_rear < _front + sizeof(double))
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");
		dValue = *reinterpret_cast<double*>(_buffer + _front);
		_front = _front + sizeof(double);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	// 데이타 얻기.
	//
	// Parameters: (char *)Dest 포인터. (int)Size.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		GetData(char* chpDest, int iSize)
	{
		if (_rear < _front + iSize)
			throw std::runtime_error("Serialization buffer has been requested to write beyond its capacity!");

		memcpy(chpDest, _buffer + _front, iSize);
		_front = _front + iSize;
		return iSize;
	}

	//////////////////////////////////////////////////////////////////////////
	// 데이타 삽입.
	//
	// Parameters: (char *)Src 포인터. (int)SrcSize.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		PutData(char* chpSrc, int iSrcSize)
	{
		if (_capacity < _rear + iSrcSize)
			throw std::runtime_error("Serialization buffer has been requested to read beyond its size!");

		memcpy(_buffer + _rear, chpSrc, iSrcSize);
		_rear = _rear + iSrcSize;
		return iSrcSize;
	}

protected:
	char _buffer[eBUFFER_DEFAULT];
	int _rear = 0;
	int _front = 0;
	int _capacity;
};

#endif