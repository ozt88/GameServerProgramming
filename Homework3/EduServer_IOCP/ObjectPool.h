#pragma once


#include "Exception.h"
#include "FastSpinlock.h"


template <class TOBJECT, int ALLOC_COUNT = 100>
class ObjectPool : public ClassTypeLock<TOBJECT>  ///# �̰� �� ������?
{
public:

	static void* operator new(size_t objSize)
	{
		//TODO: TOBJECT Ÿ�� ������ lock ���
		ClassTypeLock<TOBJECT>::LockGuard(); ///# �̷��� �ϸ� �Ҹ��ڴ� ���� �ݵǳ�? �̰� lock�� ����� �ɸ�����?
		
		
		/// �� �Ʒ� �ڵ尡 thread-safe�Ѱ�?
		
		//freeList üũ
		if (mFreeList == nullptr)
		{
			//�ʿ��� ����Ʈ ������ŭ ������Ʈ freeList�� chunk���� �Ҵ�
			mFreeList = new uint8_t[sizeof(TOBJECT)*ALLOC_COUNT];

			//������ ������Ʈ�� �ּ�
			uint8_t* pNext = mFreeList;
			//FLink�� ���� ������ �ּҸ� �޸� ������ �����Ѵ�.
			uint8_t** ppCurr = reinterpret_cast<uint8_t**>(mFreeList);

			for (int i = 0; i < ALLOC_COUNT - 1; ++i)
			{
				//������Ʈ �Ҵ� ���� �� ��(unint8_t�����ŭ)�� 
				//FLink�� �����Ѵ�.
				pNext += sizeof(TOBJECT);
				*ppCurr = pNext;
				ppCurr = reinterpret_cast<uint8_t**>(pNext);
			}
			//�Ҵ��� �� �ִ� ��Ż ī��Ʈ ++
			mTotalAllocCount += ALLOC_COUNT;
		}
		
		//pAvailable�� ��밡���� ������ ������
		//���� ��밡���� ������ mFreeList(�� ��)�� �ִ� ������Ʈ ����
		uint8_t* pAvailable = mFreeList;
		//mFreeList�� FLink�� ���� �̵�
		mFreeList = *reinterpret_cast<uint8_t**>(pAvailable);
		
		//���� ����ϴ� ī��Ʈ ++
		//�ٽ��� �� ó������ ����
		//++mCurrentUseCount;
		if(++mCurrentUseCount > mTotalAllocCount)
		{
			mFreeList = nullptr;
		}

		return pAvailable;
	}

	static void	operator delete(void* obj)
	{
		//TODO: TOBJECT Ÿ�� ������ lock ���
		ClassTypeLock<TOBJECT>::LockGuard();

	
		CRASH_ASSERT(mCurrentUseCount > 0);

		//��ȯ�ϱ� ���� ī��Ʈ --
		--mCurrentUseCount;

		//��ȯ�� free chunk�� FLink�Է�
		*reinterpret_cast<uint8_t**>(obj) = mFreeList;
		//���ο� freeList�� ���� ������ ��ȯ�� ������Ʈ
		mFreeList = static_cast<uint8_t*>(obj);
	}


private:
	static uint8_t*	mFreeList;
	static int		mTotalAllocCount; ///< for tracing
	static int		mCurrentUseCount; ///< for tracing

};


template <class TOBJECT, int ALLOC_COUNT>
uint8_t* ObjectPool<TOBJECT, ALLOC_COUNT>::mFreeList = nullptr;

template <class TOBJECT, int ALLOC_COUNT>
int ObjectPool<TOBJECT, ALLOC_COUNT>::mTotalAllocCount = 0;

template <class TOBJECT, int ALLOC_COUNT>
int ObjectPool<TOBJECT, ALLOC_COUNT>::mCurrentUseCount = 0;

