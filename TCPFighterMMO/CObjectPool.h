#ifndef  __MEMORY_POOL__
#define  __MEMORY_POOL__
#pragma once


#include <new.h>
#include <iostream>
#include <stack>
#include <assert.h>


template <class DATA, bool bPlacementNew = false>
class CMemoryPool
{
public:
	struct st_BLOCK_NODE
	{
#ifdef _DEBUG
		st_BLOCK_NODE* pNext;          // ���� �� ��� ������
		CMemoryPool<DATA, bPlacementNew>* pParent;        // �θ� �� ��� ������
		char guardBefore[8];
		DATA data;                       // T Ÿ���� ������
		char guardAfter[8];
#endif
#ifndef _DEBUG
		st_BLOCK_NODE* pNext;          // ���� �� ��� ������
		DATA data;                       // T Ÿ���� ������
#endif
	};
public:
#ifdef _DEBUG
	// ������ �޸� ����
	static const long long GUARD_VALUE = 0xEAEAEAEAEAEAEAEA;

#endif
	CMemoryPool(int iBlockNum) : _pHeadNode(nullptr)
#ifdef _DEBUG
		, m_iUseCount(0), m_iCapacity(iBlockNum)
#endif
	{

#ifdef _DEBUG
		// ��� �ʱ�ȭ
		st_BLOCK_NODE* newNode;
		for (int i = 0; i < iBlockNum; i++)
		{
			newNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));

			if constexpr (bPlacementNew)
			{
				new ((char*)newNode + offsetof(st_BLOCK_NODE, data)) DATA();
			}
			newNode->pNext = _pHeadNode;
			_pHeadNode = newNode;

			*((long long*)newNode->guardBefore) = GUARD_VALUE;
			*((long long*)newNode->guardAfter) = GUARD_VALUE;
		}
#endif

#ifndef _DEBUG
		// ��� �ʱ�ȭ
		st_BLOCK_NODE* newNode;
		for (int i = 0; i < iBlockNum; i++)
		{
			newNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));

			if constexpr (bPlacementNew)
			{
				new ((char*)newNode + offsetof(st_BLOCK_NODE, data)) DATA();
			}
			newNode->pNext = _pHeadNode;
			_pHeadNode = newNode;
		}
#endif
	};

	~CMemoryPool()
	{
		while (_pHeadNode)
		{
			st_BLOCK_NODE* temp = _pHeadNode;
			_pHeadNode = _pHeadNode->pNext;
			if constexpr (!bPlacementNew)
			{
				temp->data.~DATA();  // �޸� ����
			}
			delete temp;
		}
	}
	// �� �ϳ��� �Ҵ�޴´�.  
	DATA* Alloc(void)
	{
#ifdef _DEBUG
		st_BLOCK_NODE* pNode = nullptr;
		if (_pHeadNode == nullptr)
		{
			pNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
			new ((char*)pNode + offsetof(st_BLOCK_NODE, data)) DATA;
			pNode->pNext = nullptr;
			pNode->pParent = this;
			*((long long*)pNode->guardBefore) = GUARD_VALUE;
			*((long long*)pNode->guardAfter) = GUARD_VALUE;
			m_iCapacity++;
			m_iUseCount++;
			return (DATA*)((char*)pNode + offsetof(st_BLOCK_NODE, data));
		}
		m_iUseCount++;
		pNode = _pHeadNode;
		_pHeadNode = pNode->pNext;
		if constexpr (bPlacementNew == true)
		{
			new ((char*)pNode + offsetof(st_BLOCK_NODE, data)) DATA;
		}
		return (DATA*)((char*)pNode + offsetof(st_BLOCK_NODE, data));
#endif
#ifndef _DEBUG
		st_BLOCK_NODE* pNode = nullptr;
		if (_pHeadNode == nullptr)
		{
			pNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
			new ((char*)pNode + offsetof(st_BLOCK_NODE, data)) DATA;
			pNode->pNext = nullptr;
			return (DATA*)((char*)pNode + offsetof(st_BLOCK_NODE, data));
		}
		pNode = _pHeadNode;
		if constexpr (bPlacementNew == true)
		{
			new ((char*)pNode + offsetof(st_BLOCK_NODE, data)) DATA;
		}
		_pHeadNode = pNode->pNext;
		return (DATA*)((char*)pNode + offsetof(st_BLOCK_NODE, data));
#endif
	};
	// ������̴� ���� �����Ѵ�.
	bool	Free(DATA* pData)
	{
#ifdef _DEBUG
		st_BLOCK_NODE* pBlockData = (st_BLOCK_NODE*)((char*)pData - offsetof(st_BLOCK_NODE, data));
		if (*(long long*)(pBlockData->guardBefore) != GUARD_VALUE || *(long long*)(pBlockData->guardAfter) != GUARD_VALUE)
		{
			// ���� ���� �ջ�Ǿ����� �����ϰų� ó��
			std::cerr << "Memory corruption detected during Free!" << std::endl;
			//				std::abort();
			return false; // �Ǵ� ������ ���� ó���� �߰�
		}
		if (pBlockData->pParent != this)
		{
			return false;
		}
		if constexpr (bPlacementNew == true)
		{
			pBlockData->data.~DATA();
		}
		m_iUseCount--;
		pBlockData->pNext = _pHeadNode;
		_pHeadNode = pBlockData;
		return true;
#endif

#ifndef _DEBUG
		st_BLOCK_NODE* pBlockData = (st_BLOCK_NODE*)((char*)pData - offsetof(st_BLOCK_NODE, data));
		if constexpr (bPlacementNew == true)
		{
			pBlockData->data.~DATA();
		}
		pBlockData->pNext = _pHeadNode;
		_pHeadNode = pBlockData;
		return true;
#endif
	};

#ifdef _DEBUG
	// ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
	int		GetCapacityCount(void) { return m_iCapacity; }

	// ���� ������� �� ������ ��´�.
	int		GetUseCount(void) { return m_iUseCount; }
#endif

private:
	st_BLOCK_NODE* _pHeadNode;
#ifdef _DEBUG
	unsigned int m_iUseCount;
	unsigned int m_iCapacity;
#endif
};

#endif