#pragma once
#include <atomic>
#include "MemoryManager.h"
#include "Defines.h"

//Objects allocated through this allocator will never have their destruct called from it. Therefore it is up to the user to call upon the destructor before freeing the memory!
class StackAllocator
{
public:
	StackAllocator(size_t size);
    ~StackAllocator();

	void* AllocateMemory(const std::string& tag, size_t size, size_t alignment);
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
	void* m_pStart;
	void* m_pEnd;
	void* m_pCurrent;
	size_t m_Used;
	const size_t m_Size;
public:
	static StackAllocator& GetInstance(size_t size = 1024 * 1024 * 4) // = 4mb
	{
		thread_local static StackAllocator instance(size);
		return instance;
	}

	static int GetTotalAvailableMemory()
	{
		return s_TotalAllocated;
	}

	static int GetTotalUsedMemory()
	{
		return s_TotalUsed;
	}
private:
	static std::atomic_int32_t s_TotalAllocated;
	static std::atomic_int32_t s_TotalUsed;
};

namespace Helpers
{
	struct StackDummy {};
}

inline void* operator new(size_t size, size_t alignment, Helpers::StackDummy d, const std::string& tag)
{
	return StackAllocator::GetInstance().AllocateMemory(tag, size, alignment);
}

#define stack_new(tag)			new(1, Helpers::StackDummy(), tag)
#define stack_delete(object)	{ using T = std::remove_pointer< std::remove_reference<decltype(object)>::type >::type; object->~T(); }
#define stack_reset				StackAllocator::GetInstance().Reset
