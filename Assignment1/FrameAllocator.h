#ifndef FRAMEALLOCATOR_H
#define FRAMEALLOCATOR_H

#include <unordered_map>
#include <thread>
#include "MemoryManager.h"

//Objects allocated through this allocator will never have their destruct called from it. Therefore it is up to the user to call upon the destructor before freeing the memory!

class FrameAllocator
{
	friend MemoryManager;

private:
	void* m_pStart;
	void* m_pEnd;
	void* m_pCurrent;
public:
	~FrameAllocator();

	template<class T, typename... Args>
	T* allocate(Args&& ... args);
	template<class T>
	T* allocateArray(unsigned int size);
	void reset();

	static std::unordered_map<std::thread::id, FrameAllocator*> s_FrameAllocatorMap;

private:
	FrameAllocator(size_t size);
	FrameAllocator(void* start, void* end);

public:
	static FrameAllocator& getInstance()
	{
		std::thread::id id = std::this_thread::get_id();
		auto search = s_FrameAllocatorMap.find(id);
		if (search != s_FrameAllocatorMap.end())
		{
			return *(search->second);
		}
		FrameAllocator* allocator = new FrameAllocator(1024 * 1024);
		s_FrameAllocatorMap.insert({ id, allocator });
		return *allocator;
	}

private:
	static void Release()
	{
		for (auto& it : s_FrameAllocatorMap)
		{
			delete it.second;
		}
	}
};

#endif

template<class T, typename ... Args>
inline T * FrameAllocator::allocate(Args&&... args)
{
	T* res = nullptr;
	size_t size = sizeof(T);
	if ((size_t)m_pCurrent + size < (size_t)m_pEnd)
	{
		res = (T*)m_pCurrent;
		m_pCurrent = (void*)((size_t)m_pCurrent + size);
		new (res) T(std::forward<Args>(args) ...);
	}
	return res;
}

template<class T>
inline T * FrameAllocator::allocateArray(unsigned int size)
{
	T* res = nullptr;
	size_t arrSize = sizeof(T) * size;

	if ((size_t)m_pCurrent + arrSize < (size_t)m_pEnd)
	{
		res = (T*)m_pCurrent;
		m_pCurrent = (void*)((size_t)m_pCurrent + arrSize);
		new (res) T[size];
	}
	return res;
}
