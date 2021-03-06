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
	//TODO: InterlockedPopEntrySList를 이용하여 mFreeList에서 pop으로 메모리를 가져올 수 있는지 확인. 
	MemAllocInfo* mem = static_cast<MemAllocInfo*>(InterlockedPopEntrySList(&mFreeList));

	if (NULL == mem)
	{
		// 할당 불가능하면 직접 할당.
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
	//TODO: InterlockedPushEntrySList를 이용하여 메모리풀에 (재사용을 위해) 반납.
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

	//TODO: [2048, 4096] 범위 내에서 256바이트 단위로 SmallSizeMemoryPool을 할당하고 
	//TODO: mSmallSizeMemoryPoolTable에 O(1) access가 가능하도록 SmallSizeMemoryPool의 주소 기록
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
		//TODO: SmallSizeMemoryPool에서 할당
		//header = ...; 
		//훌륭한 코드
		header = mSmallSizeMemoryPoolTable[realAllocSize]->Pop();
	}

	return AttachMemAllocInfo(header, realAllocSize);
}

void MemoryPool::Deallocate(void* ptr, long extraInfo)
{
	MemAllocInfo* header = DetachMemAllocInfo(ptr);
	///< 최근 할당에 관련된 정보 힌트
	header->mExtraInfo = extraInfo;
	///< 두번 해제 체크 위해
	long realAllocSize = InterlockedExchange(&header->mAllocSize, 0);
	
	CRASH_ASSERT(realAllocSize> 0);

	if (realAllocSize > MAX_ALLOC_SIZE)
	{
		_aligned_free(header);
	}
	else
	{
		//TODO: SmallSizeMemoryPool에 (재사용을 위해) push..
		mSmallSizeMemoryPoolTable[realAllocSize]->Push(header);
	}
}