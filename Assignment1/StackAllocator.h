#pragma once
#include "MemoryManager.h"

//Objects allocated through this allocator will never have their destruct called from it. Therefore it is up to the user to call upon the destructor before freeing the memory!
class StackAllocator
{
public:
	StackAllocator(size_t size);
    ~StackAllocator();

	void* AllocateMemory(size_t size, size_t alignment);
    void Reset();

    template<class T, typename... Args>
    inline T* Allocate(Args&& ... args)
	{
		return new(AllocateMemory(sizeof(T), 1)) T(std::forward<Args>(args) ...);
	}

    template<class T, typename... Args>
    inline T* AllocateAligned(size_t alignment, Args&& ... args)
	{
		return new(AllocateMemory(sizeof(T), alignment)) T(std::forward<Args>(args) ...);
	}

    template<class T>
    inline T* AllocateArray(size_t count, size_t alignment = 1)
	{
		size_t arrSize = sizeof(T) * count;
		return new(AllocateMemory(arrSize, alignment)) T[count];;
	}
    
    inline size_t GetAllocatedMemory() const
    {
        return (size_t)m_pEnd - (size_t)m_pCurrent;
    }
    
    inline size_t GetTotalMemory() const
    {
        return (size_t)m_pEnd - (size_t)m_pStart;
    }
public:
	static StackAllocator& GetInstance(size_t size = 4096 * 4096)
	{
		thread_local static StackAllocator instance(size);
		return instance;
	}
private:
	void* m_pStart;
	void* m_pEnd;
	void* m_pCurrent;
};

namespace Helpers
{
	struct StackDummy {};
}

inline void* operator new(size_t size, size_t alignment, Helpers::StackDummy d)
{
	return StackAllocator::GetInstance().AllocateMemory(size, alignment);
}

#define stack_new				new(1, Helpers::StackDummy())
#define stack_delete(object)	{ using T = std::remove_pointer< std::remove_reference<decltype(object)>::type >::type; object->~T(); }