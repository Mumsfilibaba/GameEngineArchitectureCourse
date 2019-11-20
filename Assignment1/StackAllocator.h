#pragma once
#include <unordered_map>
#include <thread>
#include "MemoryManager.h"

#include "SpinLock.h"

//Objects allocated through this allocator will never have their destruct called from it. Therefore it is up to the user to call upon the destructor before freeing the memory!

class StackAllocator
{
private:
    void* m_pStart;
	void* m_pEnd;
	void* m_pCurrent;
private:
	
public:
	StackAllocator(size_t size);
    ~StackAllocator();

    template<class T, typename... Args>
    T* Allocate(Args&& ... args);
    template<class T, typename... Args>
    T* AllocateAligned(size_t alignment, Args&& ... args);
    template<class T>
    T* AllocateArray(size_t count, size_t alignment = 1);
    
    void Reset();
    
    inline size_t GetAllocatedMemory() const
    {
        return (size_t)m_pEnd - (size_t)m_pCurrent;
    }
    
    inline size_t GetTotalMemory() const
    {
        return (size_t)m_pEnd - (size_t)m_pStart;
    }
private:
    void* AllocateMem(size_t size, size_t alignment);
public:
	static StackAllocator& GetInstance(size_t size = 4096 * 4096)
	{
		thread_local static StackAllocator instance(size);
		return instance;
	}
};

template<class T, typename ... Args>
inline T* StackAllocator::Allocate(Args&&... args)
{
   return new(AllocateMem(sizeof(T), 1)) T(std::forward<Args>(args) ...);
}

template<class T, typename ... Args>
inline T* StackAllocator::AllocateAligned(size_t alignment, Args&&... args)
{
   return new(AllocateMem(sizeof(T), alignment)) T(std::forward<Args>(args) ...);
}

template<class T>
inline T* StackAllocator::AllocateArray(size_t count, size_t alignment)
{
    size_t arrSize = sizeof(T) * count;
	return new(AllocateMem(arrSize, alignment)) T[count];;
}
