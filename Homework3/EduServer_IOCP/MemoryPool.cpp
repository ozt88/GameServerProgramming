#include "stdafx.h"
#include "Exception.h"
#include "MemoryPool.h"

MemoryPool* GMemoryPool = nullptr;


SmallSizeMemoryPool::SmallSizeMemoryPool(DWORD allocSize) : mAllocSize(allocSize)
{
	CRASH_ASSERT(allocSize > MEMORY_ALLOCATION_ALIGNMENT);
	InitializeSListHead(&mFreeList);
}

MemAllocInfo* SmallSizeMemoryPool::Pop()
{
	//TODO: InterlockedPopEntrySList�� �̿��Ͽ� mFreeList���� pop���� �޸𸮸� ������ �� �ִ��� Ȯ��. 
	MemAllocInfo* mem = static_cast<MemAllocInfo*>(InterlockedPopEntrySList(&mFreeList));

	if (NULL == mem)
	{
		// �Ҵ� �Ұ����ϸ� ���� �Ҵ�.
		mem = reinterpret_cast<MemAllocInfo*>(_aligned_malloc(mAllocSize, MEMORY_ALLOCATION_ALIGNMENT));
	}
	else
	{
		CRASH_ASSERT(mem->mAllocSize == 0);
	}

	InterlockedIncrement(&mAllocCount);
	return mem;
}

void SmallSizeMemoryPool::Push(MemAllocInfo* ptr)
{
	//TODO: InterlockedPushEntrySList�� �̿��Ͽ� �޸�Ǯ�� (������ ����) �ݳ�.
	InterlockedPushEntrySList(&mFreeList, ptr);
	InterlockedDecrement(&mAllocCount);
}

/////////////////////////////////////////////////////////////////////

MemoryPool::MemoryPool()
{
	memset(mSmallSizeMemoryPoolTable, 0, sizeof(mSmallSizeMemoryPoolTable));

	int recent = 0;

	for (int i = 32; i < 1024; i+=32)
	{
		SmallSizeMemoryPool* pool = new SmallSizeMemoryPool(i);
		for (int j = recent+1; j <= i; ++j)
		{
			mSmallSizeMemoryPoolTable[j] = pool;
		}
		recent = i;
	}

	for (int i = 1024; i < 2048; i += 128)
	{
		SmallSizeMemoryPool* pool = new SmallSizeMemoryPool(i);
		for (int j = recent + 1; j <= i; ++j)
		{
			mSmallSizeMemoryPoolTable[j] = pool;
		}
		recent = i;
	}

	//TODO: [2048, 4096] ���� ������ 256����Ʈ ������ SmallSizeMemoryPool�� �Ҵ��ϰ� 
	//TODO: mSmallSizeMemoryPoolTable�� O(1) access�� �����ϵ��� SmallSizeMemoryPool�� �ּ� ���
	for(int i = 2048; i <= 4096; i += 256)
	{
		SmallSizeMemoryPool* pool = new SmallSizeMemoryPool(i);
		for(int j = recent + 1; j <= i; ++j)
		{
			mSmallSizeMemoryPoolTable[j] = pool;
		}
		recent = i;
	}
}

void* MemoryPool::Allocate(int size)
{
	MemAllocInfo* header = nullptr;
	int realAllocSize = size + sizeof(MemAllocInfo);

	if (realAllocSize > MAX_ALLOC_SIZE)
	{
		header = reinterpret_cast<MemAllocInfo*>(_aligned_malloc(realAllocSize, MEMORY_ALLOCATION_ALIGNMENT));
	}
	else
	{
		//TODO: SmallSizeMemoryPool���� �Ҵ�
		//header = ...; 
		//�Ǹ��� �ڵ�
		header = mSmallSizeMemoryPoolTable[realAllocSize]->Pop();
	}

	return AttachMemAllocInfo(header, realAllocSize);
}

void MemoryPool::Deallocate(void* ptr, long extraInfo)
{
	MemAllocInfo* header = DetachMemAllocInfo(ptr);
	///< �ֱ� �Ҵ翡 ���õ� ���� ��Ʈ
	header->mExtraInfo = extraInfo;
	///< �ι� ���� üũ ����
	long realAllocSize = InterlockedExchange(&header->mAllocSize, 0);
	
	CRASH_ASSERT(realAllocSize> 0);

	if (realAllocSize > MAX_ALLOC_SIZE)
	{
		_aligned_free(header);
	}
	else
	{
		//TODO: SmallSizeMemoryPool�� (������ ����) push..
		mSmallSizeMemoryPoolTable[realAllocSize]->Push(header);
	}
}