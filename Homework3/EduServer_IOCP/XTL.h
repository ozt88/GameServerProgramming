#pragma once
#include "MemoryPool.h"
#include <list>
#include <vector>
#include <deque>
#include <set>
#include <hash_set>
#include <hash_map>
#include <map>
#include <queue>

template <class T>
class STLAllocator
{
public:
	STLAllocator() = default;

	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	//복사 생성자
	template <class U>
	STLAllocator(const STLAllocator<U>&)
	{}

	//rebind
	template <class U>
	struct rebind
	{
		typedef STLAllocator<U> other;
	};

	void construct(pointer p, const T& t)
	{
		new(p)T(t);
	}

	void destroy(pointer p)
	{
		p->~T();
	}

	T* allocate(size_t n)
	{
		//메모리 할당해서 리턴
		return reinterpret_cast<T*>(GMemoryPool->Allocate(n));
	}

	void deallocate(T* ptr, size_t n)
	{
		//메모리풀에 반납
		GMemoryPool->Deallocate(ptr, n);
	}
};


template <class T>
struct xvector
{
	typedef std::vector<T, STLAllocator<T>> type;
};

template <class T>
struct xdeque
{
	//TODO: STL 할당자를 사용하는 deque를 type으로 선언
	typedef std::deque<T, STLAllocator<T>> type;
	//typedef ... type;
};

template <class T>
struct xlist
{
	//TODO: STL 할당자 사용
	typedef std::list<T, STLAllocator<T>> type;
};

template <class K, class T, class C = std::less<K> >
struct xmap
{
	//TODO: STL 할당자 사용하는 map을  type으로 선언
	typedef std::map<K, T, C, STLAllocator<std::pair<K,T>>> type;
	//typedef ... type;
};

template <class T, class C = std::less<T> >
struct xset
{
	//TODO: STL 할당자 사용하는 set을  type으로 선언
	typedef std::set<T, C , STLAllocator<T>> type;
	//typedef ... type;
};

template <class K, class T, class C = std::hash_compare<K, std::less<K>> >
struct xhash_map
{
	typedef std::hash_map<K, T, C, STLAllocator<std::pair<K, T>> > type;
};

template <class T, class C = std::hash_compare<T, std::less<T>> >
struct xhash_set
{
	typedef std::hash_set<T, C, STLAllocator<T> > type;
};

template <class T, class C = std::less<std::vector<T>::value_type> >
struct xpriority_queue
{
	//TODO: STL 할당자 사용하는 priority_queue을  type으로 선언
	//typedef ... type;
	typedef std::priority_queue<T, std::vector<T, STLAllocator<T>>, C > type;
};

typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, STLAllocator<wchar_t>> xstring;

